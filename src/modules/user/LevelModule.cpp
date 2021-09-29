#include "LevelModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

using namespace Discord;

LevelModule::LevelModule(UmikoBot* client)
	: Module("levels", true), m_client(client)
{
	m_timer.setInterval(30 * 1000);
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
					data.exp += 10 + qrand() % 6;
				}
			}
		}
	});
	m_timer.start();

	QTime now = QTime::currentTime();
	qsrand(now.msec());

	RegisterCommand(Commands::LEVEL_MODULE_TOP, "top",
		[this](Client& client, const Message& message, const Channel& channel)
	{
		auto& exp = m_exp[channel.guildId()];
		QStringList args = message.content().split(' ');
		GuildSetting s = GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = s.prefix;

		if (args.size() == 2) {
			qSort(exp.begin(), exp.end(),
				[](const LevelModule::GuildLevelData& v1, const LevelModule::GuildLevelData& v2)
			{
				return v1.exp > v2.exp;
			});
			Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Top " + args.back());

			QString desc = "";

			bool ok;
			int count = args.back().toInt(&ok);

			if (!ok) 
			{
				client.createMessage(message.channelId(), "**Invalid Count**");
				return;
			}

			if (count < 1) 
			{
				client.createMessage(message.channelId(), "**Invalid Count**");
				return;
			}

			if (count > 30)
			{ 
				client.createMessage(message.channelId(), "**Invalid Count**: The max count is `30`");
				return;
			}

			unsigned int numberOfDigits = QString::number(std::min(count, exp.size())).size();

			for (int i = 0; i < count; i++) 
			{
				if (i >= exp.size())
				{
					embed.setTitle("Top " + QString::number(i));
					break;
				}

				LevelModule::GuildLevelData& curr = exp[i];
				desc += "`" + QString::number(i + 1).rightJustified(numberOfDigits, ' ') + "`) ";
				desc += "**" + reinterpret_cast<UmikoBot*>(&client)->GetName(channel.guildId(), exp[i].user) + "**";

				unsigned int xp = GetData(channel.guildId(), exp[i].user).exp;

				unsigned int xpRequirement = s.expRequirement;
				unsigned int level = 1;
				while (xp > xpRequirement && level < s.maximumLevel) {
					level++;
					xp -= xpRequirement;
					xpRequirement *= s.growthRate;
				}

				if (level >= s.maximumLevel)
					level = s.maximumLevel;

				desc += " - Level " + QString::number(level) + "\n";
			}

			embed.setDescription(desc);

			client.createMessage(message.channelId(), embed);
		}
		else if (args.size() == 3)
		{
			qSort(exp.begin(), exp.end(),
				[](const LevelModule::GuildLevelData& v1, const LevelModule::GuildLevelData& v2) -> bool
			{
				return v1.exp > v2.exp;
			});

			bool ok1, ok2;
			int count1 = args[1].toInt(&ok1);
			int count2 = args[2].toInt(&ok2);

			if (!ok1 || !ok2)
			{
				client.createMessage(message.channelId(), "**Invalid Count**");
				return;
			}

			if (count1 < 1 || count2 < 1) 
			{
				client.createMessage(message.channelId(), "**Invalid Count**");
				return;
			}


			if (count2 < count1) 
			{
				client.createMessage(message.channelId(), "**Invalid Range**");
				return;
			}


			if (count2 - count1 > 30) {
				client.createMessage(message.channelId(), "**Invalid Count**: The max offset is `30`");
				return;
			}

			Embed embed;
			embed.setColor(qrand() % 16777216);
			if (count2 == count1) 
			{
				embed.setTitle("Top " + QString::number(count1));
			}
			else 
			{
				embed.setTitle("Top from " + QString::number(count1) + " to " + QString::number(count2));
			}
			

			QString desc = "";

			if (count1 > exp.size())
			{
				client.createMessage(channel.id(), "**Not enough members to create the list.**");
				return;
			}

			unsigned int numberOfDigits = QString::number(std::min(count2, exp.size())).size();

			for (int i = count1 - 1; i < count2; i++)
			{

				if (i >= exp.size())
				{
					embed.setTitle("Top from " + QString::number(count1) + " to " + QString::number(i));
					break;
				}

				LevelModule::GuildLevelData& curr = exp[i];
				desc += "`" + QString::number(i + 1).rightJustified(numberOfDigits, ' ') + "`) ";
				desc += "**" + reinterpret_cast<UmikoBot*>(&client)->GetName(channel.guildId(), exp[i].user) + "**";

				unsigned int xp = GetData(channel.guildId(), exp[i].user).exp;

				unsigned int xpRequirement = s.expRequirement;
				unsigned int level = 1;
				while (xp > xpRequirement && level < s.maximumLevel) {
					level++;
					xp -= xpRequirement;
					xpRequirement *= s.growthRate;
				}

				if (level >= s.maximumLevel)
					level = s.maximumLevel;

				desc += " - Level " + QString::number(level) + "\n";
			}

			embed.setDescription(desc);

			client.createMessage(message.channelId(), embed);
		}
		else
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help top");
			QString description = bot->GetCommandHelp("top", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		}
	});

	initiateAdminCommands();
}

