#include "modules/currency/CurrencyModule.h"
#include "modules/event/EventModule.h"
#include "core/Utility.h"

#include <QtCore/qregexp.h>
#include <QtCore/QtMath>
#include <random>

//! Maximum amount that can be bet
#define gamblebetMax Currency(100)

//! Maximum debt that a user can be in
#define debtMax Currency(-100)

//! Gamble Timeout in seconds
#define gambleTimeout 20

using namespace Discord;

void CurrencyModule::initiateEarnCommands()
{
	RegisterCommand(Commands::CURRENCY_GAMBLE, "gamble", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');
		
		if (args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		int jailRemainingTime = guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())].jailTimer->remainingTime();
		if (jailRemainingTime > 0)
		{
			QString time = utility::StringifyMilliseconds(jailRemainingTime);
			QString desc = "**You are in jail!**\nCome back after " + time + " to gamble.";
			client.createMessage(message.channelId(), desc);
			return;
		}
		
		//! Normal Mode
		if (args.size() == 1)
		{

			auto& serverGamble = gambleData[channel.guildId()];
			if (guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())].currency() - getServerData(channel.guildId()).gambleLoss < debtMax)
			{
				client.createMessage(message.channelId(), "**Nope, can't let you get to serious debt.**");
				return;
			}
			if (serverGamble.gamble) 
			{
				QString user = UmikoBot::Instance().GetName(channel.guildId(), serverGamble.userId);
				Embed embed;
				embed.setColor(qrand() % 16777216);
				embed.setTitle("Welcome to Gamble!");
				embed.setDescription("Sorry but this feature is currently in use by **" + user + "**. Please try again later!");
				client.createMessage(message.channelId(), embed);
				return;
			}

			auto& config = getServerData(channel.guildId());

			std::random_device device;
			std::mt19937 rng(device());
			std::uniform_int_distribution<std::mt19937::result_type> dist(config.minGuess, config.maxGuess);

			serverGamble.randNum = dist(rng);
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

			Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Welcome to Gamble " + name + "!");
			embed.setDescription("All you need to do is guess a random number between " + QString::number(config.minGuess) + " and " + QString::number(config.maxGuess) + " (inclusive) and if it is the same as the number I think of, you get **" + QString::number((double)config.gambleReward) + config.currencySymbol + "**!\n\n**What number do you think of?** " + utility::consts::emojis::WE_SMART);
			
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

			re = QRegExp("[+]?\\d*\\.?\\d{1,2}");
			if (!re.exactMatch(args.at(1)))
			{
				client.createMessage(message.channelId(), "**You may only specify amounts with 2 decimal places or fewer**");
				return;
			}

			auto& serverGamble = gambleData[channel.guildId()];
			auto& config = getServerData(channel.guildId());

			if (guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())].currency() - args.at(1).toDouble() < debtMax)
			{
				client.createMessage(message.channelId(), "**Nope, can't let you get to serious debt.**");
				return;
			}
			if (serverGamble.gamble)
			{
				QString user = UmikoBot::Instance().GetName(channel.guildId(), serverGamble.userId);
				Embed embed;
				embed.setColor(qrand() % 16777216);
				embed.setTitle("Welcome to Gamble!");
				embed.setDescription("Sorry but this feature is currently in use by **" + user + "**. Please try again later!");
				client.createMessage(message.channelId(), embed);
				return;
			}
			if (args.at(1).toDouble() > (double)gamblebetMax) 
			{
				client.createMessage(channel.id(), "You cannot bet an amount more than **" + QString::number((double)gamblebetMax) + config.currencySymbol+"**");
				return;
			}
			if (args.at(1).toDouble() == 0) 
			{
				client.createMessage(message.channelId(), QString(utility::consts::emojis::AANGER) + " **BRUH. Don't you dare waste my time! I ain't interested in nothing.**");
				return;
			}

			std::random_device device;
			std::mt19937 rng(device());
			std::uniform_int_distribution<std::mt19937::result_type> dist(config.minGuess, config.maxGuess);

			serverGamble.randNum = dist(rng);
			serverGamble.channelId = message.channelId();
			serverGamble.gamble = true;
			serverGamble.userId = message.author().id();
			serverGamble.doubleOrNothing = true;
			serverGamble.betAmount = args.at(1).toDouble();

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

			Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Welcome to Gamble (Double or Nothing) " + name + "!");
			embed.setDescription("All you need to do is guess a random number between " + QString::number(config.minGuess) + " and " + QString::number(config.maxGuess) + " (inclusive) and if it is the same as the number I guess, you get double the amount you just bet: **" + QString::number(2 * (double)serverGamble.betAmount) + config.currencySymbol + "**!\n\n**What number do you think of?** " + utility::consts::emojis::WE_SMART);

			client.createMessage(message.channelId(), embed);
		}

	});

	RegisterCommand(Commands::CURRENCY_DONATE, "donate", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		if (args.size() == 1 || args.size() == 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}
		QRegExp re("[+]?\\d*\\.?\\d+");
		if (!re.exactMatch(args.at(1))) 
		{
				client.createMessage(message.channelId(), "**You can't donate in invalid amounts**");
				return;
		}

		re = QRegExp("[+]?\\d*\\.?\\d{1,2}");
		if (!re.exactMatch(args.at(1)))
		{
			client.createMessage(message.channelId(), "**You may only specify amounts with 2 decimal places or fewer**");
			return;
		}

		if (args.at(1).toDouble() == 0) 
		{
			client.createMessage(message.channelId(), "**Please don't donate at all if you don't want to donate anything.**");
			return;
		}

		if (guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())].currency() - args.at(1).toDouble() < 0)
		{
			client.createMessage(message.channelId(), "**I can't let you do that, otherwise you'll be in debt!**");
			return;
		}

		QList<User> mentions = message.mentions();
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
			if (user.bot())
			{
				client.createMessage(message.channelId(), QString("**Just _who_ do you think supplies your money here? Donate it to someone else!** <:") + utility::consts::emojis::reacts::ANGRY_PING + QString(">"));
				return;
			}
		}

		auto& userCurrency = guildList[channel.guildId()][getUserIndex(channel.guildId(), message.author().id())];
		userCurrency.setCurrency(userCurrency.currency() - args.at(1).toDouble());

		Currency donation = args.at(1).toDouble() / mentions.size();
		QString desc = "<@" + QString::number(message.author().id()) + "> donated **" + QString::number((double)donation) + getServerData(channel.guildId()).currencySymbol + "** to";

		for (auto user : mentions) 
		{
			auto index = getUserIndex(channel.guildId(), user.id());
			desc += " <@" + QString::number(user.id()) + ">";
			guildList[channel.guildId()][index].setCurrency(guildList[channel.guildId()][index].currency() + donation);
		}

		Embed embed;
		embed.setTitle("Donation by " + UmikoBot::Instance().GetName(channel.guildId(), message.author().id()) + "!");
		embed.setDescription(desc);
		embed.setColor(qrand() % 16777216);

		client.createMessage(message.channelId(), embed);

	});

	RegisterCommand(Commands::CURRENCY_BRIBE, "bribe", [this](Client& client, const Message& message, const Channel& channel)
	{

		QStringList args = message.content().split(' ');
		auto& config = getServerData(channel.guildId());
		snowflake_t authorId = message.author().id();

		auto& authorCurrency = guildList[channel.guildId()][getUserIndex(channel.guildId(), authorId)];
		int remainingJailTime = authorCurrency.jailTimer->remainingTime();
		bool inJail = authorCurrency.jailTimer->isActive();

		if (inJail == false)
		{
			QString output = QString(
				":police_officer: **Hey you're not in JAIL!** :police_officer:\n"
			);
			client.createMessage(message.channelId(), output);
			return;
		}

		if (args.size() != 2)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!**");
			return;
		}

		QRegExp re("[+]?\\d*\\.?\\d+");
		if (!re.exactMatch(args.at(1)))
		{
			client.createMessage(message.channelId(), "**Your bribe amount must be valid!**");
			return;
		}

		re = QRegExp("[+]?\\d*\\.?\\d{1,2}");
		if (!re.exactMatch(args.at(1)))
		{
			client.createMessage(message.channelId(), "**You may only specify amounts with 2 decimal places or fewer**");
			return;
		}

		Currency amountToBribe = args.at(1).toDouble();

		auto guildID = channel.guildId();
		QObject::connect(authorCurrency.jailTimer, &QTimer::timeout, [this, &client, guildID, authorId]()
			{
				guildList[guildID][getUserIndex(guildID, authorId)].isBribeUsed = false;
			});

		if (authorCurrency.isBribeUsed)
		{
			QString output = QString(
				":police_officer: **You already tried to bribe me and failed... Do you want me to extend your sentence again?** :police_officer:\n"
			);

			client.createMessage(message.channelId(), output);
			return;
		}

		else if (amountToBribe > config.bribeMaxAmount)
		{
			QString maxBribeAmount = QString::number((double)config.bribeMaxAmount);
			QString output = QString(
				":police_officer: **If I take more than `%1` %2 then I might get caught... ** :police_officer:\n"
			).arg(maxBribeAmount, config.currencySymbol);

			client.createMessage(message.channelId(), output);
			return;
		}

		else if (amountToBribe < config.bribeLeastAmount)
		{
			QString leastBribeAmount = QString::number((double)config.bribeLeastAmount);
			QString output = QString(
				":police_officer: **Pfft! Such measly amounts... Do you want to be in jail for longer?** :police_officer:\n"
				"*(You can always give me `%1 %2` or more... maybe then I could do something?)*\n"
			).arg(leastBribeAmount, config.currencySymbol);

			client.createMessage(message.channelId(), output);
			return;
		}

		if (authorCurrency.currency() - (amountToBribe) < debtMax)
		{
			client.createMessage(message.channelId(), ":police_officer: **I would love to be bribed with that much money but unfortunately you can't afford it.** :police_officer:");
			return;
		}

		// This success chance is in the range of 0 to 1
		double successChance = (static_cast<double>(config.bribeSuccessChance) / (static_cast<double>(config.bribeMaxAmount) * 100)) * static_cast<double>(amountToBribe);

		QString authorName = UmikoBot::Instance().GetName(channel.guildId(), authorId);

		std::random_device device;
		std::mt19937 prng{ device() };
		std::discrete_distribution<> distribution{ { 1 - successChance, successChance } };
		int roll = distribution(prng);

		if (roll)
		{
			authorCurrency.setCurrency(authorCurrency.currency() - amountToBribe);
			authorCurrency.jailTimer->stop();

			QString output = QString(
				":unlock: **Thanks for the BRIBE!** :unlock:\n"
				"*%1* you are free from jail now!.\n"
			).arg(authorName);

			client.createMessage(message.channelId(), output);

			snowflake_t chan = message.channelId();
			UmikoBot::Instance().createReaction(chan, message.id(), utility::consts::emojis::reacts::PARTY_CAT);
		}
		else
		{
			authorCurrency.isBribeUsed = true;
			int totalTime = remainingJailTime + 3600000;
			authorCurrency.jailTimer->start(totalTime);

			QString time = utility::StringifyMilliseconds(totalTime);

			QString output = QString(
				":police_officer: **Your bribes don't affect my loyalty!** :police_officer:\n"
				"You have been reported and your sentence has been extended by `1` hour!\n"
				"*(You need to wait %1 to get out of jail)*"
			).arg(time);
			client.createMessage(message.channelId(), output);
		}
	});

	RegisterCommand(Commands::CURRENCY_STEAL, "steal", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');
		auto& config = getServerData(channel.guildId());
		snowflake_t authorId = message.author().id();

		if (args.size() != 3)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!**");
			return;
		}

		int jailRemainingTime = guildList[channel.guildId()][getUserIndex(channel.guildId(), authorId)].jailTimer->remainingTime();
		if (jailRemainingTime > 0)
		{
			QString time = utility::StringifyMilliseconds(jailRemainingTime);
			QString desc = "**You are in jail!**\nCome back after " + time + " to steal more.";
			client.createMessage(message.channelId(), desc);
			return;
		}

		QRegExp re("[+]?\\d*\\.?\\d+");
		if (!re.exactMatch(args.at(1)))
		{
			client.createMessage(message.channelId(), "**You can't steal invalid amounts**");
			return;
		}

		re = QRegExp("[+]?\\d*\\.?\\d{1,2}");
		if (!re.exactMatch(args.at(1)))
		{
			client.createMessage(message.channelId(), "**You may only specify amounts with 2 decimal places or fewer**");
			return;
		}

		Currency amountToSteal = args.at(1).toDouble();
		if (amountToSteal == 0.0)
		{
			client.createMessage(message.channelId(), "**Don't bother me if you don't want to steal anything.**");
			return;
		}

		if (guildList[channel.guildId()][getUserIndex(channel.guildId(), authorId)].currency() - (amountToSteal * config.stealFinePercent / 100.0) < debtMax)
		{
			client.createMessage(message.channelId(), "**I can't let you do that, you might go into serious debt.**");
			return;
		}

		QList<User> mentions = message.mentions();
		if (mentions.size() == 0)
		{
			client.createMessage(message.channelId(), "**Who do you want to steal from? Please `@` that person.**");
			return;
		}
		else if (mentions.size() > 1)
		{
			client.createMessage(message.channelId(), "**You can only steal from one person at a time.**");
			return;
		}

		snowflake_t victimId = mentions[0].id();
		if (victimId == authorId)
		{
			client.createMessage(message.channelId(), "**You cannot steal from yourself.**");
			return;
		}
		if (mentions[0].bot())
		{
			client.createMessage(message.channelId(), "**Nice try, but we machines are too advanced to be stolen from.**");
			return;
		}

		if (guildList[channel.guildId()][getUserIndex(channel.guildId(), victimId)].currency() - amountToSteal < debtMax)
		{
			client.createMessage(message.channelId(), "**I can't let you make your victim go into serious debt.**");
			return;
		}
		QString thiefName = UmikoBot::Instance().GetName(channel.guildId(), authorId);
		QString victimName = UmikoBot::Instance().GetName(channel.guildId(), victimId);

		auto& victimCurrency = guildList[channel.guildId()][getUserIndex(channel.guildId(), victimId)];
		auto& authorCurrency = guildList[channel.guildId()][getUserIndex(channel.guildId(), authorId)];

		EventModule* eventModule = static_cast<EventModule*>(UmikoBot::Instance().GetModuleByName("event"));
		auto& eventConfig = eventModule->getServerEventData(channel.guildId());

		// https://www.desmos.com/calculator/z6b0k7wb1t
		// This success chance is in the range of 0 to 1
		double successChance;
		if (eventConfig.isHighRiskHighRewardRunning)
		{
			successChance = (config.highRiskRewardStealSuccessChance / 100.0)* qExp(-0.0001 * qPow((double)amountToSteal, 1.5));
		}
		else if (eventConfig.isLowRiskLowRewardRunning)
		{
			successChance = (config.lowRiskRewardStealSuccessChance / 100.0) * qExp(-0.0001 * qPow((double)amountToSteal, 1.5));
		}
		else if(!eventConfig.isHighRiskHighRewardRunning || !eventConfig.isLowRiskLowRewardRunning)
		{
			successChance = (config.stealSuccessChance / 100.0) * qExp(-0.0001 * qPow((double)amountToSteal, 1.5));
		}

		std::random_device device;
		std::mt19937 prng { device() };
		std::discrete_distribution<> distribution { { 1 - successChance, successChance } };
		int roll = distribution(prng);

		if (roll)
		{
			if (eventConfig.isHighRiskHighRewardRunning)
			{
				Currency bonus = (double)amountToSteal * highRiskRewardBonus * 0.01;
				victimCurrency.setCurrency(victimCurrency.currency() - amountToSteal);
				authorCurrency.setCurrency(authorCurrency.currency() + amountToSteal + bonus);
				QString num = QString::number((double)bonus);

				QString stealAmount = QString::number((double)amountToSteal);
				QString output = QString(
					":man_detective: **Steal success!** :man_detective:\n"
					"*%1* has discreetly stolen **`%2 %3`** from under *%4's* nose.\n"
					"HighRiskHighReward event **BONUS**: `%5 %3`\n"
				).arg(thiefName, stealAmount, config.currencySymbol, victimName, num);

				client.createMessage(message.channelId(), output);
				return;
			}
			if (eventConfig.isLowRiskLowRewardRunning)
			{
				Currency penalty = (double)amountToSteal * lowRiskRewardPenalty * 0.01;
				victimCurrency.setCurrency(victimCurrency.currency() - (amountToSteal - penalty));
				authorCurrency.setCurrency(authorCurrency.currency() + (amountToSteal - penalty));
				QString num = QString::number((double)penalty);

				QString stealAmount = QString::number((double)amountToSteal);
				QString output = QString(
					":man_detective: **Steal success!** :man_detective:\n"
					"*%1* has discreetly stolen **`%2 %3`** from under *%4's* nose.\n"
					"LowRiskLowReward event **PENALTY**: `%5 %3`\n"
				).arg(thiefName, stealAmount, config.currencySymbol, victimName, num);
				client.createMessage(message.channelId(), output);
				return;
			}
			victimCurrency.setCurrency(victimCurrency.currency() - amountToSteal);
			authorCurrency.setCurrency(authorCurrency.currency() + amountToSteal);

			QString stealAmount = QString::number((double)amountToSteal);
			QString output = QString(
					":man_detective: **Steal success!** :man_detective:\n"
					"*%1* has discreetly stolen **`%2 %3`** from under *%4's* nose.\n"
				).arg(thiefName, stealAmount, config.currencySymbol, victimName);

			client.createMessage(message.channelId(), output);
		}
		else
		{
			authorCurrency.setCurrency(authorCurrency.currency() - amountToSteal * config.stealFinePercent / 100.0);
			victimCurrency.setCurrency(victimCurrency.currency() + amountToSteal * config.stealVictimBonusPercent / 100.0);
			authorCurrency.jailTimer->start(config.stealFailedJailTime * 60 * 60 * 1000);

			QString fineAmount = QString::number((double)amountToSteal * config.stealFinePercent / 100.0);
			QString victimBonus = QString::number((double)amountToSteal * config.stealVictimBonusPercent / 100.0);
			QString jailTime = QString::number(config.stealFailedJailTime);
			QString output = QString(
					":rotating_light: **You got caught!** :rotating_light:\n"
					"*%1* has been fined **`%2 %3`** and placed in jail for %4 hours.\n"
					"*%5* has been granted **`%6 %3`** in insurance."
				).arg(thiefName, fineAmount, config.currencySymbol, jailTime, victimName, victimBonus);
				
			client.createMessage(message.channelId(), output);
		}
	});


}
