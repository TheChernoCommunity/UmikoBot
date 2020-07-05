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

//! Gamble Timeout in seconds
#define gambleTimeout 20

CurrencyModule::CurrencyModule(UmikoBot* client)
	: Module("currency", true) 
{

	m_timer.setInterval(24*60*60*1000); //!24hr timer
	QObject::connect(&m_timer, &QTimer::timeout, [this, client]() 
		{
		
		//!Clear the daily bonus for everyone
		for (auto it = m_settings.begin(); it != m_settings.end(); ++it
			) 
		{
			it->isDailyClaimed = false;
		}

		if (!isRandomGiveAwayDone) 
		{
			client->createMessage(config.giveawayChannelId, "Hey everyone! Today's freebie expires in **"+ QString::number(config.freebieExpireTime) +" seconds**. `!claim` it now!");

			allowGiveaway = true;

			QTimer timer;
			timer.setInterval(config.freebieExpireTime * 1000);
			QObject::connect(&timer, &QTimer::timeout, 
				[this, client]() 
				{
					if(isRandomGiveAwayDone)
						isRandomGiveAwayDone = false;

				});
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
			QString creditScore = QString::number(m_settings[message.author().id()].currency);
			QString desc = "**Current Credits: ** `" + creditScore + "` **" + config.currencySymbol + "** (" + config.currencyName +")";
			embed.setTitle("Your Wallet");
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
		if (m_settings[message.author().id()].isDailyClaimed)
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
			m_settings[message.author().id()].isDailyClaimed = true;
			m_settings[message.author().id()].currency += config.dailyReward;

			client.createMessage(message.channelId(), "**You now have "+ QString::number(config.dailyReward) + " more " +  config.currencyName + "(s) in your wallet!**");
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
			if (selfGambleData.gamble) 
			{
				Discord::Embed embed;
				embed.setColor(qrand() % 16777216);
				embed.setTitle("Welcome to Gamble!");
				embed.setDescription("**Sorry but this feature is currently in use by another person. Please try again later!**");
				client.createMessage(message.channelId(), embed);
				return;
			}

			selfGambleData.randNum = qrand() % (config.maxGuess - config.minGuess +1) + config.minGuess;
			selfGambleData.channelId = message.channelId();
			selfGambleData.gamble = true;
			selfGambleData.userId = message.author().id();

			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Welcome to Gamble!");
			embed.setDescription("All you need to do is guess a random number between "+ QString::number(config.minGuess) + " and " + QString::number(config.maxGuess) + " (inclusive) and if it is the same as the number I guess, you get **"+ QString::number(config.gambleReward) + config.currencySymbol + "**!\n\n**What number do you think of?** <:wesmart:388340133864407043>");
			
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
			if (selfGambleData.gamble) 
			{
				Discord::Embed embed;
				embed.setColor(qrand() % 16777216);
				embed.setTitle("Welcome to Gamble!");
				embed.setDescription("**Sorry but this feature is currently in use by another person. Please try again later!**");
				client.createMessage(message.channelId(), embed);
				return;
			}
			if (args.at(1).toDouble() > gamblebetMax) 
			{
				client.createMessage(channel.id(), "You cannot bet an amount more than **" + QString::number(gamblebetMax) + config.currencySymbol+"**");
				return;
			}
			selfGambleData.randNum = qrand() % (config.maxGuess - config.minGuess + 1) + config.minGuess;
			selfGambleData.channelId = message.channelId();
			selfGambleData.gamble = true;
			selfGambleData.userId = message.author().id();
			selfGambleData.doubleOrNothing = true;
			selfGambleData.betAmount = args.at(1).toDouble();

			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Welcome to Gamble (Double or Nothing)!");
			embed.setDescription("All you need to do is guess a random number between " + QString::number(config.minGuess) + " and " + QString::number(config.maxGuess) + " (inclusive) and if it is the same as the number I guess, you get double the amount you just bet: **" + QString::number(2*selfGambleData.betAmount) + config.currencySymbol + "**!\n\n**What number do you think of?**");

			client.createMessage(message.channelId(), embed);
		}

		//! Set a timer to reset if the player doesn't respond
		auto timer = new QTimer();
		timer->setSingleShot(true);
		QObject::connect(timer, &QTimer::timeout, [this, &message, &client]() 
			{
				if (selfGambleData.gamble) {

					selfGambleData.gamble = false;

					client.createMessage(message.channelId(), "**Gamble Timeout due to no response.**");
				}
			});

		timer->start(gambleTimeout * 1000);

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
			if (!isRandomGiveAwayDone) 
			{
				if (allowGiveaway) 
				{
					Discord::Embed embed;
					embed.setColor(qrand() % 11777216);
					embed.setTitle("Claim FREEBIE");
					embed.setDescription(":drum: And today's FREEBIE goes to **" + message.author().username() + "**! \n\n Congratulations! You just got **"+ QString::number(config.freebieReward) + config.currencySymbol +"**!");

					m_settings[message.author().id()].currency += config.freebieReward;

					client.createMessage(message.channelId(), embed);
					isRandomGiveAwayDone = true;
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
				embed.setDescription("Sorry, today's freebie has been claimed (or it expired) :cry: \n\n But you can always try again the next day!");
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
			[args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}});

		if (args.size() > 1) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 1) 
		{
			config.giveawayChannelId = message.channelId();
			client.createMessage(message.channelId(), "**Giveaway announcement channel successfully changed to current channel.**");
		}

		});

	RegisterCommand(Commands::CURRENCY_SET_NAME, "setcurrenname", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{
		
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.size() == 1) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}});

		if (args.first() != prefix + "setcurrenname")
			return;

		if (args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) 
		{
			config.currencyName = args.at(1);
			client.createMessage(message.channelId(), "**Currency Name set to** " + config.currencyName);
		}

		});

	RegisterCommand(Commands::CURRENCY_SET_SYMBOL, "setcurrensymb", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}});

		if (args.size() == 1) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.first() != prefix + "setcurrensymb")
			return;

		if (args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) 
		{
			config.currencySymbol = args.at(1);
			client.createMessage(message.channelId(), "**Currency Symbol set to** " + config.currencySymbol);
		}

		});

	RegisterCommand(Commands::CURRENCY_SET_DAILY, "setdaily", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setdaily")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}});

		if (args.size() == 1 || args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) 
		{
			config.dailyReward = args.at(1).toInt();
			client.createMessage(message.channelId(), "**Daily Reward Amount set to **" + QString::number(config.dailyReward));
		}

		});

	RegisterCommand(Commands::CURRENCY_SET_PRIZE , "setprize", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setprize")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}});

		if (args.size() == 1 || args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) 
		{
			config.freebieReward = args.at(1).toInt();
			client.createMessage(message.channelId(), "**Freebie Reward Amount set to **" + QString::number(config.freebieReward));
		}

		});

	RegisterCommand(Commands::CURRENCY_SET_PRIZE_PROB, "setprizeprob", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setprizeprob")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}});

		if (args.size() == 1 || args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) 
		{
			config.randGiveawayProb = args.at(1).toDouble();
			client.createMessage(message.channelId(), "**Giveaway Probability Amount set to **" + QString::number(config.randGiveawayProb));
		}

		});

	RegisterCommand(Commands::CURRENCY_SET_PRIZE_EXPIRY, "setprizeexpiry", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setprizeexpiry")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}});

		if (args.size() == 1 || args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) 
		{
			config.freebieExpireTime = args.at(1).toUInt();
			client.createMessage(message.channelId(), "**Freebie Expiry Time (secs) set to **" + QString::number(config.freebieExpireTime));
		}

		});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_LOSS, "setgambleloss", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setgambleloss")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}});

		if (args.size() == 1 || args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) 
		{
			config.gambleLoss = args.at(1).toInt();
			client.createMessage(message.channelId(), "**Gamble Loss Amount set to **" + QString::number(config.gambleLoss));
		}
		});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_MAX_GUESS, "setgamblemaxguess", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "setgamblemaxguess")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}});

		if (args.size() == 1 || args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) 
		{
			config.maxGuess = args.at(1).toInt();
			client.createMessage(message.channelId(), "**Gamble Max Guess set to **" + QString::number(config.maxGuess));
		}
		});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_MIN_GUESS, "setgambleminguess", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{
		
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setgambleminguess")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}});

		if (args.size() == 1 || args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) 
		{
			config.minGuess = args.at(1).toInt();
			client.createMessage(message.channelId(), "**Gamble Min Guess set to **" + QString::number(config.minGuess));
		}
		
		});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_REWARD, "setgamblereward", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "setgamblereward")
			return;

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[args, &client, message, channel](bool result) 
			{
				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result) 
				{
					client.createMessage(message.channelId(), "You don't have permissions to use this command.");
					return;
				}});

		if (args.size() == 1 || args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) 
		{
			config.gambleReward = args.at(1).toInt();
			client.createMessage(message.channelId(), "**Gamble Reward Amount set to **" + QString::number(config.gambleReward));
		}


		});
}

