#include "UserModule.h"
#include "CurrencyModule.h"
#include "UmikoBot.h"

#define descriptionTimeout 60

UserModule::UserModule()
	: Module("users", true)
{
	RegisterCommand(Commands::USER_MODULE_WHO_IS, "whois",
					[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		snowflake_t authorId = message.author().id();

		if (args.size() != 2)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!**");
			return;
		}

		QList<Discord::User> mentions = message.mentions();
		snowflake_t userId;

		if (mentions.size() > 0)
		{
			userId = mentions[0].id();
		}
		else
		{
			userId = UmikoBot::Instance().GetUserFromArg(channel.guildId(), args, 1);
			if (userId == 0)
			{
				client.createMessage(message.channelId(), "Could not find user!");
				return;
			}
		}

		DescriptionData& data = guildDescriptionData[channel.guildId()];

		if (data.isBeingUsed && data.userId == userId)
		{
			QString msg = "**Sorry,";
			msg += UmikoBot::Instance().GetUsername(channel.guildId(), data.userId);
			msg += " is currently setting their description.**\nTry again later...";
			client.createMessage(message.channelId(), msg);
			return;
		}

		UserDescription& desc = userDescriptions[channel.guildId()][getUserIndex(channel.guildId(), userId)];
		QString msg = formDescriptionMessage(desc);

		if (msg.isEmpty())
		{
			msg = UmikoBot::Instance().GetName(channel.guildId(), userId) + " prefers an air of mystery around them...";
			client.createMessage(message.channelId(), msg);
		}
		else
		{
			UmikoBot::Instance().GetAvatar(channel.guildId(), userId).then(
				[this, msg, userId, channel, &client, message](const QString& icon)
				{
					Discord::Embed embed;
					QString name = UmikoBot::Instance().GetName(channel.guildId(), userId);
					embed.setAuthor(Discord::EmbedAuthor(name, "", icon));
					embed.setColor(qrand() % 16777216);
					embed.setTitle("Description");
					embed.setDescription(msg);

					client.createMessage(message.channelId(), embed);
				}
			);
		}
	});

	RegisterCommand(Commands::USER_MODULE_I_AM, "iam",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		snowflake_t authorId = message.author().id();
		snowflake_t guildId = channel.guildId();
		
		DescriptionData& data = guildDescriptionData[guildId];

		if (data.isBeingUsed)
		{
			QString msg = "**Sorry, ";
			msg += UmikoBot::Instance().GetUsername(channel.guildId(), data.userId);
			msg += " is currently setting their description.**\nTry again later...";
			client.createMessage(message.channelId(), msg);
			return;
		}
		
		if (args.size() > 1)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!**");
			return;
		}

		data.isBeingUsed = true;
		data.userId = authorId;
		data.messageId = message.id();
		data.questionUpTo = 0;
		data.currentUserDescription = &userDescriptions[guildId][getUserIndex(guildId, authorId)];
		data.oldUserDescription = *data.currentUserDescription;

		data.timer = new QTimer();
		data.timer->setInterval(descriptionTimeout * 1000);
		QObject::connect(data.timer, &QTimer::timeout, [this, &client, &data, message]() {
			if (data.isBeingUsed)
			{
				data.isBeingUsed = false;
				delete data.timer;
				data.timer = nullptr;
				client.createMessage(message.channelId(), "**Description timeout due to no valid response.**");
			}
		});
		data.timer->start();
		
		QString msg =
			"**Tell me about yourself!** At any time, you can type:\n"
			"\t\tskip - to skip the current question (and clear the answer)\n"
			"\t\tcontinue - to skip the current question (and leave the answer)\n"
			"\t\tcancel - to revert all changes\n\n";
		msg += descriptionQuestions[0].first;
		client.createMessage(message.channelId(), msg);
	});

	RegisterCommand(Commands::USER_MODULE_STATS, "stats",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "stats")
			return;

		if (args.size() > 2)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!**");
			return;
		}

		snowflake_t userId;

		if (args.size() == 1)
		{
			userId = message.author().id();
		}
		else if (message.mentions().size() > 0)
		{
			userId = message.mentions()[0].id();
		}
		else
		{
			userId = UmikoBot::Instance().GetUserFromArg(channel.guildId(), args, 1);
			if (userId == 0)
			{
				client.createMessage(message.channelId(), "Could not find user!");
				return;
			}
		}

		UmikoBot::Instance().getGuildMember(channel.guildId(), userId).then(
			[this, &client, userId, channel, message](const Discord::GuildMember& member)
		{
			UmikoBot::Instance().GetAvatar(channel.guildId(), userId).then(
				[this, userId, channel, &client, message, member](const QString& icon)
			{
				Discord::Embed embed;
				QString name = UmikoBot::Instance().GetName(channel.guildId(), userId);
				embed.setAuthor(Discord::EmbedAuthor(name + "'s Statistics", "", icon));
				embed.setColor(qrand() % 16777216);

				QString desc = "**General:**\n";
				desc += "Date Joined: **" + member.joinedAt().date().toString() + "**\n";

				CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
				if (currencyModule)
				{
					const CurrencyModule::CurrencyConfig& serverConfig = currencyModule->getServerData(channel.guildId());
					const CurrencyModule::UserCurrency& userCurrency = currencyModule->getUserData(channel.guildId(), userId);

					desc += "\n**Currency:**\n";
					desc += "Max " + serverConfig.currencyName + "s: **" + QString::number(userCurrency.maxCurrency) + " " + serverConfig.currencySymbol + "**\n";
					desc += "`daily`s claimed: **" + QString::number(userCurrency.numberOfDailysClaimed) + "**\n";
					desc += "`claim`s claimed: **" + QString::number(userCurrency.numberOfGiveawaysClaimed) + "**\n";
				}

				embed.setDescription(desc);

				client.createMessage(message.channelId(), embed);
			});
		});

	});

	
	RegisterCommand(Commands::USER_MODULE_SAFE_MODE, "safemode", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		snowflake_t authorId = message.author().id();

		if (args.size() < 1)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!**");
			return;
		}

		for (int i = 0; i < args.size(); i++)
		{
			auto text = args[i].toLower();
			QString next = "";
			if (i + 1 < args.size())
			{
				next = args[i + 1];
			}

			if (text == "--buy")
			{
				CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
				if (currencyModule)
				{
					const CurrencyModule::CurrencyConfig& serverConfig = currencyModule->getServerData(channel.guildId());
					const CurrencyModule::UserCurrency& userCurrency = currencyModule->getUserData(channel.guildId(), authorId);


					currencyModule->getUserData(channel.guildId(), authorId).setCurrency(-+150);

					userSafeModes.HasSafeMode = true;
					userSafeModes.SafeModeAmts.push_back(0);
					userSafeModes.pricePayed = 150;
					userSafeModes.userId = message.author().id();
				}
			}
			else if (text == "--on")
			{
				CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
				if (currencyModule)
				{
					const CurrencyModule::CurrencyConfig& serverConfig = currencyModule->getServerData(channel.guildId());
					const CurrencyModule::UserCurrency& userCurrency = currencyModule->getUserData(channel.guildId(), authorId);

					if (userSafeModes.HasSafeMode)
					{
						if (!userCurrency.canSteal && !userCurrency.canClaimFreebies)
						{
							client.createMessage(message.channelId(), "**You are already in safemode!**");
							return;
						}
						else if (userCurrency.canSteal && userCurrency.canClaimFreebies)
						{
							currencyModule->setSafeMode(channel, message, true);

							if (userCurrency.jailTimer->isActive())
							{
								client.createMessage(message.channelId(), "**You can't go into safemode while in jail!**");
								return;
							}

							client.createMessage(message.channelId(), "**Safe Mode on!**");
						}
					}
					else 
					{
						client.createMessage(message.channelId(), "**You don't have a safe mode!**");
						return;
					}
				}
			}
			else if (text == "--off")
			{
				CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
				if (currencyModule)
				{
					const CurrencyModule::CurrencyConfig& serverConfig = currencyModule->getServerData(channel.guildId());
					const CurrencyModule::UserCurrency& userCurrency = currencyModule->getUserData(channel.guildId(), authorId);

					if (userCurrency.canSteal && userCurrency.canClaimFreebies)
					{
						client.createMessage(message.channelId(), "**You are not in safemode!**");
						return;
					}
					else if (!userCurrency.canSteal && !userCurrency.canClaimFreebies)
					{
						currencyModule->setSafeMode(channel, message, false);
						client.createMessage(message.channelId(), "**Safe Mode off!**");
					}
				}
			}
			else if(text.isEmpty())
			{
				client.createMessage(message.channelId(), "**Wrong Usage of Command!**");
				return;
			}
		}

	});
	
}

