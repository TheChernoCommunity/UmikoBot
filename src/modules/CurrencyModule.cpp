#include "CurrencyModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/qregexp.h>

//! Currency Config Location
#define currenConfigLoc QString("currencyConfig")

//! Maximum amount that can be bet
#define gamblebetMax 100

//! Maximum debt that a user can be in
#define debtMax -100

//! Gamble Timeout in seconds
#define gambleTimeout 20

CurrencyModule::CurrencyModule(UmikoBot* client) : Module("currency", true), m_client(client)
{

	m_timer.setInterval(24*60*60*1000); //!24hr timer
	QObject::connect(&m_timer, &QTimer::timeout, [this, client]() 
		{
		for (auto server : guildList.keys()) 
		{
			//!Clear the daily bonus for everyone
			for (int i = 0; i < guildList[server].size(); i++) 
			{

				auto user = guildList[server][i];
				if (UmikoBot::Instance().GetName(server, user.userId) != "") 
				{
					user.isDailyClaimed = false;
				}
				else 
				{
					//remove if the user is no longer in the server
					guildList[server].removeAt(i);
				}
			}
				auto guildId = server;
				auto& serverConfig = getServerData(guildId);

				if (!serverConfig.isRandomGiveawayDone) 
				{
					client->createMessage(serverConfig.giveawayChannelId, "Hey everyone! Today's freebie expires in **" + QString::number(serverConfig.freebieExpireTime) + " seconds**. `!claim` it now!");

					serverConfig.allowGiveaway = true;
					
					//! Delete the previously allocated thingy
					if (serverConfig.freebieTimer != nullptr) {
						delete serverConfig.freebieTimer;
						serverConfig.freebieTimer = nullptr;
					}

					serverConfig.freebieTimer = new QTimer;
					serverConfig.freebieTimer->setInterval(serverConfig.freebieExpireTime * 1000);
					serverConfig.freebieTimer->setSingleShot(true);
					QObject::connect(serverConfig.freebieTimer, &QTimer::timeout,
					[this, client, guildId] ()
						{
							auto& serverConfig = getServerData(guildId);
							serverConfig.allowGiveaway = false;
						});
					serverConfig.freebieTimer->start();
				}
				serverConfig.isRandomGiveawayDone = false;
			}
	});

	m_timer.start();

	RegisterCommand(Commands::CURRENCY_WALLET, "wallet", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "wallet")
			return;
		if (args.size() > 1) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}
		if (args.size() == 1) 
		{
			Discord::Embed embed;
			embed.setColor(11777216);

			//! Get User and Sever Data
			auto config = getServerData(channel.guildId());

			QString creditScore = QString::number(getUserData(channel.guildId(), message.author().id()).currency);
			
			QString desc = "**Current Credits: ** `" + creditScore + "` **" + config.currencySymbol + "** (" + config.currencyName +")";
			embed.setTitle(UmikoBot::Instance().GetName(channel.guildId(), message.author().id()) + "'s Wallet");
			embed.setDescription(desc);
			client.createMessage(message.channelId(), embed);
		}
		});

	RegisterCommand(Commands::CURRENCY_DAILY, "daily", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "daily")
			return;
		if (args.size() > 1) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}
		if (getUserData(channel.guildId(), message.author().id()).isDailyClaimed)
		{

			int remainingTime = m_timer.remainingTime();

			QTime conv(0, 0, 0);
			conv = conv.addMSecs(remainingTime);
			QString time = QString::number(conv.hour()) + "(hrs) " + QString::number(conv.minute()) + "(mins) " + QString::number(conv.second()) + "(secs)";
			QString desc = "**You have already claimed your daily credits.**\nCome back after `" + time + "` to get more rich!";

			client.createMessage(message.channelId(), desc);
		}
		else 
		{
			auto index = getUserIndex(channel.guildId(), message.author().id());
			
			guildList[channel.guildId()][index].isDailyClaimed = true;
			guildList[channel.guildId()][index].currency += getServerData(channel.guildId()).dailyReward;

			client.createMessage(message.channelId(), "**You now have "+ QString::number(getServerData(channel.guildId()).dailyReward) + " more " + getServerData(channel.guildId()).currencyName + "(s) in your wallet!**");
		}

		});


	RegisterCommand(Commands::CURRENCY_GAMBLE, "gamble", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{
		
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;
		
		if (args.first() != prefix + "gamble")
			return;

		if (args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		//! Normal Mode
		if (args.size() == 1)
		{

			auto& serverGamble = gambleData[channel.guildId()];
			if (guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())].currency - getServerData(channel.guildId()).gambleLoss < debtMax) 
			{
				client.createMessage(message.channelId(), "**Nope, can't let you get to serious debt.**");
				return;
			}
			if (serverGamble.gamble) 
			{
				QString user = UmikoBot::Instance().GetName(channel.guildId(), serverGamble.userId);
				Discord::Embed embed;
				embed.setColor(qrand() % 16777216);
				embed.setTitle("Welcome to Gamble!");
				embed.setDescription("Sorry but this feature is currently in use by **" + user + "**. Please try again later!");
				client.createMessage(message.channelId(), embed);
				return;
			}

			auto& config = getServerData(channel.guildId());

			serverGamble.randNum = qrand() % (config.maxGuess - config.minGuess +1) + config.minGuess;
			serverGamble.channelId = message.channelId();
			serverGamble.gamble = true;
			serverGamble.userId = message.author().id();

			auto guild = channel.guildId();

			serverGamble.timer = new QTimer();

			serverGamble.timer->setInterval(gambleTimeout * 1000);
			serverGamble.timer->setSingleShot(true);
			QObject::connect(serverGamble.timer, &QTimer::timeout, [this, &client, guild, message]() 
				{
				if (gambleData[guild].gamble || gambleData[guild].doubleOrNothing) 
				{
					gambleData[guild].gamble = false;
					gambleData[guild].doubleOrNothing = false;

					client.createMessage(message.channelId(), "**Gamble Timeout Due to No Valid Response**");

				}				
				});

			serverGamble.timer->start();

			QString name = UmikoBot::Instance().GetName(channel.guildId(), serverGamble.userId);

			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Welcome to Gamble " + name + "!");
			embed.setDescription("All you need to do is guess a random number between " + QString::number(config.minGuess) + " and " + QString::number(config.maxGuess) + " (inclusive) and if it is the same as the number I think of, you get **" + QString::number(config.gambleReward) + config.currencySymbol + "**!\n\n**What number do you think of?** <:wesmart:388340133864407043>");
			
			client.createMessage(message.channelId(), embed);
		}

		//! Double or Nothing
		if (args.size() == 2)
		{
			QRegExp re("[+]?\\d*\\.?\\d+");
			if (!re.exactMatch(args.at(1))) 
			{
				client.createMessage(message.channelId(), "**Wrong Usage of Command!** The argument must be a **positive number**.");
					return;
			}

			auto& serverGamble = gambleData[channel.guildId()];
			auto& config = getServerData(channel.guildId());

			if (guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())].currency - args.at(1).toDouble() < debtMax)
			{
				client.createMessage(message.channelId(), "**Nope, can't let you get to serious debt.**");
				return;
			}
			if (serverGamble.gamble)
			{
				QString user = UmikoBot::Instance().GetName(channel.guildId(), serverGamble.userId);
				Discord::Embed embed;
				embed.setColor(qrand() % 16777216);
				embed.setTitle("Welcome to Gamble!");
				embed.setDescription("Sorry but this feature is currently in use by **" + user + "**. Please try again later!");
				client.createMessage(message.channelId(), embed);
				return;
			}
			if (args.at(1).toDouble() > gamblebetMax) 
			{
				client.createMessage(channel.id(), "You cannot bet an amount more than **" + QString::number(gamblebetMax) + config.currencySymbol+"**");
				return;
			}
			if (args.at(1).toDouble() == 0) 
			{
				client.createMessage(message.channelId(), "<:aanger:730377398314467439> **BRUH. Don't you dare waste my time! I ain't interested in nothing.**");
				return;
			}
			serverGamble.randNum = qrand() % (config.maxGuess - config.minGuess + 1) + config.minGuess;
			serverGamble.channelId = message.channelId();
			serverGamble.gamble = true;
			serverGamble.userId = message.author().id();
			serverGamble.doubleOrNothing = true;
			serverGamble.betAmount = args.at(1).toDouble();

			auto guild = channel.guildId();

			serverGamble.timer = new QTimer();

			serverGamble.timer->setInterval(gambleTimeout * 1000);
			serverGamble.timer->setSingleShot(true);
			QObject::connect(serverGamble.timer, &QTimer::timeout, [this, &client, guild, message]() {

				if (gambleData[guild].gamble || gambleData[guild].doubleOrNothing) 
				{
					gambleData[guild].gamble = false;
					gambleData[guild].doubleOrNothing = false;

					client.createMessage(message.channelId(), "**Gamble Timeout Due to No Valid Response**");

				}
				
				});

			serverGamble.timer->start();
			QString name = UmikoBot::Instance().GetName(channel.guildId(), serverGamble.userId);

			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Welcome to Gamble (Double or Nothing) " + name + "!");
			embed.setDescription("All you need to do is guess a random number between " + QString::number(config.minGuess) + " and " + QString::number(config.maxGuess) + " (inclusive) and if it is the same as the number I guess, you get double the amount you just bet: **" + QString::number(2* serverGamble.betAmount) + config.currencySymbol + "**!\n\n**What number do you think of?** <:wesmart:388340133864407043>");

			client.createMessage(message.channelId(), embed);
		}

		});

	RegisterCommand(Commands::CURRENCY_CLAIM, "claim", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "claim")
			return;

		if (args.size() > 1) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 1) 
		{
			if (!getServerData(channel.guildId()).isRandomGiveawayDone) 
			{
				if (getServerData(channel.guildId()).allowGiveaway)
				{
					auto& config = getServerData(channel.guildId());
					Discord::Embed embed;
					embed.setColor(qrand() % 11777216);
					embed.setTitle("Claim FREEBIE");
					embed.setDescription(":drum: And today's FREEBIE goes to **" + message.author().username() + "**! \n\n Congratulations! You just got **"+ QString::number(config.freebieReward) + config.currencySymbol +"**!");

					auto index = getUserIndex(channel.guildId(), message.author().id());

					guildList[channel.guildId()][index].currency += config.freebieReward;

					client.createMessage(message.channelId(), embed);
					getServerData(channel.guildId()).isRandomGiveawayDone = true;
					getServerData(channel.guildId()).allowGiveaway = false;
				}
				else 
				{
					client.createMessage(message.channelId(), "**BRUH**, ***yOu CaN't JuSt GeT fReE sTuFf aNyTiMe.***");
				}
			}
			else 
			{
				Discord::Embed embed;
				embed.setColor(qrand()%11777216);
				embed.setTitle("Claim FREEBIE");
				embed.setDescription("Sorry, today's freebie has been claimed :cry: \n\n But you can always try again the next day!");
				client.createMessage(message.channelId(), embed);
			}
		}

		});

	RegisterCommand(Commands::CURRENCY_SET_PRIZE_CHANNEL, "setannouncechan", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setannouncechan")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}
			
				if (args.size() > 1) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() == 1) 
				{
					auto& config = getServerData(channel.guildId());
					config.giveawayChannelId = message.channelId();
					client.createMessage(message.channelId(), "**Giveaway announcement channel successfully changed to current channel.**");
				}
			});
	});

	RegisterCommand(Commands::CURRENCY_SET_NAME, "setcurrenname", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{
		
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;
		
		if (args.first() != prefix + "setcurrenname")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}
				if (args.size() == 1) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() > 2) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() == 2) 
				{
					auto& config = getServerData(channel.guildId());
					config.currencyName = args.at(1);
					client.createMessage(message.channelId(), "**Currency Name set to** " + config.currencyName);
				}
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_SYMBOL, "setcurrensymb", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "setcurrensymb")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}

				if (args.size() == 1) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() > 2) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() == 2) 
				{
					auto& config = getServerData(channel.guildId());
					config.currencySymbol = args.at(1);
					client.createMessage(message.channelId(), "**Currency Symbol set to** " + config.currencySymbol);
				}
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_DAILY, "setdaily", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setdaily")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}

				if (args.size() == 1 || args.size() > 2) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() == 2) 
				{
					auto& config = getServerData(channel.guildId());
					config.dailyReward = args.at(1).toInt();
					client.createMessage(message.channelId(), "**Daily Reward Amount set to **" + QString::number(config.dailyReward));
				}
			});
	});

	RegisterCommand(Commands::CURRENCY_SET_PRIZE , "setprize", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setprize")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}

				if (args.size() == 1 || args.size() > 2) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() == 2) 
				{
					auto& config = getServerData(channel.guildId());
					config.freebieReward = args.at(1).toInt();
					client.createMessage(message.channelId(), "**Freebie Reward Amount set to **" + QString::number(config.freebieReward));
				}
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_PRIZE_PROB, "setprizeprob", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setprizeprob")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}

				if (args.size() == 1 || args.size() > 2) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() == 2) 
				{
					auto& config = getServerData(channel.guildId());
					config.randGiveawayProb = args.at(1).toDouble();
					client.createMessage(message.channelId(), "**Giveaway Probability Amount set to **" + QString::number(config.randGiveawayProb));
				}
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_PRIZE_EXPIRY, "setprizeexpiry", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setprizeexpiry")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}

				if (args.size() == 1 || args.size() > 2) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() == 2) 
				{
					auto& config = getServerData(channel.guildId());
					config.freebieExpireTime = args.at(1).toUInt();
					client.createMessage(message.channelId(), "**Freebie Expiry Time (secs) set to **" + QString::number(config.freebieExpireTime));
				}

		});
	});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_LOSS, "setgambleloss", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setgambleloss")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}

				if (args.size() == 1 || args.size() > 2) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() == 2) 
				{
					auto& config = getServerData(channel.guildId());
					config.gambleLoss = args.at(1).toInt();
					client.createMessage(message.channelId(), "**Gamble Loss Amount set to **" + QString::number(config.gambleLoss));
				}
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_MAX_GUESS, "setgamblemaxguess", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "setgamblemaxguess")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}

				if (args.size() == 1 || args.size() > 2) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() == 2) 
				{
					auto& config = getServerData(channel.guildId());
					config.maxGuess = args.at(1).toInt();
					client.createMessage(message.channelId(), "**Gamble Max Guess set to **" + QString::number(config.maxGuess));
				}
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_MIN_GUESS, "setgambleminguess", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{
		
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setgambleminguess")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}

				if (args.size() == 1 || args.size() > 2) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() == 2) 
				{
					auto& config = getServerData(channel.guildId());
					config.minGuess = args.at(1).toInt();
					client.createMessage(message.channelId(), "**Gamble Min Guess set to **" + QString::number(config.minGuess));
				}
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_REWARD, "setgamblereward", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setgamblereward")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}

				if (args.size() == 1 || args.size() > 2) 
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (args.size() == 2) 
				{
					auto& config = getServerData(channel.guildId());
					config.gambleReward = args.at(1).toInt();
					client.createMessage(message.channelId(), "**Gamble Reward Amount set to **" + QString::number(config.gambleReward));
				}

		});
	});

	RegisterCommand(Commands::CURRENCY_RICH_LIST, "richlist", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{
		
			QStringList args = message.content().split(' ');
			GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
			QString prefix = setting->prefix;


			if (args.first() != prefix + "richlist")
				return;

			if (args.size() > 1) 
			{
				client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
				return;
			}
			else 
			{
				//! Print the top 30 (or less depending on number of 
				//! members) people in the leaderboard

				auto& leaderboard = guildList[channel.guildId()];

				//! Remove the users which aren't in the server anymore
				for (int i = 0; i < leaderboard.size(); i++) 
				{
					if (UmikoBot::Instance().GetName(channel.guildId(), leaderboard[i].userId) == "")
					{
						leaderboard.removeAt(i);
					}
				}

				int offset{ 30 };
				if (leaderboard.size() < 30) 
				{
					offset = leaderboard.size();
				}

				qSort(leaderboard.begin(), leaderboard.end(), [](UserCurrency u1, UserCurrency u2)
					{
						return u1.currency > u2.currency;
					});

				Discord::Embed embed;
				embed.setTitle("Currency Leaderboard (Top 30)");
				QString desc;
				int rank = 0;
				for (auto& user : leaderboard.mid(0, offset)) {
					rank++;
					QString username = UmikoBot::Instance().GetName(channel.guildId(), user.userId);
					QString currency = QString::number(user.currency);

					desc += "**" + QString::number(rank) + ") " + username + ":** ";
					desc += "`" + currency + "`" + "**" + getServerData(channel.guildId()).currencySymbol + "**\n";
				}

				embed.setColor(qrand() % 16777216);
				embed.setDescription(desc);

				client.createMessage(message.channelId(), embed);
			}
		
		}
	);

	RegisterCommand(Commands::CURRENCY_DONATE, "donate", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) {

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "donate")
			return;

		if (args.size() == 1 || args.size() == 2) {
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}
		QRegExp re("[+]?\\d*\\.?\\d+");
		if (!re.exactMatch(args.at(1))) 
		{
				client.createMessage(message.channelId(), "**You can't donate in invalid amounts**");
					return;
		}

		if (args.at(1).toDouble() == 0) 
		{
			client.createMessage(message.channelId(), "**Please don't donate at all if you don't want to donate anything.**");
			return;
		}

		if (guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())].currency - args.at(1).toDouble() < 0)
		{
			client.createMessage(message.channelId(), "**I can't let you do that, otherwise you'll be in debt!**");
			return;
		}

		QList<Discord::User> mentions = message.mentions();
		if (mentions.size() == 0) {
			client.createMessage(message.channelId(), "**Who do you want to donate to? Please `@` all those people.**");
			return;
		}

		for (auto user : mentions) 
		{
		if (user.id() == message.author().id()) 
		{
			client.createMessage(message.channelId(), "**You cannot donate to yourself!**\nPlease remove yourself from the list.");
			return;
		}
		}

		guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())].currency -= args.at(1).toDouble();

		double donation = args.at(1).toDouble() / mentions.size();
		QString desc = "<@" + QString::number(message.author().id()) + "> donated **" + QString::number(donation) + getServerData(channel.guildId()).currencySymbol + "** to";

		for (auto user : mentions) 
		{
			auto index = getUserIndex(channel.guildId(), user.id());
			desc += " <@" + QString::number(user.id()) + ">";
			guildList[channel.guildId()][index].currency += donation;
		}

		Discord::Embed embed;
		embed.setTitle("Donation by " + UmikoBot::Instance().GetName(channel.guildId(), message.author().id()) + "!");
		embed.setDescription(desc);
		embed.setColor(qrand() % 16777216);

		client.createMessage(message.channelId(), embed);

		}
	);
}

