#include "modules/user/LevelModule.h"
#include "core/Permissions.h"
#include "UmikoBot.h"

#include <QtMath>

using namespace Discord;

void LevelModule::initiateAdminCommands()
{
	RegisterCommand(Commands::LEVEL_MODULE_MAX_LEVEL, "setmaxlevel",
		[this](Client& client, const Message& message, const Channel& channel)
	{
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QStringList args = message.content().split(' ');
		QString prefix = setting->prefix;

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Embed embed;
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

			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args, setting]()
			{
				setting->maximumLevel = args[1].toUInt();
				client.createMessage(message.channelId(), "Maximum level set to " + QString::number(setting->maximumLevel) + " succesfully!");
			});
		}
		else
			printHelp();
	});

	RegisterCommand(Commands::LEVEL_MODULE_EXP_REQUIREMENT, "setexpreq",
		[this](Client& client, const Message& message, const Channel& channel)
	{
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QStringList args = message.content().split(' ');
		QString prefix = setting->prefix;

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Embed embed;
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

			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args, setting]()
			{
				setting->expRequirement = args[1].toUInt();
				client.createMessage(message.channelId(), "Exp requirement set to " + QString::number(setting->expRequirement) + " succesfully!");
			});
		}
		else
			printHelp();
	});

	RegisterCommand(Commands::LEVEL_MODULE_EXP_GROWTH_RATE, "setgrowthrate",
		[this](Client& client, const Message& message, const Channel& channel)
	{
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QStringList args = message.content().split(' ');
		QString prefix = setting->prefix;

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Embed embed;
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

			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args, setting]()
			{
				setting->growthRate = args[1].toFloat();
				client.createMessage(message.channelId(), "Growth rate set to " + QString::number(setting->growthRate) + " succesfully!");
			});
		}
		else
			printHelp();
	});


	RegisterCommand(Commands::LEVEL_MODULE_EXP_TAKE, "takexp",
		[this](Client& client, const Message& message, const Channel& channel)
	{
		GuildSetting s = GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = s.prefix;
		QStringList args = message.content().split(' ');

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help takexp");
			QString description = bot->GetCommandHelp("takexp", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		};

		if (args.size() >= 3)
		{
			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 3, args, false, [this, &client, channel, message, args, s]()
			{
				auto& exp = m_exp[channel.guildId()];

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
		[this](Client& client, const Message& message, const Channel& channel)
	{
		GuildSetting s = GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = s.prefix;
		QStringList args = message.content().split(' ');

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help blockxp");
			QString description = bot->GetCommandHelp("blockxp", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		};

		if (args.size() > 1 && args[1] == "whitelist")
		{
			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, &client, channel, message, args]()
			{
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
			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, &client, channel, message, args]()
			{
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

	RegisterCommand(Commands::LEVEL_MODULE_EXP_GIVE, "givexp",
		[this](Client& client, const Message& message, const Channel& channel)
	{
		GuildSetting s = GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = s.prefix;
		QStringList args = message.content().split(' ');

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help givexp");
			QString description = bot->GetCommandHelp("givexp", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		};

		if (args.size() >= 3)
		{
			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 3, args, false, [this, &client, channel, message, args, s]()
			{
				auto& exp = m_exp[channel.guildId()];

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

	RegisterCommand(Commands::LEVEL_MODULE_RANK, "rank",
		[this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		auto printHelp = [&client, prefix, message]()
		{
			UmikoBot* bot = reinterpret_cast<UmikoBot*>(&client);
			Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help rank");
			QString description = bot->GetCommandHelp("rank", prefix);
			embed.setDescription(description);
			bot->createMessage(message.channelId(), embed);
		};
		
		if (args.size() < 2)
		{
			printHelp();
			return;
		}

		if (args.last() == "list" && args.size() == 2)
		{
			QList<LevelRank> ranks = setting->ranks;
			Embed embed;
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
			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 4, args, false, [this, &client, channel, message, args]()
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				
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
			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 3, args, false, [this, &client, channel, message, args]()
			{
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
			UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 5, args, false, [this, &client, channel, message, args, printHelp]()
			{
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
}
