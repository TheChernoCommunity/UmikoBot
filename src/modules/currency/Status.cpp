#include "modules/currency/CurrencyModule.h"

using namespace Discord;

void CurrencyModule::initiateStatus()
{
	RegisterCommand(Commands::CURRENCY_WALLET, "wallet", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		if (args.size() > 2) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		snowflake_t user;
		QList<User> mentions = message.mentions();

		if (mentions.size() > 0)
		{
			user = mentions[0].id();
		}
		else
		{
			if (args.size() == 2)
			{
				user = UmikoBot::Instance().GetUserFromArg(channel.guildId(), args, 1);

				if (user == 0)
				{
					client.createMessage(message.channelId(), "**Couldn't find " + args.at(1) + "**");
					return;
				}
			}
			else
			{
				user = message.author().id();
			}
		}

		UmikoBot::Instance().GetAvatar(channel.guildId(), user).then(
			[this, user, channel, &client, message](const QString& icon)
		{
			//! Get User and Sever Data
			auto guild = channel.guildId();
			auto config = getServerData(guild);

			Embed embed;
			embed.setColor(qrand() % 11777216);
			embed.setAuthor(EmbedAuthor(UmikoBot::Instance().GetName(channel.guildId(), user) + "'s Wallet", "", icon));

			QString credits = QString::number((double)getUserData(guild, user).currency());
			QString dailyStreak = QString::number(getUserData(guild, user).dailyStreak);
			QString dailyClaimed = getUserData(guild, user).isDailyClaimed ? "Yes" : "No";

			QString desc = "Current Credits: **" + credits + " " + config.currencySymbol + "** (" + config.currencyName + ")";
			desc += "\n";
			desc += "Daily Streak: **" + dailyStreak + "/" + QString::number(config.dailyBonusPeriod) + "**\n";
			desc += "Today's Daily Claimed? **" + dailyClaimed + "**";
			embed.setDescription(desc);
			client.createMessage(message.channelId(), embed);
		});
	});

	RegisterCommand(Commands::CURRENCY_RICH_LIST, "richlist", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');
		unsigned int min;
		unsigned int max;

		if (args.size() == 1)
		{
			min = 1;
			max = 30;
		}
		else if (args.size() == 2)
		{
			min = 1;
			max = args[1].toUInt();

			if (max == 0)
			{
				client.createMessage(message.channelId(), "**Your argument must be an integer greater than 0**");
				return;
			}
		}
		else if (args.size() == 3)
		{
			min = args[1].toUInt();
			max = args[2].toUInt();

			if (min == 0 || max == 0)
			{
				client.createMessage(message.channelId(), "**Your arguments must be integers greater than 0**");
				return;
			}
		}
		else
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		auto& leaderboard = guildList[channel.guildId()];

		if (min > leaderboard.size())
		{
			client.createMessage(message.channelId(), "**Not enough members to create the list.**");
			return;
		}
		if (max > leaderboard.size())
		{
			max = leaderboard.size();
		}
		if (min > max)
		{
			client.createMessage(message.channelId(), "**The upper bound must be greater than the lower bound.**");
			return;
		}

		qSort(leaderboard.begin(), leaderboard.end(), [](const UserCurrency& u1, const UserCurrency& u2)
		{
			return u1.currency() > u2.currency();
		});

		QString desc;
		Embed embed;
		embed.setTitle("Currency Leaderboard (From " + QString::number(min) + " To " + QString::number(max) + ")");
		int numberOfDigits = QString::number(max).size();

		unsigned int rank = min;

		for (unsigned int i = min; i <= max; i++)
		{
			auto& user = leaderboard[i - 1]; // `i` is one-based, not zero-based
			QString username = UmikoBot::Instance().GetName(channel.guildId(), user.userId);

			if (username.isEmpty())
			{
				max += 1;
				if (max > leaderboard.size())
				{
					max = leaderboard.size();
				}

				continue;
			}

			QString currency = QString::number((double) user.currency());

			desc += "`" + QString::number(rank).rightJustified(numberOfDigits, ' ') + "`) **" + username + "** - ";
			desc += currency + " " + getServerData(channel.guildId()).currencySymbol + "\n";

			rank += 1;
		}

		embed.setColor(qrand() % 16777216);
		embed.setDescription(desc);

		client.createMessage(message.channelId(), embed);
	});
}
