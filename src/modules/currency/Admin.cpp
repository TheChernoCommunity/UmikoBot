#include "modules/currency/CurrencyModule.h"
#include "modules/event/EventModule.h"
#include "core/Permissions.h"

using namespace Discord;

void CurrencyModule::initiateAdminCommands()
{
	RegisterCommand(Commands::CURRENCY_SET_PRIZE_CHANNEL, "setannouncechan", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 1, args, false, [this, &client, channel, message]()
		{
			auto& config = getServerData(channel.guildId());
			config.giveawayChannelId = message.channelId();
			client.createMessage(message.channelId(), "**Giveaway announcement channel successfully changed to current channel.**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_NAME, "setcurrenname", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.currencyName = args.at(1);
			client.createMessage(message.channelId(), "Currency Name set to **" + config.currencyName + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_SYMBOL, "setcurrensymb", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.currencySymbol = args.at(1);
			client.createMessage(message.channelId(), "Currency Symbol set to **" + config.currencySymbol + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_DAILY, "setdaily", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.dailyReward = args.at(1).toInt();
			client.createMessage(message.channelId(), "Daily Reward Amount set to **" + QString::number((double)config.dailyReward) + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_PRIZE , "setprize", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.freebieReward = args.at(1).toInt();
			client.createMessage(message.channelId(), "Freebie Reward Amount set to **" + QString::number((double)config.freebieReward) + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_PRIZE_PROB, "setprizeprob", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.randGiveawayProb = args.at(1).toDouble();
			client.createMessage(message.channelId(), "Giveaway Probability Amount set to **" + QString::number(config.randGiveawayProb) + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_PRIZE_EXPIRY, "setprizeexpiry", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.freebieExpireTime = args.at(1).toUInt();
			client.createMessage(message.channelId(), "Freebie Expiry Time (secs) set to **" + QString::number(config.freebieExpireTime) + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_LOSS, "setgambleloss", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.gambleLoss = args.at(1).toInt();
			client.createMessage(message.channelId(), "Gamble Loss Amount set to **" + QString::number((double)config.gambleLoss) + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_MAX_GUESS, "setgamblemaxguess", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.maxGuess = args.at(1).toInt();
			client.createMessage(message.channelId(), "Gamble Max Guess set to **" + QString::number(config.maxGuess) + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_MIN_GUESS, "setgambleminguess", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.minGuess = args.at(1).toInt();
			client.createMessage(message.channelId(), "Gamble Min Guess set to **" + QString::number(config.minGuess) + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_GAMBLE_REWARD, "setgamblereward", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.gambleReward = args.at(1).toInt();
			client.createMessage(message.channelId(), "Gamble Reward Amount set to **" + QString::number((double)config.gambleReward) + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_BRIBE_SUCCESS_CHANCE, "setbribesuccesschance", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.bribeSuccessChance = args.at(1).toInt();
			client.createMessage(message.channelId(), "Bribe Success Chance set to **" + QString::number(config.bribeSuccessChance) + "%**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_MAX_BRIBE_AMOUNT, "setmaxbribeamount", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.bribeMaxAmount = args.at(1).toInt();
			client.createMessage(message.channelId(), "Bribe Max Amount set to **" + QString::number((double)config.bribeMaxAmount) + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_LEAST_BRIBE_AMOUNT, "setleastbribeamount", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.bribeLeastAmount = args.at(1).toInt();
			client.createMessage(message.channelId(), "Bribe Least Amount set to **" + QString::number((double)config.bribeLeastAmount) + "**");
		});
	});


	RegisterCommand(Commands::EVENT_SET_HRHR_STEAL_SUCCESS_CHANCE, "setHRHRstealsuccesschance", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');
		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			EventModule* eventModule = static_cast<EventModule*>(UmikoBot::Instance().GetModuleByName("event"));
			auto& eventConfig = eventModule->getServerEventData(channel.guildId());
			if (eventConfig.isEventRunning)
			{
				client.createMessage(message.channelId(), "**You cannot set configarations regarding steal while an event is running!** ");
				return;
			}
			config.highRiskRewardStealSuccessChance = args.at(1).toInt();
			client.createMessage(message.channelId(), "HRHR Steal Success Chance set to **" + QString::number(config.highRiskRewardStealSuccessChance) + "%**");
		});
	});

	RegisterCommand(Commands::EVENT_SET_LRLR_STEAL_SUCCESS_CHANCE, "setLRLRstealsuccesschance", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');
		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			EventModule* eventModule = static_cast<EventModule*>(UmikoBot::Instance().GetModuleByName("event"));
			auto& eventConfig = eventModule->getServerEventData(channel.guildId());
			if (eventConfig.isEventRunning)
			{
				client.createMessage(message.channelId(), "**You cannot set configarations regarding steal while an event is running!** ");
				return;
			}
			config.lowRiskRewardStealSuccessChance = args.at(1).toInt();
			client.createMessage(message.channelId(), "LRLR Steal Success Chance set to **" + QString::number(config.lowRiskRewardStealSuccessChance) + "%**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_STEAL_SUCCESS_CHANCE, "setstealsuccesschance", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			EventModule* eventModule = static_cast<EventModule*>(UmikoBot::Instance().GetModuleByName("event"));
			auto& eventConfig = eventModule->getServerEventData(channel.guildId());
			if (eventConfig.isEventRunning)
			{
				client.createMessage(message.channelId(), "**You cannot set configarations regarding steal while an event is running!** ");
				return;
			}
			config.stealSuccessChance = args.at(1).toInt();
			client.createMessage(message.channelId(), "Steal Success Chance set to **" + QString::number(config.stealSuccessChance) + "%**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_STEAL_FINE_PERCENT, "setstealfine", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			EventModule* eventModule = static_cast<EventModule*>(UmikoBot::Instance().GetModuleByName("event"));
			auto& eventConfig = eventModule->getServerEventData(channel.guildId());
			if (eventConfig.isEventRunning)
			{
				client.createMessage(message.channelId(), "**You cannot set configarations regarding steal while an event is running!** ");
				return;
			}
			config.stealFinePercent = args.at(1).toInt();
			client.createMessage(message.channelId(), "Steal Fine Amount set to **" + QString::number(config.stealFinePercent) + "%**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_STEAL_VICTIM_BONUS, "setstealvictimbonus", [this](Client& client, const Message& message, const Channel& channel) 
	{

		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			EventModule* eventModule = static_cast<EventModule*>(UmikoBot::Instance().GetModuleByName("event"));
			auto& eventConfig = eventModule->getServerEventData(channel.guildId());
			if (eventConfig.isEventRunning)
			{
				client.createMessage(message.channelId(), "**You cannot set configarations regarding steal while an event is running!** ");
				return;
			}
			config.stealVictimBonusPercent = args.at(1).toInt();
			client.createMessage(message.channelId(), "Steal Victim Bonus set to **" + QString::number(config.stealVictimBonusPercent) + "%**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_STEAL_JAIL_HOURS, "setstealjailhours", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			EventModule* eventModule = static_cast<EventModule*>(UmikoBot::Instance().GetModuleByName("event"));
			auto& eventConfig = eventModule->getServerEventData(channel.guildId());
			if (eventConfig.isEventRunning)
			{
				client.createMessage(message.channelId(), "**You cannot set configarations regarding steal while an event is running!** ");
				return;
			}
			config.stealFailedJailTime = args.at(1).toInt();
			client.createMessage(message.channelId(), "Steal Jail Time set to **" + QString::number(config.stealFailedJailTime) + " hour(s)**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_DAILY_BONUS_AMOUNT, "setdailybonus", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.dailyBonusAmount = args.at(1).toInt();
			client.createMessage(message.channelId(), "Daily Bonus Amount set to **" + QString::number((double)config.dailyBonusAmount) + "**");
		});
	});

	RegisterCommand(Commands::CURRENCY_SET_DAILY_BONUS_PERIOD, "setdailybonusperiod", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
		{
			auto& config = getServerData(channel.guildId());
			config.dailyBonusPeriod = args.at(1).toInt();
			client.createMessage(message.channelId(), "Daily Bonus will occur every **" + QString::number(config.dailyBonusPeriod) + " days**");
		});
	});

	RegisterCommand(Commands::CURRENCY_COMPENSATE, "compensate", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		//! Doesn't use VerifyAndRunCmd because that currently 
		//! doesn't handle cases where we need at least n arguments.

		::Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::MODERATOR,
			[this, args, &client, message, channel](bool result) 
			{
			
				if (!result) 
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}

				if (args.size() > 3 || args.size() == 1) 
				{
					client.createMessage(channel.id(), "**Wrong Usage of Command!**");
					return;
				}

				QRegExp numReg{ "[+]?\\d*\\.?\\d+" };

				if (args.size() == 2) 
				{
					auto& config = getServerData(channel.guildId());

					if (!numReg.exactMatch(args.at(1))) 
					{
						client.createMessage(channel.id(), "**That's an invalid compensation amount.**");
						return;
					}

					Currency amt = args.at(1).toDouble();

					for (auto& user : guildList[channel.guildId()]) 
					{
						user.setCurrency(user.currency() + amt);
					}

					client.createMessage(message.channelId(), "**Everyone has been compensated with `" + QString::number((double)amt) + config.currencySymbol + "`**\nSorry for any inconvenience!");
					return;
				}

				if (args.size() == 3) 
				{
					auto& config = getServerData(channel.guildId());

					if (!numReg.exactMatch(args.at(2))) {
						client.createMessage(channel.id(), "**That's an invalid compensation amount.**");
						return;
					}

					snowflake_t userId = message.mentions().at(0).id();
					Currency amt = args.at(2).toDouble();

					auto& list = guildList[channel.guildId()];
					auto pos = std::find_if(list.begin(), list.end(),
						[userId, amt](const UserCurrency& usr) 
						{
							return usr.userId == userId;
						});

					if (pos != list.end()) 
					{
						pos->setCurrency(pos->currency() + amt);
					}
					else 
					{
						client.createMessage(message.channelId(), "**Couldn't find the user!**");
						return;
					}

					client.createMessage(message.channelId(), "**" + UmikoBot::Instance().GetName(channel.guildId(), userId) + " has been compensated with `" + QString::number((double)amt) + config.currencySymbol + "`**\nSorry for any inconvenience!");
					return;
				}
			
			});

	});
}
