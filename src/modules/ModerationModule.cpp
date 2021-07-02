#include "ModerationModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

using namespace Discord;

ModerationModule::ModerationModule()
	: Module("moderation", true)
{
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
}

void ModerationModule::OnMessage(Client& client, const Message& message)
{
	client.getChannel(message.channelId()).then(
	[this, message, &client](const Channel& channel)
	{
		if (m_invitationModeration)
		{
			if (message.content().contains("https://discord.gg/", Qt::CaseInsensitive))
			{
				auto authorID = message.author().id();
				::Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
					[this, message, &client, authorID, channel](bool result)
				{
					if (!result)
					{
						client.deleteMessage(message.channelId(), message.id());
						UmikoBot::Instance().createDm(authorID)
							.then([authorID, &client, message](const Channel& channel)
								{
									client.createMessage(channel.id(), "**Invitation link of servers aren't allowed in any channels on this server. Please take it to DMs!** Here is your message which you posted in the server:\n");
									client.createMessage(channel.id(), message.content());
								});
					}
				});
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
	json["moderation"] = moderation;

	doc.setObject(moderation);
}

void ModerationModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject json = doc.object();
	QJsonObject moderation = json["moderation"].toObject();
	m_invitationModeration = json["invitationModeration"].toBool();
}

unsigned int ModerationModule::countWarnings(snowflake_t user, bool countExpired)
{
	QList<UserWarning> userWarnings = warnings[user];

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
