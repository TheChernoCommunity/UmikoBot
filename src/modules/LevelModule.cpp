#include "LevelModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

LevelModule::LevelModule()
	: Module("levels", true)
{
	m_timer.setInterval(/*300 */ 1000);
	QObject::connect(&m_timer, &QTimer::timeout, 
		[this]()
	{
		for (auto it = m_exp.begin(); it != m_exp.end(); it++)
		{
			for (GuildLevelData& data : it.value())
			{
				if (data.messageCount > 0) 
				{
					data.messageCount = 0;
					data.exp += qrand() % 31 + 15;
				}
			}
		}
	});
	m_timer.start();

	QTime now = QTime::currentTime();
	qsrand(now.msec());

	RegisterCommand(Commands::LEVEL_MODULE_TOP, "top",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		QString prefix = GuildSettings::GetGuildSetting(channel.guildId()).prefix;

		if (args.first() != prefix + "top")
			return;

		if (args.size() == 2) {
			qSort(m_exp[channel.guildId()].begin(), m_exp[channel.guildId()].end(),
				[](const LevelModule::GuildLevelData& v1, const LevelModule::GuildLevelData& v2) -> bool
			{
				return v1.exp > v2.exp;
			});

			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Top " + args.back());

			QString desc = "";

			bool ok;
			int count = args.back().toInt(&ok);

			if (!ok) 
			{
				client.createMessage(message.channelId(), "Invalid count");
				return;
			}

			if (count > 30)
				count = 30;

			for (int i = 0; i < count; i++) 
			{
				if (i >= m_exp[channel.guildId()].size())
				{
					embed.setTitle("Top " + QString::number(i));
					break;
				}

				LevelModule::GuildLevelData& curr = m_exp[channel.guildId()][i];
				desc += QString::number(i + 1) + ". ";
				desc += reinterpret_cast<UmikoBot*>(&client)->GetNick(channel.guildId(), m_exp[channel.guildId()][i].user);
				desc += " - " + QString::number(curr.exp) + "\n";
			}

			embed.setDescription(desc);

			client.createMessage(message.channelId(), embed);
		}
		else
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help top");
			QString description = bot->GetCommandHelp("top", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		}
	});

	RegisterCommand(Commands::LEVEL_MODULE_RANK, "rank",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "rank")
			return;

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help rank");
			QString description = bot->GetCommandHelp("rank", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		};

		if (args.last() == "list")
		{
			QList<LevelRank> ranks = setting->ranks;
			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Rank list");

			QString description = "";
			
			if (ranks.size() == 0)
				description = "No ranks found!";
			else
				for (int i = 0; i < ranks.size(); i++)
					description += ranks[i].name + " id " + QString::number(i) + " minimum level: " + QString::number(ranks[i].minimumLevel) + "\n";
			
			embed.setDescription(description);
			client.createMessage(message.channelId(), embed);
		}
		else if (args[1] == "add" && args.size() > 3)
		{
			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[args, &client, message, channel](bool result)
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result)
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command");
					return;
				}

				bool ok;
				unsigned int minimumLevel = args[2].toUInt(&ok);
				if (!ok)
				{
					client.createMessage(message.channelId(), "Invalid minimum level");
					return;
				}
				QString name = "";
				for (int i = 3; i < args.size(); i++)
				{
					name += args[i];
					if (i < args.size() - 1)
						name += " ";
				}

				LevelRank rank = { name, minimumLevel };

				for (int i = 0; i < setting->ranks.size(); i++)
					if (setting->ranks[i].minimumLevel == minimumLevel)
					{
						client.createMessage(message.channelId(), "Cannot add rank, minimum level already used");
						return;
					}

				setting->ranks.push_back(rank);
				qSort(setting->ranks.begin(), setting->ranks.end(),
					[](const LevelRank& v1, const LevelRank& v2) -> bool
				{
					return v1.minimumLevel < v2.minimumLevel;
				});

				for (int i = 0; i < setting->ranks.size(); i++)
					if (setting->ranks[i].name == name)
					{
						client.createMessage(message.channelId(), "Added rank " + name + " with id " + QString::number(i));
						break;
					}
			});
			
		}
		else
		{
			printHelp();
		}
	});
}

void LevelModule::OnSave(QJsonDocument& doc) const
{
	QJsonObject json;
	for (auto it = m_exp.begin(); it != m_exp.end(); it++) {
		QJsonObject level;

		for (const GuildLevelData& user : it.value()) {
			level[QString::number(user.user)] = user.exp;
		}
		json[QString::number(it.key())] = level;
	}

	doc.setObject(json);
}

void LevelModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject json = doc.object();

	QStringList guildIds = json.keys();

	for (const QString& guild : guildIds)
	{
		snowflake_t guildId = guild.toULongLong();

		QJsonObject levels = json[guild].toObject();

		QStringList userids = levels.keys();

		for (const QString& user : userids)
			m_exp[guildId].append({ user.toULongLong(), levels[user].toInt(), 0 });
	}
}

void LevelModule::StatusCommand(QString& result, snowflake_t guild, snowflake_t user)
{
	GuildSetting s = GuildSettings::GetGuildSetting(guild);

	if (s.ranks.size() > 0) {
		result += "Rank: ...\n";
	}
	unsigned int xp = GetData(guild, user).exp;
	result += "Total exp: " + QString::number(xp) + "\n";
	
	unsigned int xpRequirement = LEVELMODULE_EXP_REQUIREMENT;
	unsigned int level = 1;
	while (xp > xpRequirement && level < LEVELMODULE_MAXIMUM_LEVEL) {
		level++;
		xp -= xpRequirement;
		xpRequirement *= 1.5;
	}

	if (level >= LEVELMODULE_MAXIMUM_LEVEL)
	{
		xp = 0;
		level = LEVELMODULE_MAXIMUM_LEVEL;
		xpRequirement = 0;
	}

	result += "Level: " + QString::number(level) + "\n";
	if(xp == 0 && xpRequirement == 0)
		result += QString("Exp needed for rankup: Maximum Level\n");
	else
		result += QString("Exp needed for rankup: %1/%2\n").arg(QString::number(xp), QString::number(xpRequirement));
	result += "\n";
}

void LevelModule::OnMessage(Discord::Client& client, const Discord::Message& message) 
{
	Module::OnMessage(client, message);

	client.getChannel(message.channelId()).then(
		[this, message](const Discord::Channel& channel) 
	{
		if (message.author().bot())
			return;

		for (GuildLevelData& data : m_exp[channel.guildId()]) {
			if (data.user == message.author().id()) {
				data.messageCount++;
				return;
			}
		}

		m_exp[channel.guildId()].append({ message.author().id(), 0, 1 });
	});
}

LevelModule::GuildLevelData LevelModule::GetData(snowflake_t guild, snowflake_t user)
{
	for (GuildLevelData data : m_exp[guild])
	{
		if (data.user == user) {
			return data;
		}
	}
	return { user, 0,0 };
}