void CurrencyModule::StatusCommand(QString& result, snowflake_t guild, snowflake_t user) 
{
	QString creditScore = QString::number(getUserData(guild, user).currency);
	result += "**Wallet: **" + creditScore + " **" + getServerData(guild).currencySymbol +"**";
	result+='\n';
}

void CurrencyModule::OnMessage(Discord::Client& client, const Discord::Message& message) 
{

	client.getChannel(message.channelId()).then(
		[this, message, &client](const Discord::Channel& channel) 
		{
			if (channel.guildId() != 0 && !message.author().bot()) 
				//! Make sure the message is not a DM
			{
				auto guildId = channel.guildId();
				auto& serverConfig = getServerData(guildId);

				if (!serverConfig.isRandomGiveawayDone && !serverConfig.allowGiveaway) 
				{
						randp.param(std::bernoulli_distribution::param_type(serverConfig.randGiveawayProb));
						bool outcome = randp(random_engine);
						if (outcome) {
							client.createMessage(serverConfig.giveawayChannelId, "Hey everyone! **FREEBIE** available now! Go `!claim` some juicy coins!");
							serverConfig.allowGiveaway = true;
						}
				
				}


				//! If the message is a number, continue with the gambling mech
				QRegExp re("\\d*");
				if (re.exactMatch(message.content())) 
				{
		
					if (gambleData[guildId].gamble && !message.author().bot() && message.channelId() == gambleData[guildId].channelId && message.author().id() == gambleData[guildId].userId)
					{

						if (message.content().toInt() > serverConfig.maxGuess || message.content().toInt() < serverConfig.minGuess)
						{

							client.createMessage(message.channelId(), "**Your guess is out of range!** \nTry a number between " + QString::number(serverConfig.minGuess) + " and " + QString::number(serverConfig.maxGuess) + " (inclusive). ");
							return;

						}
						if (message.content().toInt() == gambleData[guildId].randNum) 
						{
							if (!gambleData[guildId].doubleOrNothing) 
							{
								auto index = getUserIndex(guildId, message.author().id());
								guildList[guildId][index].currency += serverConfig.gambleReward;
								client.createMessage(message.channelId(), "**You guessed CORRECTLY!**\n(**" + QString::number(serverConfig.gambleReward) + serverConfig.currencySymbol + "** have been added to your wallet!)");
							}
							else 
							{
								auto index = getUserIndex(guildId, message.author().id());
								guildList[guildId][index].currency += 2 * gambleData[guildId].betAmount;
								client.createMessage(message.channelId(), "**You guessed CORRECTLY!**\n(**" + QString::number(2 * gambleData[guildId].betAmount) + serverConfig.currencySymbol + "** have been added to your wallet!)");
							}

						}

						//! Player lost
						else 
						{
							if (!gambleData[guildId].doubleOrNothing) 
							{
								auto index = getUserIndex(guildId, message.author().id());
								guildList[guildId][index].currency -= serverConfig.gambleLoss;
								client.createMessage(message.channelId(), "**Better Luck next time!**\n*(psst! I took **" + QString::number(serverConfig.gambleLoss) + serverConfig.currencySymbol
									+ "** from your wallet for my time...)*");
							}
							else 
							{
								auto index = getUserIndex(guildId, message.author().id());
								guildList[guildId][index].currency -= gambleData[guildId].betAmount;
								client.createMessage(message.channelId(), "**Better Luck next time!**\n*(psst! I took **" + QString::number(gambleData[guildId].betAmount) + serverConfig.currencySymbol
									+ "** from your wallet...)*");
							}

						}

						gambleData[guildId].gamble = false;
						gambleData[guildId].doubleOrNothing = false;
						delete gambleData[guildId].timer;
					}
				}
			}

		});

	Module::OnMessage(client, message);
}