void CurrencyModule::StatusCommand(QString& result, snowflake_t guild, snowflake_t user) 
{
	QString creditScore = QString::number(m_settings[user].currency);
	result += "**Wallet: **" + creditScore + " **" + config.currencySymbol +"**";
	result+='\n';
}

void CurrencyModule::OnMessage(Discord::Client& client, const Discord::Message& message) 
{

	if (!isRandomGiveAwayDone) 
	{
		static int pingCount = 0;
		if (pingCount == 0) 
		{
			randp.param(std::bernoulli_distribution::param_type(config.randGiveawayProb));
			bool outcome = randp(random_engine);
			if (outcome) 
			{
				client.createMessage(config.giveawayChannelId, "Hey everyone! **FREEBIE** available now! Go `!claim` some juicy coins!");
				allowGiveaway = true;
				pingCount++;
			}
		}
	}

	//! If the message is a number, continue with the gambling mech
	QRegExp re("\\d*");
	if (re.exactMatch(message.content())) 
	{
		if (selfGambleData.gamble && !message.author().bot() && message.channelId() == selfGambleData.channelId && message.author().id() == selfGambleData.userId) 
		{

			if (message.content().toInt() > config.maxGuess || message.content().toInt() < config.minGuess) 
			{

				client.createMessage(message.channelId(), "**Your guess is out of range!** \nTry a number between " + QString::number(config.minGuess) + " and " + QString::number(config.maxGuess) + " (inclusive). ");
				return;

			}
			if (message.content().toInt() == selfGambleData.randNum) 
			{
				if (!selfGambleData.doubleOrNothing) 
				{
					m_settings[message.author().id()].currency += config.gambleReward;
					client.createMessage(message.channelId(), "**You guessed CORRECTLY!**\n(**" + QString::number(config.gambleReward) + config.currencySymbol + "** have been added to your wallet!)");
				}
				else 
				{
					m_settings[message.author().id()].currency += 2 * selfGambleData.betAmount;
					client.createMessage(message.channelId(), "**You guessed CORRECTLY!**\n(**" + QString::number(2 * selfGambleData.betAmount) + config.currencySymbol + "** have been added to your wallet!)");
				}

			}

			//! Player lost
			else 
			{
				if (!selfGambleData.doubleOrNothing) 
				{
					m_settings[message.author().id()].currency -= config.gambleLoss;
					client.createMessage(message.channelId(), "**Better Luck next time!**\n*(psst! I took **" + QString::number(config.gambleLoss) + config.currencySymbol
						+ "** from your wallet for my time...)*");
				}
				else 
				{
					m_settings[message.author().id()].currency -= selfGambleData.betAmount;
					client.createMessage(message.channelId(), "**Better Luck next time!**\n*(psst! I took **" + QString::number(selfGambleData.betAmount) + config.currencySymbol
						+ "** from your wallet...)*");
				}

			}

			selfGambleData.gamble = false;
			selfGambleData.doubleOrNothing = false;
		}
	}

	Module::OnMessage(client, message);
}

