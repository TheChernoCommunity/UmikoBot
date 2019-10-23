#include "LevelModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

#include <QtMath>

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
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		auto& exp = m_exp[channel.guildId()];
		QStringList args = message.content().split(' ');
		GuildSetting s = GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = s.prefix;

		if (args.first() != prefix + "top")
			return;

		
		if (args.size() == 2) {
			qSort(exp.begin(), exp.end(),
				[](const LevelModule::GuildLevelData& v1, const LevelModule::GuildLevelData& v2)
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
				if (i >= exp.size())
				{
					embed.setTitle("Top " + QString::number(i));
					break;
				}

				LevelModule::GuildLevelData& curr = exp[i];
				desc += QString::number(i + 1) + ". ";
				desc += reinterpret_cast<UmikoBot*>(&client)->GetName(channel.guildId(), exp[i].user);

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
				client.createMessage(message.channelId(), "Invalid count");
				return;
			}
			if (count1 < 1)
				count1 = 1;

			if (count2 > 30)
				count2 = 30;

			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Top from " + QString::number(count1) + " to " + QString::number(count1 + count2 - 1));

			QString desc = "";

			if (count1 > exp.size())
			{
				client.createMessage(channel.id(), "Not enough members to create the top.");
				return;
			}

			for (int i = count1 - 1; i < count1 + count2 - 1; i++)
			{
				if (i >= exp.size())
				{
					embed.setTitle("Top from " + QString::number(count1) + " to " + QString::number(i));
					break;
				}

				LevelModule::GuildLevelData& curr = exp[i];
				desc += QString::number(i + 1) + ". ";
				desc += reinterpret_cast<UmikoBot*>(&client)->GetName(channel.guildId(), exp[i].user);

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
		
		if (args.first() != prefix + "rank")
			return;
		
		if (args.size() < 2)
		{
			printHelp();
			return;
		}

		if (args.last() == "list" && args.size() == 2)
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
			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[args, &client, message, channel](bool result)
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result)
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}

				bool ok;
				unsigned int minimumLevel = args[2].toUInt(&ok);
				if (!ok)
				{
					client.createMessage(message.channelId(), "Invalid minimum level.");
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
						client.createMessage(message.channelId(), "Cannot add rank, minimum level already used.");
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
		else if (args[1] == "remove" && args.size() == 3)
			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[args, &client, message, channel](bool result)
			{
				if (!result)
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}

				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());

				bool ok;
				unsigned int id = args[2].toUInt(&ok);
				if (!ok)
				{
					client.createMessage(message.channelId(), "Invalid id.");
					return;
				}

				if (id >= (unsigned int)setting->ranks.size())
				{
					client.createMessage(message.channelId(), "Id not found.");
					return;
				}

				client.createMessage(message.channelId(), "Deleted rank " + setting->ranks[id].name + " succesfully.");
				setting->ranks.erase(setting->ranks.begin() + id);
			});
		else if (args[1] == "edit" && args.size() >= 4)
			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[args, &client, message, channel, printHelp](bool result)
			{
				if (!result)
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());

				bool ok;
				unsigned int id = args[3].toUInt(&ok);
				if (!ok)
				{
					client.createMessage(message.channelId(), "Invalid id.");
					return;
				}

				if (id >= (unsigned int)setting->ranks.size())
				{
					client.createMessage(message.channelId(), "Id not found.");
					return;
				}

				LevelRank& rank = setting->ranks[id];
				if (args[2] == "name")
				{
					QString name = "";
					for (int i = 4; i < args.size(); i++)
					{
						name += args[i];
						if (i < args.size() - 1)
							name += " ";
					}
					rank.name = name;
					client.createMessage(message.channelId(), "Rank id " + QString::number(id) + " has been succesfully edited.");
				}
				else if (args[2] == "level")
				{
					bool ok;
					unsigned int newlevel = args[4].toUInt(&ok);
					if (!ok)
					{
						client.createMessage(message.channelId(), "Invalid new level.");
						return;
					}

					rank.minimumLevel = newlevel;
					client.createMessage(message.channelId(), "Rank id " + QString::number(id) + " has been succesfully edited.");
				}
				else
				{
					printHelp();
				}
			});
		else
			printHelp();
	});

	RegisterCommand(Commands::LEVEL_MODULE_MAX_LEVEL, "setmaxlevel",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QStringList args = message.content().split(' ');
		QString prefix = setting->prefix;

		if (args.first() != prefix + "setmaxlevel")
			return;

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help setmaxlevel");
			QString description = bot->GetCommandHelp("setmaxlevel", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		};

		if (args.size() == 2) 
		{
			if (args.last() == "current")
			{
				client.createMessage(message.channelId(), "Maximum level is currently set to " + QString::number(setting->maximumLevel));
				return;
			}

			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[args, &client, message, channel, printHelp, setting](bool result)
			{
				if (!result)
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}
				bool ok;
				unsigned int level = args[1].toUInt(&ok);
				if (!ok)
				{
					client.createMessage(message.channelId(), "Invalid level.");
					return;
				}
				setting->maximumLevel = level;
				client.createMessage(message.channelId(), "Maximum level set to " + QString::number(level) + " succesfully!");
			});
		}
		else
			printHelp();
	});

	RegisterCommand(Commands::LEVEL_MODULE_EXP_REQUIREMENT, "setexpreq",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QStringList args = message.content().split(' ');
		QString prefix = setting->prefix;

		if (args.first() != prefix + "setexpreq")
			return;

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help setexpreq");
			QString description = bot->GetCommandHelp("setexpreq", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		};

		if (args.size() == 2)
		{
			if (args.last() == "current")
			{
				client.createMessage(message.channelId(), "Exp requirement is currently set to " + QString::number(setting->expRequirement));
				return;
			}

			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[args, &client, message, channel, printHelp, setting](bool result)
			{
				if (!result)
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}
				bool ok;
				unsigned int expReq = args[1].toUInt(&ok);
				if (!ok)
				{
					client.createMessage(message.channelId(), "Invalid exp requirement.");
					return;
				}
				setting->expRequirement = expReq;
				client.createMessage(message.channelId(), "Exp requirement set to " + QString::number(expReq) + " succesfully!");
			});
		}
		else
			printHelp();
	});

	RegisterCommand(Commands::LEVEL_MODULE_EXP_GROWTH_RATE, "setgrowthrate",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QStringList args = message.content().split(' ');
		QString prefix = setting->prefix;

		if (args.first() != prefix + "setgrowthrate")
			return;

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help setgrowthrate");
			QString description = bot->GetCommandHelp("setgrowthrate", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		};

		if (args.size() == 2)
		{
			if (args.last() == "current")
			{
				client.createMessage(message.channelId(), "Exp growth is currently set to " + QString::number(setting->growthRate));
				return;
			}

			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[args, &client, message, channel, printHelp, setting](bool result)
			{
				if (!result)
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}
				bool ok;
				float growthRate = args[1].toFloat(&ok);
				if (!ok || growthRate < 1)
				{
					client.createMessage(message.channelId(), "Invalid growth rate.");
					return;
				}
				setting->growthRate = growthRate;
				client.createMessage(message.channelId(), "Growth rate set to " + QString::number(growthRate) + " succesfully!");
			});
		}
		else
			printHelp();
	});

	RegisterCommand(Commands::LEVEL_MODULE_EXP_GIVE, "givexp",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		GuildSetting s = GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = s.prefix;
		QStringList args = message.content().split(' ');

		if (args.first() != prefix + "givexp")
			return;

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help givexp");
			QString description = bot->GetCommandHelp("givexp", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		};

		if (args.size() >= 3)
		{
			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[this, args, &client, message, channel, printHelp, s](bool result)
			{
				auto& exp = m_exp[channel.guildId()];

				if (!result)
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}

				snowflake_t userId = static_cast<UmikoBot*>(&client)->GetUserFromArg(channel.guildId(), args, 2);

				if (userId == 0) {
					client.createMessage(message.channelId(), "Could not find user!");
					return;
				}

				LevelModule::GuildLevelData* levelData;

				for (int i = 0; i < exp.size(); i++) 
				{
					if (exp[i].user == userId)
						levelData = &exp[i];
				}

				ExpLevelData userRes = ExpToLevel(channel.guildId(), levelData->exp);

				int finalExp = levelData->exp;
				int addedExp = 0;
				
				if (userRes.level == s.maximumLevel) {
					client.createMessage(message.channelId(), "User is already maximum level!");
					return;
				}

				if (args[1].endsWith("L"))
				{
					QStringRef substring(&args[1], 0, args[1].size() - 1);
					unsigned int levels = substring.toUInt();

					if (userRes.level + levels >= s.maximumLevel) {
						finalExp = s.expRequirement * (qPow(s.growthRate, s.maximumLevel) - 1) / (s.growthRate - 1);
					}
					else
					{
						for (unsigned int i = 0; i < levels; i++) {
							addedExp += userRes.xpRequirement;
							userRes.xpRequirement *= s.growthRate;
						}
						finalExp += addedExp;
					}
					
				}
				else 
				{
					addedExp += args[1].toUInt();
					ExpLevelData newRes = ExpToLevel(channel.guildId(), addedExp + finalExp);
					if (newRes.level >= s.maximumLevel) {
						finalExp = s.expRequirement * (pow(s.growthRate, s.maximumLevel) - 1) / (s.growthRate - 1);
					}
				}
				levelData->exp = finalExp;

				client.createMessage(message.channelId(), "Succesfully given " + static_cast<UmikoBot*>(&client)->GetName(channel.guildId(), userId) + " " + QString::number(addedExp) + " exp");

			});
		}
		else
			printHelp();
	});

	RegisterCommand(Commands::LEVEL_MODULE_EXP_TAKE, "takexp",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		GuildSetting s = GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = s.prefix;
		QStringList args = message.content().split(' ');

		if (args.first() != prefix + "takexp")
			return;

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help takexp");
			QString description = bot->GetCommandHelp("takexp", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		};

		if (args.size() >= 3)
		{
			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[this, args, &client, message, channel, printHelp, s](bool result)
			{
				auto& exp = m_exp[channel.guildId()];

				if (!result)
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}

				snowflake_t userId = static_cast<UmikoBot*>(&client)->GetUserFromArg(channel.guildId(), args, 2);

				if (userId == 0) {
					client.createMessage(message.channelId(), "Could not find user!");
					return;
				}

				LevelModule::GuildLevelData* levelData;

				for (int i = 0; i < exp.size(); i++)
				{
					if (exp[i].user == userId)
						levelData = &exp[i];
				}

				ExpLevelData userRes = ExpToLevel(channel.guildId(), levelData->exp);

				int finalExp = levelData->exp;
				int subtractedExp = 0;

				if (userRes.level == 0) {
					client.createMessage(message.channelId(), "User is already minimum level!");
					return;
				}

				if (args[1].endsWith("L"))
				{
					QStringRef substring(&args[1], 0, args[1].size() - 1);
					unsigned int levels = substring.toUInt();

					if (userRes.level - levels < 0) 
					{
						subtractedExp = levelData->exp;
						finalExp = 0;
					}
					else {
						userRes.xpRequirement /= s.growthRate;
						for (unsigned int i = 0; i < levels; i++)
						{
							subtractedExp += userRes.xpRequirement;
							userRes.xpRequirement /= s.growthRate;
						}
					}

				}
				else
				{
					subtractedExp += args[1].toUInt();
				}
				if (finalExp - subtractedExp < 0)
					finalExp = 0;
				else
					finalExp -= subtractedExp;

				levelData->exp = finalExp;

				client.createMessage(message.channelId(), "Succesfully taken " + QString::number(subtractedExp) + " exp from " + static_cast<UmikoBot*>(&client)->GetName(channel.guildId(), userId));

			});
		}
		else
			printHelp();
	});

	RegisterCommand(Commands::LEVEL_MODULE_BLOCK_EXP, "blockxp",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		GuildSetting s = GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = s.prefix;
		QStringList args = message.content().split(' ');

		if (args.first() != prefix + "blockxp")
			return;

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help blockxp");
			QString description = bot->GetCommandHelp("blockxp", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		};

		if (args.size() > 1 && args[1] == "whitelist")
		{
			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[this, &client, printHelp, message, args, channel](bool result)
			{
				if (!result)
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}

				GuildSetting& s = GuildSettings::GetGuildSetting(channel.guildId());
				for (int i = 0; i < s.levelBlacklistedChannels.size(); i++) {
					if (s.levelBlacklistedChannels[i] == channel.id())
					{
						s.levelBlacklistedChannels.erase(s.levelBlacklistedChannels.begin() + i);
						client.createMessage(message.channelId(), "Channel removed from blacklisted!");
						return;
					}
				}

				for (int i = 0; i < s.levelWhitelistedChannels.size(); i++) {
					if (s.levelWhitelistedChannels[i] == channel.id())
					{
						client.createMessage(message.channelId(), "Channel is already whitelisted!");
						return;
					}
				}

				s.levelWhitelistedChannels.push_back(channel.id());
				client.createMessage(message.channelId(), "Channel has been whitelisted!");
			});
		}
		else if (args.size() > 1 && args[1] == "blacklist")
		{
			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[this, &client, printHelp, message, args, channel](bool result)
			{
				if (!result)
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}

				GuildSetting& s = GuildSettings::GetGuildSetting(channel.guildId());
				for (int i = 0; i < s.levelWhitelistedChannels.size(); i++) {
					if (s.levelWhitelistedChannels[i] == channel.id())
					{
						s.levelWhitelistedChannels.erase(s.levelWhitelistedChannels.begin() + i);
						client.createMessage(message.channelId(), "Channel removed from whitelisted!");
						return;
					}
				}

				for (int i = 0; i < s.levelBlacklistedChannels.size(); i++) {
					if (s.levelBlacklistedChannels[i] == channel.id())
					{
						client.createMessage(message.channelId(), "Channel is already blacklisted!");
						return;
					}
				}

				s.levelBlacklistedChannels.push_back(channel.id());
				client.createMessage(message.channelId(), "Channel has been blacklisted!");
			});
		}
		else
			printHelp();
	});
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

void LevelModule::OnMessage(Discord::Client& client, const Discord::Message& message) 
{
	Module::OnMessage(client, message);

	client.getChannel(message.channelId()).then(
		[this, message](const Discord::Channel& channel) 
	{
		auto& exp = m_exp[channel.guildId()];

		if (!GuildSettings::IsModuleEnabled(channel.guildId(), GetName(), IsEnabledByDefault()))
			return;

		if (message.author().bot())
			return;

		if(!GuildSettings::ExpAllowed(channel.guildId(), channel.id()))
			return;

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