void CurrencyModule::OnSave(QJsonDocument& doc) const 
{
	QJsonObject docObj;

	//! User Data
	for (auto server : guildList.keys()) 
	{
		QJsonObject serverJSON;
		
		for (auto user = guildList[server].begin(); user != guildList[server].end(); user++) 
		{	
			QJsonObject obj;
			obj["currency"] = user->currency;
			obj["isDailyClaimed"] = user->isDailyClaimed;

			serverJSON[QString::number(user->userId)] = obj;
		}

		docObj[QString::number(server)] = serverJSON;
	}

	doc.setObject(docObj);
	

	//! Server Data (Config)
	QFile currenConfigfile("configs/" + currenConfigLoc + ".json");
	if (currenConfigfile.open(QFile::ReadWrite | QFile::Truncate)) 
	{
		QJsonDocument doc;
		QJsonObject serverList;
		for (auto server : serverCurrencyConfig.keys())
		{
			QJsonObject obj;
			
			auto config = serverCurrencyConfig[server];
			obj["name"] = config.currencyName;
			obj["symbol"] = config.currencySymbol;
			obj["freebieChannelId"] = QString::number(config.giveawayChannelId);
			obj["dailyReward"] = QString::number(config.dailyReward);
			obj["freebieReward"] = QString::number(config.freebieReward);
			obj["gambleLoss"] = QString::number(config.gambleLoss);
			obj["gambleReward"] = QString::number(config.gambleReward);
			obj["gambleMinGuess"] = QString::number(config.minGuess);
			obj["gambleMaxGuess"] = QString::number(config.maxGuess);
			obj["freebieProb"] = QString::number(config.randGiveawayProb);
			obj["freebieExpireTime"] = QString::number(config.freebieExpireTime);

			serverList[QString::number(server)] = obj;
			
		}
		doc.setObject(serverList);
		currenConfigfile.write(doc.toJson());
		currenConfigfile.close();
	}
}

