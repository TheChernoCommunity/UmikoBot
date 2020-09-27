#include "UserModule.h"
#include "UmikoBot.h"

UserModule::UserModule()
	: Module("users", true)
{
	RegisterCommand(Commands::USER_MODULE_WHO_IS, "whois",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;
		snowflake_t authorId = message.author().id();

		if (args.first() != prefix + "whois")
			return;
		
		if (args.size() > 2)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!**");
			return;
		}

		QList<Discord::User> mentions = message.mentions();
		if (mentions.size() == 0)
		{
			// TODO(fkp): Check for names
			client.createMessage(message.channelId(), "Please **mention someone** whose description you want to see.");
			return;
		}

		snowflake_t userId = mentions[0].id();
		UserDescription& desc = userDescriptions[channel.guildId()][getUserIndex(channel.guildId(), userId)];
		QString msg = formDescriptionMessage(desc);
		
		if (msg == "")
		{
			msg = UmikoBot::Instance().GetUsername(channel.guildId(), userId) + " prefers an air of mystery around them...";
		}
		else
		{
			msg = "**" + UmikoBot::Instance().GetUsername(channel.guildId(), userId) + "'s Description**\n" + msg;
		}
		
		client.createMessage(message.channelId(), msg);
	});

	RegisterCommand(Commands::USER_MODULE_I_AM, "iam",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;
		snowflake_t authorId = message.author().id();
		snowflake_t guildId = channel.guildId();

		if (args.first() != prefix + "iam")
			return;
		
		if (args.size() > 1)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!**");
			return;
		}

		DescriptionData& data = guildDescriptionData[guildId];
		data.isBeingUsed = true;
		data.userId = authorId;
		data.timer = nullptr; // TODO(fkp): Timer
		data.questionUpTo = 0;
		data.currentUserDescription = &userDescriptions[guildId][getUserIndex(guildId, authorId)];
		data.oldUserDescription = *data.currentUserDescription;
		
		QString msg =
			"**Tell me about yourself!** At any time, you can type:\n"
			"\tskip - to skip the current question (and clear the answer)\n"
			"\tcontinue - to skip the current question (and leave the answer)\n"
			"\tcancel - to revert all changes\n\n";
		msg += descriptionQuestions[0];
		client.createMessage(message.channelId(), msg);
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
					return;
				}

				if (shouldWrite)
				{
					switch (descriptionData.questionUpTo)
					{
					case 0:
						descriptionData.currentUserDescription->name = contentToWrite;
						break;
					case 1:
						descriptionData.currentUserDescription->location = contentToWrite;
						break;
					case 2:
						descriptionData.currentUserDescription->industry = contentToWrite;
						break;
					case 3:
						descriptionData.currentUserDescription->programmingInterests = contentToWrite;
						break;
					case 4:
						descriptionData.currentUserDescription->currentlyWorkingOn = contentToWrite;
						break;
					case 5:
						descriptionData.currentUserDescription->githubLink = contentToWrite;
						break;
					}
				}

				descriptionData.questionUpTo += 1;

				if (descriptionData.questionUpTo == descriptionQuestions.size())
				{
					client.createMessage(message.channelId(), "**All done!** Thanks for your time.");
					descriptionData.isBeingUsed = false;
					delete descriptionData.timer;
					
					return;
				}

				QString msg = descriptionQuestions[descriptionData.questionUpTo];
				client.createMessage(message.channelId(), msg);
			}
		}
	);

	Module::OnMessage(client, message);
}
