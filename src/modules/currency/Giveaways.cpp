#include "modules/currency/CurrencyModule.h"
#include "core/Utility.h"

using namespace Discord;

void CurrencyModule::initiateGiveaways(UmikoBot* client)
{
	m_timer.setInterval(24*60*60*1000); //!24hr timer
	QObject::connect(&m_timer, &QTimer::timeout, [this, client]() 
		{
			for (auto server : guildList.keys()) 
			{
				//!Reset the daily claimed bool for everyone
				for (int i = 0; i < guildList[server].size(); i++) 
				{
					auto& user = guildList[server][i];
					
					if (!user.isDailyClaimed)
					{
						user.dailyStreak = 0;
					}

					user.isDailyClaimed = false;

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
					QObject::connect(serverConfig.freebieTimer, &QTimer::timeout, [this, client, guildId] ()
						{
							auto& serverConfig = getServerData(guildId);
							serverConfig.allowGiveaway = false;
							serverConfig.giveawayClaimer = 0;
						});
					serverConfig.freebieTimer->start();
				}
				serverConfig.isRandomGiveawayDone = false;
				serverConfig.giveawayClaimer = 0;
			}
	});

	m_timer.start();

	auto holidaySpecialCheck = [this]()
	{
		// Y/M/D but we ignore the year (set to 1 because 0 is invalid)
		QList<QDate> specialDates = { QDate { 1, 12, 25 }, QDate { 1, 1, 1 } };
		QDate currentDate = QDate::currentDate();
		bool isHoliday = false;

		for (const QDate& date : specialDates)
		{
			if (date.month() == currentDate.month() && date.day() == currentDate.day())
			{
				isHoliday = true;
				break;
			}
		}

		if (isHoliday)
		{
			if (!isHolidaySpecialActive)
			{
				for (snowflake_t guild : serverCurrencyConfig.keys())
				{
					snowflake_t channel = serverCurrencyConfig[guild].giveawayChannelId;
					QString emoji = utility::consts::emojis::GIFT;
					UmikoBot::Instance().createMessage(channel, emoji + " **A holiday special is now occurring!** " + emoji + "\nLook out for gifts during the day!");
				}

				isHolidaySpecialActive = true;
				int numberOfMinutes = (qrand() % 30) + 15; // 15 - 45 after start
				holidaySpecialTimer.setInterval(numberOfMinutes * 60 * 1000);
				holidaySpecialTimer.start();
			}
		}
		else
		{
			if (isHolidaySpecialActive)
			{
				for (snowflake_t guild : serverCurrencyConfig.keys())
				{
					snowflake_t channel = serverCurrencyConfig[guild].giveawayChannelId;
					UmikoBot::Instance().createMessage(channel, "**The holiday special has ended!**\nBe sure to grab your gifts next time!");
				}

				isHolidaySpecialActive = false;
				isHolidaySpecialClaimable = false;
				holidaySpecialTimer.stop();
			}
		}
	};

	holidaySpecialCheckTimer.setInterval(1 * 60 * 60 * 1000); // Hourly timer
	QObject::connect(&holidaySpecialCheckTimer, &QTimer::timeout, holidaySpecialCheck);
	holidaySpecialCheckTimer.start();

	QObject::connect(&holidaySpecialTimer, &QTimer::timeout, [this, client]()
	{
		if (!isHolidaySpecialActive)
		{
			isHolidaySpecialClaimable = false;
			holidaySpecialTimer.stop();
			return;
		}

		if (isHolidaySpecialClaimable)
		{
			isHolidaySpecialClaimable = false;
			int numberOfMinutes = (qrand() % 40) + 40; // 40 - 80 minutes later
			holidaySpecialTimer.setInterval(numberOfMinutes * 60 * 1000);

			for (snowflake_t guild : serverCurrencyConfig.keys())
			{
				snowflake_t channel = serverCurrencyConfig[guild].giveawayChannelId;
				UmikoBot::Instance().createMessage(channel, "**The holiday gift window has closed!**\nThere may be another one soon...");;
			}
		}
		else
		{
			isHolidaySpecialClaimable = true;
			int numberOfMinutes = 5;
			holidaySpecialTimer.setInterval(numberOfMinutes * 60 * 1000);

			for (snowflake_t guild : serverCurrencyConfig.keys())
			{
				// Resets everyone's claimed flag
				for (UserCurrency& userCurrency : guildList[guild])
				{
					userCurrency.hasClaimedCurrentGift = false;
				}

				snowflake_t channel = serverCurrencyConfig[guild].giveawayChannelId;
				QString emoji = utility::consts::emojis::GIFT;
				QString msg = emoji + " **Holiday gift window is now open!** " + emoji + "\nGet your `!gift` within the next " + QString::number(numberOfMinutes) + " minutes!";
				UmikoBot::Instance().createMessage(channel, msg);
			}
		}
	});

	RegisterCommand(Commands::CURRENCY_DAILY, "daily", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');
		auto& config = getServerData(channel.guildId());

		if (args.size() > 1) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		int jailRemainingTime = guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())].jailTimer->remainingTime();
		
		if (getUserData(channel.guildId(), message.author().id()).isDailyClaimed)
		{
			QString time = utility::StringifyMilliseconds(m_timer.remainingTime());
			QString desc = "**You have already claimed your daily credits.**\nCome back after `" + time + "` to get richer!";

			client.createMessage(message.channelId(), desc);
		}
		else if (jailRemainingTime > 0)
		{
			QString time = utility::StringifyMilliseconds(jailRemainingTime);
			QString desc = "**You are in jail!**\nCome back after " + time + " to collect your daily credits.";
			client.createMessage(message.channelId(), desc);
			return;
		}
		else 
		{
			auto index = getUserIndex(channel.guildId(), message.author().id());
			unsigned int& dailyStreak = guildList[channel.guildId()][index].dailyStreak;
			Currency todaysReward = config.dailyReward;
			bool bonus = false;

			if (++dailyStreak % config.dailyBonusPeriod == 0)
			{
				todaysReward += config.dailyBonusAmount;
				bonus = true;
			}

			Currency current = guildList[channel.guildId()][index].currency();
			Currency newC = guildList[channel.guildId()][index].currency() + todaysReward;

			guildList[channel.guildId()][index].isDailyClaimed = true;
			guildList[channel.guildId()][index].setCurrency(guildList[channel.guildId()][index].currency() + todaysReward);
			guildList[channel.guildId()][index].numberOfDailysClaimed += 1;
			QString displayedMessage = "";

			Currency after = guildList[channel.guildId()][index].currency();

			if (bonus)
			{
				displayedMessage += "**Bonus!** ";
			}

			displayedMessage += "You now have **" + QString::number((double)todaysReward) + "** more " + getServerData(channel.guildId()).currencyName + "s in your wallet!\n";
			displayedMessage += "Streak: **" + QString::number(dailyStreak) + "/" + QString::number(config.dailyBonusPeriod) + "**";

			if (dailyStreak % config.dailyBonusPeriod == 0)
			{
				dailyStreak = 0;
			}

			client.createMessage(message.channelId(), displayedMessage);
		}
	});

	RegisterCommand(Commands::CURRENCY_CLAIM, "claim", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

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
					int jailRemainingTime = guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())].jailTimer->remainingTime();
					if (jailRemainingTime > 0)
					{
						QString time = utility::StringifyMilliseconds(jailRemainingTime);
						QString desc = "**You are in jail!**\nYou can't make claims to freebies for another " + time;
						client.createMessage(message.channelId(), desc);
						return;
					}
					
					auto& config = getServerData(channel.guildId());
					config.giveawayClaimer = message.author().id();
					Embed embed;
					embed.setColor(qrand() % 11777216);
					embed.setTitle("Claim FREEBIE");
					embed.setDescription(":drum: And today's FREEBIE goes to **" + message.author().username() + "**! \n\n Congratulations! You just got **"+ QString::number((double)config.freebieReward) + config.currencySymbol +"**!");

					auto index = getUserIndex(channel.guildId(), message.author().id());

					guildList[channel.guildId()][index].setCurrency(guildList[channel.guildId()][index].currency() + config.freebieReward);
					guildList[channel.guildId()][index].numberOfGiveawaysClaimed += 1;

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
				auto& config = getServerData(channel.guildId());
				Embed embed;
				embed.setColor(qrand()%11777216);
				embed.setTitle("Claim FREEBIE");
				embed.setDescription("Sorry, today's freebie has been claimed by " + UmikoBot::Instance().GetName(channel.guildId(), config.giveawayClaimer) + " :cry: \n\n But you can always try again the next day!");
				client.createMessage(message.channelId(), embed);
			}
		}
	});

	RegisterCommand(Commands::CURRENCY_GIFT, "gift", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		if (args.size() > 1)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (!isHolidaySpecialActive)
		{
			client.createMessage(message.channelId(), "**Today is not a special day!**\nWait for one to arrive before looking for gifts.");
			return;
		}

		if (!isHolidaySpecialClaimable)
		{
			client.createMessage(message.channelId(), "**There is no gift available at the moment!**");
			return;
		}

		int jailRemainingTime = guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())].jailTimer->remainingTime();
		if (jailRemainingTime > 0)
		{
			QString time = utility::StringifyMilliseconds(jailRemainingTime);
			QString desc = "**You are in jail!**\nYou can't receive gifts for another " + time;
			client.createMessage(message.channelId(), desc);
			return;
		}

		auto& userCurrency = getUserData(channel.guildId(), message.author().id());
		if (userCurrency.hasClaimedCurrentGift)
		{
			client.createMessage(message.channelId(), "**You have already claimed this gift!**\nDon't bother me until the next one...");
			return;
		}

		int amountReceived = (qrand() % 6) + 20;
		userCurrency.setCurrency(userCurrency.currency() + amountReceived);
		userCurrency.hasClaimedCurrentGift = true;

		QString name = UmikoBot::Instance().GetName(channel.guildId(), message.author().id());
		QString amountString = QString::number(amountReceived);
		QString currencySymbol = getServerData(channel.guildId()).currencySymbol;
		QString emoji = utility::consts::emojis::GIFT_HEART;
		QString msg = emoji + " **Congratulations!** " + emoji + "\n" + name + " has been gifted `" + amountString + " " + currencySymbol + "`";
		client.createMessage(message.channelId(), msg);
	});
}