void LevelModule::OnSave(QJsonDocument& doc) const
{
	QJsonObject json;
	QJsonObject levels;
	QJsonObject backups;

	for (auto it = m_exp.begin(); it != m_exp.end(); it++)
		for (int i = 0; i < it.value().size(); i++) 
			if (m_client->GetName(it.key(), it.value()[i].user) == "")
			{
				bool found = false;
				for (GuildLevelData& data : m_backupexp[it.key()])
					if (data.user == it.value()[i].user) {
						data.exp += it.value()[i].exp;
						found = true;
						break;
					}

				if (!found)
					m_backupexp[it.key()].append(it.value()[i]);

				it.value().erase(it.value().begin() + i);
			}

	for (auto it = m_backupexp.begin(); it != m_backupexp.end(); it++)
		for (int i = 0; i < it.value().size(); i++)
			if (m_client->GetName(it.key(), it.value()[i].user) != "")
			{
				bool found = false;
				for (GuildLevelData& data : m_exp[it.key()])
					if (data.user == it.value()[i].user) {
						data.exp += it.value()[i].exp;
						found = true;
						break;
					}
				
				if (!found)
					m_exp[it.key()].append(it.value()[i]);

				it.value().erase(it.value().begin() + i);
			}

	if(m_exp.size() > 0)
		for (auto it = m_exp.begin(); it != m_exp.end(); it++) 
		{
			QJsonObject level;

			for (int i = 0; i < it.value().size(); i++)
				level[QString::number(it.value()[i].user)] = it.value()[i].exp;

			levels[QString::number(it.key())] = level;
		}

	if(m_backupexp.size() > 0)
		for (auto it = m_backupexp.begin(); it != m_backupexp.end(); it++)
		{
			QJsonObject backup;

			for (int i = 0; i < it.value().size(); i++)
				backup[QString::number(it.value()[i].user)] = it.value()[i].exp;

			backups[QString::number(it.key())] = backup;
		}

	json["levels"] = levels;
	json["backups"] = backups;
	doc.setObject(json);
}

void LevelModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject json = doc.object();

	QJsonObject backups = json["backups"].toObject();
	QJsonObject levels = json["levels"].toObject();

	QStringList guildIds = levels.keys();

	for (const QString& guild : guildIds)
	{
		snowflake_t guildId = guild.toULongLong();

		QJsonObject level = levels[guild].toObject();
		QJsonObject backup = backups[guild].toObject();

		for (const QString& user : level.keys())
			m_exp[guildId].append({ user.toULongLong(), level[user].toInt(), 0 });

		for (const QString& user : backup.keys())
			m_backupexp[guildId].append({ user.toULongLong(), backup[user].toInt(), 0 });
	}
}

ExpLevelData LevelModule::ExpToLevel(snowflake_t guild, unsigned int exp)
{
	GuildSetting s = GuildSettings::GetGuildSetting(guild);

	ExpLevelData res;

	res.xpRequirement = s.expRequirement;
	res.level = 1;
	res.exp = exp;

	while (res.exp > res.xpRequirement && res.level < s.maximumLevel)
	{
		res.level++;
		res.exp -= res.xpRequirement;
		res.xpRequirement *= s.growthRate;
	}

	if (res.level >= s.maximumLevel)
	{
		res.exp = 0;
		res.level = s.maximumLevel;
		//res.xpRequirement = 0;
	}

	return res;
}

void LevelModule::StatusCommand(QString& result, snowflake_t guild, snowflake_t user)
{
	GuildSetting s = GuildSettings::GetGuildSetting(guild);

	unsigned int xp = GetData(guild, user).exp;

	ExpLevelData res = ExpToLevel(guild, xp);

	// Sort to get leaderboard index
	auto& exp = m_exp[guild];
	qSort(exp.begin(), exp.end(),
			[](const LevelModule::GuildLevelData& v1, const LevelModule::GuildLevelData& v2)
		{
			return v1.exp > v2.exp;
		});
	int leaderboardIndex = -1;
	for (int i = 0; i < exp.size(); ++i)
	{
		if (exp.at(i).user == user)
		{
			leaderboardIndex = i;
			break;
		}
	}

	unsigned int rankLevel = res.level;
	QString rank = "";
	if (s.ranks.size() > 0) {
		for (int i = 0; i < s.ranks.size() - 1; i++)
		{
			if (rankLevel >= s.ranks[i].minimumLevel && rankLevel < s.ranks[i + 1].minimumLevel)
			{
				rank = s.ranks[i].name;
				break;
			}
		}
		if (rank == "")
			rank = s.ranks[s.ranks.size() - 1].name;

		if (leaderboardIndex >= 0)
			result += "Rank: " + rank + " (#" + QString::number(leaderboardIndex + 1) + ")\n";
		else
			result += "Rank: " + rank + "\n";
	}
	else if (leaderboardIndex >= 0) {
		result += "Rank: #" + QString::number(leaderboardIndex + 1) + "\n";
	}

	result += "Level: " + QString::number(res.level) + "\n";
	result += "Total XP: " + QString::number(GetData(guild, user).exp) + "\n";
	if(res.level < s.maximumLevel)
		result += QString("XP until next level: %1\n").arg(res.xpRequirement - res.exp);
	result += "\n";
}

void LevelModule::OnMessage(Client& client, const Message& message) 
{
	Module::OnMessage(client, message);

	client.getChannel(message.channelId()).then(
		[this, message](const Channel& channel) 
	{
		auto& exp = m_exp[channel.guildId()];

		if (!GuildSettings::IsModuleEnabled(channel.guildId(), GetName(), IsEnabledByDefault()))
			return;

		if (message.author().bot())
			return;

		if(!GuildSettings::ExpAllowed(channel.guildId(), channel.id()))
			return;

		QList<Command> commands = UmikoBot::Instance().GetAllCommands();
		GuildSetting setting = GuildSettings::GetGuildSetting(channel.guildId());

		for (Command& command : commands)
		{
			if (message.content().startsWith(setting.prefix + command.name))
				return;
		}

		for (GuildLevelData& data : exp) {
			if (data.user == message.author().id()) {
				data.messageCount++;
				return;
			}
		}

		exp.append({ message.author().id(), 0, 1 });
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
