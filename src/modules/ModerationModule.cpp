#include "ModerationModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

using namespace Discord;

ModerationModule::ModerationModule()
	: Module("moderation", true)
{
	m_warningCheckTimer.setInterval(24 * 60 * 60 * 1000); // 24hr timer
	QObject::connect(&m_warningCheckTimer, &QTimer::timeout, [this]
	{
		checkWarningsExpiry();
	});
	m_warningCheckTimer.start();

	RegisterCommand(Commands::MODERATION_INVITATION_TOGGLE, "invitations",
		[this](Client& client, const Message& message, const Channel& channel)
	{
			QStringList args = message.content().split(' ');
			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 1, args, true, [this, &client, channel, message, args]()
			{
				m_invitationModeration ^= true;
				client.createMessage(message.channelId(), m_invitationModeration ? "Invitations will be deleted!" : "Invitations won't be deleted!");
			});
	});

	RegisterCommand(Commands::MODERATION_WARN, "warn",
					[this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');
		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, &client, channel, message, args]()
		{
			snowflake_t warnedBy = message.author().id();
			snowflake_t user;
			QList<User> mentions = message.mentions();

			if (mentions.size() > 0)
			{
				user = mentions[0].id();
			}
			else
			{
				user = UmikoBot::Instance().GetUserFromArg(channel.guildId(), args, 1);

				if (!user)
				{
					client.createMessage(message.channelId(), "**Couldn't find " + args.at(1) + "**");
					return;
				}
			}

			QString msg = "[no message]";

			if (args.size() > 2)
			{
				// The 6 starts the search after "!warn "
				msg = message.content().mid(message.content().indexOf(QRegExp("[ \t\n\v\f\r]"), 6)).trimmed();
			}

			warnings[user].append(UserWarning { warnedBy, msg });

			unsigned int numberOfWarnings = countWarnings(user);
			QString warningNumberString = "";

			switch (numberOfWarnings)
			{
			case 1: warningNumberString = "First"; break;
			case 2: warningNumberString = "Second"; break;
			case 3: warningNumberString = "Third"; break;

			// Good enough, shouldn't be seen very often (if at all)
			default: warningNumberString = QString::number(numberOfWarnings) + "th"; break;
			}

			QString output = QString("%1 warning for <@%2>.").arg(warningNumberString, QString::number(user));
			client.createMessage(message.channelId(), output);
		});
	});

	// This is used by both !warnings and !warnings-all
	auto warningsCommand = [this](Client& client, const Message& message, const Channel& channel, bool showExpired)
	{
		QStringList args = message.content().split(' ');
		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, &client, channel, message, args, showExpired]()
		{
			snowflake_t user;
			QList<User> mentions = message.mentions();

			if (mentions.size() > 0)
			{
				user = mentions[0].id();
			}
			else
			{
				user = UmikoBot::Instance().GetUserFromArg(channel.guildId(), args, 1);

				if (!user)
				{
					client.createMessage(message.channelId(), "**Couldn't find " + args.at(1) + "**");
					return;
				}
			}

			UmikoBot::Instance().GetAvatar(channel.guildId(), user).then(
				[this, user, channel, &client, message, showExpired](const QString& icon)
			{
				Embed embed;
				embed.setColor(qrand() % 11777216);
				embed.setAuthor(EmbedAuthor("Warnings for " + UmikoBot::Instance().GetName(channel.guildId(), user), "", icon));

				QString desc = countWarnings(user, showExpired) == 0 ? "Nothing to see here..." : "";
				QList<UserWarning>& userWarnings = warnings[user];
				bool hasOutputExpiredMessage = false;

				// Sorts the warnings in order of newest to oldest
				qSort(userWarnings.begin(), userWarnings.end(), [](const UserWarning& first, const UserWarning& second)
				{
					return second.when < first.when;
				});

				for (auto& warning : userWarnings)
				{
					if (!showExpired && warning.expired)
					{
						continue;
					}

					if (warning.expired)
					{
						if (!showExpired)
							continue;

						if (hasOutputExpiredMessage)
							continue;

						desc += "\n===== Expired =====\n\n";
						hasOutputExpiredMessage = true;
					}

					desc += QString("%1 - warned by %2\n**%3**\n\n").arg(warning.when.toString("yyyy-MM-dd hh:mm:ss"),
																		 UmikoBot::Instance().GetName(channel.guildId(), warning.warnedBy),
																		 warning.message);

				}

				embed.setDescription(desc);
				client.createMessage(message.channelId(), embed);
			});
		});
	};

	RegisterCommand(Commands::MODERATION_WARNINGS, "warnings",
					[this, warningsCommand](Client& client, const Message& message, const Channel& channel)
	{
		return warningsCommand(client, message, channel, false);
	});

	RegisterCommand(Commands::MODERATION_WARNINGS_ALL, "warnings-all",
					[this, warningsCommand](Client& client, const Message& message, const Channel& channel)
	{
		return warningsCommand(client, message, channel, true);
	});

	RegisterCommand(Commands::MODERATION_ADD_DODGY_DOMAIN, "add-dodgy-domain",
		[this](Client& client, const Message& message, const Channel& channel)
	{
			QStringList args = message.content().split(' ');
			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, &client, channel, message, args]()
			{
				dodgyDomainNames.insert(args.at(1));
				client.createMessage(message.channelId(), QString("Added '%1' as a dodgy domain name.").arg(args.at(1)));
			});
	});

	RegisterCommand(Commands::MODERATION_REMOVE_DODGY_DOMAIN, "remove-dodgy-domain",
		[this](Client& client, const Message& message, const Channel& channel)
	{
			QStringList args = message.content().split(' ');
			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, &client, channel, message, args]()
			{
				if (dodgyDomainNames.remove(args.at(1)))
				{
					client.createMessage(message.channelId(), QString("Removed '%1' as a dodgy domain name.").arg(args.at(1)));
				}
				else
				{
					client.createMessage(message.channelId(), QString("Could not find '%1' in my list of dodgy domain names.").arg(args.at(1)));
				}
			});
	});
}

