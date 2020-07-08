#pragma once
#include <qtimer.h>
#include "core/Module.h"
#include "UmikoBot.h"
#include <random>

class UmikoBot;

class CurrencyModule : public Module 
{
private:
	struct UserCurrency 
	{
		snowflake_t userId;
		double currency;
		bool isDailyClaimed;

	};

	//! Map server id with user currency list
	QMap<snowflake_t, QList<UserCurrency>> guildList;

	struct CurrencyConfig {
		double randGiveawayProb{ 0.001 };
		unsigned int freebieExpireTime{ 60 };	//in seconds
		int dailyReward{ 100 };
		int freebieReward{ 300 };
		int gambleReward{ 50 };
		int minGuess{ 0 };
		int maxGuess{ 5 };
		int gambleLoss{ 10 };
		snowflake_t giveawayChannelId{ 0 };
		QString currencyName;
		QString currencySymbol;
		bool isRandomGiveawayDone{ false };
		bool allowGiveaway{ false };
	};

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
		double betAmount{ 0 }; //!Use if doubleOrNothing
		QTimer* timer;
	};
	
	//! Map each !gamble (on a server) with its own gamble
	QMap<snowflake_t, GambleData> gambleData;

private:
	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;
	
	UserCurrency getUserData(snowflake_t guild, snowflake_t id) 
	{
		for (auto user : guildList[guild]) 
		{
			if (user.userId == id) 
			{
				return user;
			}
		}

		//! If user is not added to the system, make a new one
		UserCurrency user{ id, 0, false };

		guildList[guild].append(user);
		return guildList[guild].back();
	}
	snowflake_t getUserIndex(snowflake_t guild, snowflake_t id) {

		for (auto it = guildList[guild].begin(); it != guildList[guild].end(); ++it) {
			if (it->userId == id) {
				return std::distance(guildList[guild].begin(), it);
			}
		}
		return -1;
	}

	CurrencyConfig& getServerData(snowflake_t guild) 
	{
		return serverCurrencyConfig[guild];
	}

public:
	CurrencyModule(UmikoBot* client);
	void StatusCommand(QString& result, snowflake_t guild, snowflake_t user) override;
	void OnMessage(Discord::Client& client, const Discord::Message& message) override;
};