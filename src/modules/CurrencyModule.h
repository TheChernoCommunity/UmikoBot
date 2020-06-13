#pragma once
#include <qtimer.h>
#include "core/Module.h"
#include <random>

class UmikoBot;

class CurrencyModule : public Module 
{
private:
	struct Setting 
	{
		double currency;
		bool isDailyClaimed;
	};

	QMap<snowflake_t, Setting> m_settings;

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
	}config;

	std::random_device random_device;
	std::mt19937 random_engine{ random_device() };
	std::bernoulli_distribution randp{ config.randGiveawayProb };
	QTimer m_timer;
	bool isRandomGiveAwayDone{ false };
	bool allowGiveaway{ false };
	struct SelfGamble {
		bool gamble{ false };
		bool doubleOrNothing{ false };
		snowflake_t userId{ 0 };
		int randNum = 0;
		snowflake_t channelId{ 0 };
		snowflake_t guildId{ 0 };
		double betAmount{ 0 }; //!Use if doubleOrNothing
	}selfGambleData;

private:
	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;

public:
	CurrencyModule(UmikoBot* client);
	void StatusCommand(QString& result, snowflake_t guild, snowflake_t user) override;
	void OnMessage(Discord::Client& client, const Discord::Message& message) override;
};