void CurrencyModule::OnSave(QJsonDocument& doc) const 
{
	QJsonObject docObj;
	// User Data
	for (auto it = m_settings.begin(); it != m_settings.end(); ++it) 
	{
		QJsonObject obj;
		obj["currency"] = it->currency;
		obj["isDailyClaimed"] = it->isDailyClaimed;

		docObj[QString::number(it.key())] = obj;
	}

	doc.setObject(docObj);

	QFile currenConfigfile("configs/" + currenConfigLoc + ".json");
	if (currenConfigfile.open(QFile::ReadWrite | QFile::Truncate)) 
	{
		QJsonDocument doc;
		QJsonObject obj;
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

		doc.setObject(obj);
		currenConfigfile.write(doc.toJson());
		currenConfigfile.close();
	}
}

void CurrencyModule::OnLoad(const QJsonDocument& doc) 
{
	QJsonObject docObj = doc.object();

	for (auto it = docObj.begin(); it != docObj.end(); ++it) 
	{
		const QJsonObject obj = it.value().toObject();
		Setting& setting = m_settings[it.key().toULongLong()];
		setting.currency = obj["currency"].toDouble();
		setting.isDailyClaimed = obj["isDailyClaimed"].toBool();
	}

	QFile currenConfigfile("configs/" + currenConfigLoc + ".json");
	if (currenConfigfile.open(QFile::ReadOnly)) 
	{
		QJsonDocument d = QJsonDocument::fromJson(currenConfigfile.readAll());
		QJsonObject rootObj = d.object();

		config.currencyName = rootObj["name"].toString();
		config.currencySymbol = rootObj["symbol"].toString();
		config.giveawayChannelId = rootObj["freebieChannelId"].toString().toULongLong();
		config.dailyReward = rootObj["dailyReward"].toString().toInt();
		config.freebieReward = rootObj["freebieReward"].toString().toInt();
		config.gambleLoss = rootObj["gambleLoss"].toString().toInt();
		config.gambleReward = rootObj["gambleReward"].toString().toInt();
		config.minGuess = rootObj["gambleMinGuess"].toString().toInt();
		config.maxGuess = rootObj["gambleMaxGuess"].toString().toInt();
		config.randGiveawayProb = rootObj["freebieProb"].toString().toDouble();
		config.freebieExpireTime = rootObj["freebieExpireTime"].toString().toUInt();

		currenConfigfile.close();
	}
}
