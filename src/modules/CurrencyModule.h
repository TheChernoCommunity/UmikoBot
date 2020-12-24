#pragma once
#include <qtimer.h>
#include "core/Module.h"
#include "UmikoBot.h"
#include <random>
#include "core/Currency.h"

class UmikoBot;

class CurrencyModule : public Module
{
	friend class EventModule;
public:
	struct UserCurrency
	{
	private:
		Currency m_Currency;

	public:
		snowflake_t userId;
		bool isDailyClaimed;
		bool isBribeUsed;
		Currency maxCurrency;
		unsigned int dailyStreak;
		QTimer* jailTimer;
		unsigned int numberOfDailysClaimed;
		unsigned int numberOfGiveawaysClaimed;

		UserCurrency(snowflake_t userId, Currency currency, Currency maxCurrency, bool isDailyClaimed, unsigned int dailyStreak, unsigned int numberOfDailysClaimed, unsigned int numberOfGiveawaysClaimed)
			: userId(userId), maxCurrency(maxCurrency), isDailyClaimed(isDailyClaimed), isBribeUsed(false), dailyStreak(dailyStreak), jailTimer(new QTimer()), numberOfDailysClaimed(numberOfDailysClaimed), numberOfGiveawaysClaimed(numberOfGiveawaysClaimed)
		{
			setCurrency(currency);
			jailTimer->setSingleShot(true);
		}

		~UserCurrency()
		{
			delete jailTimer;
		}

		UserCurrency(const UserCurrency& other)
			: userId(other.userId), maxCurrency(other.maxCurrency), isDailyClaimed(other.isDailyClaimed), isBribeUsed(other.isBribeUsed), dailyStreak(other.dailyStreak), jailTimer(new QTimer()), numberOfDailysClaimed(other.numberOfDailysClaimed), numberOfGiveawaysClaimed(other.numberOfGiveawaysClaimed)
		{
			setCurrency(other.currency());
			jailTimer->setSingleShot(true);

			if (other.jailTimer->remainingTime() > 0)
			{
				jailTimer->start(other.jailTimer->remainingTime());
			}
		}

		UserCurrency& operator=(const UserCurrency& other)
		{
			userId = other.userId;
			maxCurrency = other.maxCurrency;
			setCurrency(other.currency());
			isDailyClaimed = other.isDailyClaimed;
			isBribeUsed = other.isBribeUsed;
			dailyStreak = other.dailyStreak;
			numberOfDailysClaimed = other.numberOfDailysClaimed;
			numberOfGiveawaysClaimed = other.numberOfGiveawaysClaimed;

			jailTimer = new QTimer();
			jailTimer->setSingleShot(true);

			if (other.jailTimer->remainingTime() > 0)
			{
				jailTimer->start(other.jailTimer->remainingTime());
			}

			return *this;
		}

		const Currency& currency() const
		{
			return m_Currency;
		}

		void setCurrency(const Currency& value)
		{
			m_Currency = value;

			if (maxCurrency < currency())
				maxCurrency = currency();
		}
	};

	struct CurrencyConfig
	{
		double randGiveawayProb { 0.001 };
		unsigned int freebieExpireTime { 60 };	//in seconds
		Currency dailyReward { 100 };
		Currency freebieReward { 300 };
		Currency gambleReward { 50 };
		int minGuess { 0 };
		int maxGuess { 5 };
		Currency gambleLoss { 10 };
		snowflake_t giveawayChannelId { 0 };
		QString currencyName;
		QString currencySymbol;
		bool isRandomGiveawayDone{ false };
		bool allowGiveaway{ false };
		snowflake_t giveawayClaimer { 0 };
		QTimer* freebieTimer{ nullptr };
		Currency dailyBonusAmount { 50 };
		int dailyBonusPeriod { 3 };
		int stealSuccessChance { 30 };
		int stealFinePercent { 50 };
		int stealVictimBonusPercent { 25 };
		int stealFailedJailTime { 3 };
		Currency bribeMaxAmount { 150 };
		Currency bribeLeastAmount { 20 };
		int bribeSuccessChance { 68 };

		int lowRiskRewardStealSuccessChance{ 50 };
		int highRiskRewardStealSuccessChance{ 30 };
	};

private:
	//! Map server id with user currency list
	QMap<snowflake_t, QList<UserCurrency>> guildList;

	std::random_device random_device;
	std::mt19937 random_engine{ random_device() };
	std::bernoulli_distribution randp;

	QTimer m_timer; //! For dailies, and the giveaway

	QMap<snowflake_t, CurrencyConfig>serverCurrencyConfig;

	struct GambleData {
		bool gamble{ false };
		bool doubleOrNothing{ false };
		snowflake_t userId{ 0 };
		int randNum = 0;
		snowflake_t channelId{ 0 };
		Currency betAmount{ 0 }; //!Use if doubleOrNothing
		QTimer* timer;
	};

	//! Map each !gamble (on a server) with its own gamble
	QMap<snowflake_t, GambleData> gambleData;

	UmikoBot* m_client;

	// Holiday special stuff
	QTimer holidaySpecialCheckTimer; // Checks if the current day is special
	QTimer holidaySpecialTimer; // Either stores time left for people to claim or time until next gift event
	bool isHolidaySpecialActive = false; // Whether today is a special day
	bool isHolidaySpecialClaimable = false;

private:
	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;

	snowflake_t getUserIndex(snowflake_t guild, snowflake_t id) {

		for (auto it = guildList[guild].begin(); it != guildList[guild].end(); ++it) {
			if (it->userId == id) {
				return std::distance(guildList[guild].begin(), it);
			}
		}
		//! If user is not added to the system, make a new one
		guildList[guild].append(UserCurrency{ id, 0, 0, false, 0, 0, 0 });
		return std::distance(guildList[guild].begin(), std::prev(guildList[guild].end()));
	}

public:
	CurrencyModule(UmikoBot* client);
	void StatusCommand(QString& result, snowflake_t guild, snowflake_t user) override;
	void OnMessage(Discord::Client& client, const Discord::Message& message) override;

	UserCurrency& getUserData(snowflake_t guild, snowflake_t id)
	{
		for (auto& user : guildList[guild])
		{
			if (user.userId == id)
			{
				return user;
			}
		}

		//! If user is not added to the system, make a new one
		UserCurrency user{ id, 0, 0, false, 0, 0, 0 };

		guildList[guild].append(user);
		return guildList[guild].back();
	}

	CurrencyConfig& getServerData(snowflake_t guild)
	{
		return serverCurrencyConfig[guild];
	}
};