void ModerationModule::OnMessage(Client& client, const Message& message)
{
	client.getChannel(message.channelId()).then(
	[this, message, &client](const Channel& channel)
	{
		const QString& content = message.content();
		
		if (m_invitationModeration)
		{
			if (content.contains("https://discord.gg/", Qt::CaseInsensitive))
			{
				auto authorID = message.author().id();
				::Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
					[this, message, &client, authorID, channel, content](bool result)
				{
					if (!result)
					{
						client.deleteMessage(message.channelId(), message.id());
						UmikoBot::Instance().createDm(authorID)
							.then([authorID, &client, message, content](const Channel& channel)
								{
									client.createMessage(channel.id(), "**Invitation link of servers aren't allowed in any channels on this server. Please take it to DMs!** Here is your message which you posted in the server:\n");
									client.createMessage(channel.id(), content);
								});
					}
				});
			}
		}

		for (auto& domain : dodgyDomainNames)
		{
			if (content.contains(domain))
			{
				if (content.contains(".com") || content.contains(".ru") || content.contains(".net"))
				{
					// TODO(fkp): Primary channel
					client.createMessage(message.channelId(), QString("<@%1> posted a message containing a dodgy URL in <#%2>. Removed!")
										 .arg(message.author().id()).arg(message.channelId()));
					client.deleteMessage(message.channelId(), message.id()).then([](){ printf("Deleted\n"); }).otherwise([](){ printf("Failed\n"); });
				}
			}
		}
	});

	Module::OnMessage(client, message);
}


void ModerationModule::OnSave(QJsonDocument& doc) const 
{
	QJsonObject json;
	QJsonObject moderation;

	moderation["invitationModeration"] = m_invitationModeration;

	QJsonObject warningsJson; // Holds a map of user:array_of_warnings
	for (auto user : warnings.keys())
	{
		const QList<UserWarning>& userWarnings = warnings.value(user);
		QJsonArray warningsArrayJson; // Holds the array of warnings for a specific user

		for (auto& warning : userWarnings)
		{
			QJsonObject warningJson;
			warningJson["warnedBy"] = QString::number(warning.warnedBy);
			warningJson["when"] = warning.when.toString();
			warningJson["message"] = warning.message;
			warningJson["expired"] = warning.expired;

			warningsArrayJson.append(warningJson);
		}

		warningsJson[QString::number(user)] = warningsArrayJson;
	}

	moderation["warnings"] = warningsJson;

	QJsonArray dodgyDomainsArray;
	for (auto& domain : dodgyDomainNames)
	{
		dodgyDomainsArray.append(domain);
	}
	moderation["dodgyDomainNames"] = dodgyDomainsArray;
	
	json["moderation"] = moderation;
	doc.setObject(moderation);
}

void ModerationModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject json = doc.object();
	QJsonObject moderation = json["moderation"].toObject();
	m_invitationModeration = json["invitationModeration"].toBool();

	dodgyDomainNames.clear();
	QJsonArray dodgyDomainsArray = json["dodgyDomainNames"].toArray();
	for (const auto& domain : dodgyDomainsArray)
	{
		dodgyDomainNames.insert(domain.toString());
	}

	// Loads warnings
	warnings.clear();
	QJsonObject warningsObj = json["warnings"].toObject();
	QStringList users = warningsObj.keys();

	for (auto& userString : users)
	{
		snowflake_t user = userString.toULongLong();
		QList<UserWarning> userWarnings;
		QJsonArray warningsArrayJson = warningsObj[userString].toArray();

		for (const auto& warningJson : warningsArrayJson)
		{
			QJsonObject warningObj = warningJson.toObject();

			UserWarning warning {
				warningObj["warnedBy"].toString().toULongLong(),
				QDateTime::fromString(warningObj["when"].toString()),
				warningObj["message"].toString(),
				warningObj["expired"].toBool()
			};

			userWarnings.append(warning);
		}

		warnings[user] = userWarnings;
	}

	checkWarningsExpiry();
}

unsigned int ModerationModule::countWarnings(snowflake_t user, bool countExpired)
{
	const QList<UserWarning>& userWarnings = warnings[user];

	if (countExpired)
	{
		return userWarnings.size();
	}

	unsigned int total = 0;
	
	for (const UserWarning& warning : userWarnings)
	{
		if (!warning.expired)
		{
			total += 1;
		}
	}

	return total;
}

// Checks expiry (warnings expire after 3 months)
void ModerationModule::checkWarningsExpiry()
{
	QDateTime now = QDateTime::currentDateTime();

	for (auto& userWarnings : warnings)
	{
		for (auto& warning : userWarnings)
		{
			if (!warning.expired && (warning.when.addMonths(3) <= now))
			{
				warning.expired = true;
			}
		}
	}
}