void UserModule::OnSave(QJsonDocument& doc) const
{
	// User data
	QJsonObject docObj;
	
	for (auto server : userDescriptions.keys())
	{
		QJsonObject serverJson;

		for (const UserDescription& user : userDescriptions[server])
		{
			if (formDescriptionMessage(user) == "")
				continue;
			
			QJsonObject obj;
			obj["name"] = user.name;
			obj["location"] = user.location;
			obj["industry"] = user.industry;
			obj["programmingInterests"] = user.programmingInterests;
			obj["currentlyWorkingOn"] = user.currentlyWorkingOn;
			obj["githubLink"] = user.githubLink;

			serverJson[QString::number(user.userId)] = obj;
		}

		docObj[QString::number(server)] = serverJson;
	}

	doc.setObject(docObj);
}

void UserModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject docObj = doc.object();
	QStringList servers = docObj.keys();

	userDescriptions.clear();

	// User data
	for (auto server : servers)
	{
		snowflake_t guildId = server.toULongLong();
		QJsonObject obj = docObj[server].toObject();
		QStringList users = obj.keys();
		QList<UserDescription> descriptions;

		for (const QString& user : users)
		{
			auto userObj = obj[user].toObject();
			
			UserDescription description {
				user.toULongLong(),
				userObj["name"].toString(),
				userObj["location"].toString(),
				userObj["industry"].toString(),
				userObj["programmingInterests"].toString(),
				userObj["currentlyWorkingOn"].toString(),
				userObj["githubLink"].toString(),
			};

			descriptions.append(description);
		}

		userDescriptions.insert(guildId, descriptions);
	}
}