void CurrencyModule::OnLoad(const QJsonDocument& doc) 
{
	QJsonObject docObj = doc.object();

	QStringList servers = docObj.keys();

	guildList.clear();

	//!User Data (Currency)
	for (auto server : servers) 
	{
		auto guildId = server.toULongLong();
		auto obj = docObj[server].toObject();
		QStringList users = obj.keys();

		QList<UserCurrency> list;
		for (auto user : users) {
			UserCurrency currencyData;
			currencyData.userId = user.toULongLong();
			currencyData.currency = obj[user].toObject()["currency"].toDouble();
			currencyData.isDailyClaimed = obj[user].toObject()["isDailyClaimed"].toBool();
			list.append(currencyData);
		}
		guildList.insert(guildId, list);
		
	}
	QFile currenConfigfile("configs/" + currenConfigLoc + ".json");
	if (currenConfigfile.open(QFile::ReadOnly)) 
	{
		QJsonDocument d = QJsonDocument::fromJson(currenConfigfile.readAll());
		QJsonObject rootObj = d.object();

		serverCurrencyConfig.clear();
		auto servers = rootObj.keys();
		for (const auto& server : servers) {
			CurrencyConfig config;
			auto serverObj = rootObj[server].toObject();
			config.currencyName = serverObj["name"].toString();
			config.currencySymbol = serverObj["symbol"].toString();
			config.giveawayChannelId = serverObj["freebieChannelId"].toString().toULongLong();
			config.dailyReward = serverObj["dailyReward"].toString().toInt();
			config.freebieReward = serverObj["freebieReward"].toString().toInt();
			config.gambleLoss = serverObj["gambleLoss"].toString().toInt();
			config.gambleReward = serverObj["gambleReward"].toString().toInt();
			config.minGuess = serverObj["gambleMinGuess"].toString().toInt();
			config.maxGuess = serverObj["gambleMaxGuess"].toString().toInt();
			config.randGiveawayProb = serverObj["freebieProb"].toString().toDouble();
			config.freebieExpireTime = serverObj["freebieExpireTime"].toString().toUInt();
			auto guildId = server.toULongLong();
			serverCurrencyConfig.insert(guildId, config);
		}

		
		currenConfigfile.close();
	}
}
