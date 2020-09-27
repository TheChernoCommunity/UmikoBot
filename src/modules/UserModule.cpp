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

		snowflake_t victimId = mentions[0].id();

		if (userDescriptions[channel.guildId()][getUserIndex(channel.guildId(), victimId)].description == "")
		{
			QString msg = UmikoBot::Instance().GetUsername(channel.guildId(), victimId) + " prefers an air of mystery around them...";
			client.createMessage(message.channelId(), msg);
		}
		else
		{
			QString msg = userDescriptions[channel.guildId()][getUserIndex(channel.guildId(), victimId)].description;
			client.createMessage(message.channelId(), msg);
		}
	});

	RegisterCommand(Commands::USER_MODULE_I_AM, "iam",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;
		snowflake_t authorId = message.author().id();

		if (args.first() != prefix + "iam")
			return;
		
		if (args.size() <= 1)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!**");
			return;
		}

		// There might be a better way to do this that I don't know of.
		// I just need to get the rest of the message (excluding the 
		// command) as a string
		QString description = "**" + UmikoBot::Instance().GetUsername(channel.guildId(), authorId) + "'s Description:**\n";
		for (int i = 1; i < args.size(); i++)
		{
			description += args[i] + " ";
		}
		userDescriptions[channel.guildId()][getUserIndex(channel.guildId(), authorId)].description = description;

		client.createMessage(message.channelId(), "Description set!");
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
			if (user.description == "")
				continue;
			
			QJsonObject obj;
			obj["description"] = user.description;

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
			UserDescription description {
				user.toULongLong(),
				obj[user].toObject()["description"].toString(),
			};

			descriptions.append(description);
		}

		userDescriptions.insert(guildId, descriptions);
	}
}