void UserModule::OnMessage(Discord::Client& client, const Discord::Message& message)
{
	client.getChannel(message.channelId()).then(
		[this, message, &client](const Discord::Channel& channel)
		{
			snowflake_t guildId = channel.guildId();
			
			if (guildId == 0 || message.author().bot())
			{
				// It's a bot / we're in a DM
				return;
			}

			DescriptionData& descriptionData = guildDescriptionData[guildId];
			snowflake_t authorId = message.author().id();
			QString messageContent = message.content();
			
			if (descriptionData.messageId == message.id())
			{
				// This is the !iam command, ignore it
				return;
			}

			if (descriptionData.isBeingUsed && descriptionData.userId == authorId)
			{
				QString contentToWrite = messageContent;
				bool shouldWrite = true;
				
				if (messageContent == "skip")
				{
					contentToWrite = "";
				}
				else if (messageContent == "continue")
				{
					shouldWrite = false;
				}
				else if (messageContent == "cancel")
				{
					*descriptionData.currentUserDescription = descriptionData.oldUserDescription;
					descriptionData.isBeingUsed = false;
					delete descriptionData.timer;
					descriptionData.timer = nullptr;

					client.createMessage(message.channelId(), "**Cancelled setting description\n**Reverting to old values.");
					return;
				}

				if (shouldWrite)
				{
					descriptionQuestions[descriptionData.questionUpTo].second(*descriptionData.currentUserDescription, contentToWrite);
				}

				descriptionData.questionUpTo += 1;
				descriptionData.timer->start();

				if (descriptionData.questionUpTo == descriptionQuestions.size())
				{
					client.createMessage(message.channelId(), "**All done!** Thanks for your time.");
					descriptionData.isBeingUsed = false;
					delete descriptionData.timer;
					descriptionData.timer = nullptr;
					
					return;
				}

				QString msg = descriptionQuestions[descriptionData.questionUpTo].first;
				client.createMessage(message.channelId(), msg);
			}
		}
	);

	Module::OnMessage(client, message);
}

snowflake_t UserModule::getUserIndex(snowflake_t guild, snowflake_t id)
{
	QList<UserDescription>& guildDescriptions = userDescriptions[guild];

	for (auto it = guildDescriptions.begin(); it != guildDescriptions.end(); ++it)
	{
		if (it->userId == id)
		{
			return std::distance(guildDescriptions.begin(), it);
		}
	}

	// If user is not added to the system, make a new one
	guildDescriptions.append(UserDescription { id, "" });
	return std::distance(guildDescriptions.begin(), std::prev(guildDescriptions.end()));
}

QString UserModule::formDescriptionMessage(const UserDescription& desc) const
{
	QString msg;

	if (desc.name != "")
		msg += "**Name: **" + desc.name + "\n";
		
	if (desc.location != "")
		msg += "**Location: **" + desc.location + "\n";
		
	if (desc.industry != "")
		msg += "**Industry: **" + desc.industry + "\n";
		
	if (desc.programmingInterests != "")
		msg += "**Programming Interests: **" + desc.programmingInterests + "\n";
		
	if (desc.currentlyWorkingOn != "")
		msg += "**Currently working on: **" + desc.currentlyWorkingOn + "\n";
		
	if (desc.githubLink != "")
		msg += "**GitHub: **" + desc.githubLink + "\n";

	return msg;
}
