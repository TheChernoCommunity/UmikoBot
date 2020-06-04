#pragma once
#include <qtimer.h>
#include "core/Module.h"
#include <random>

class UmikoBot;

#define randGiveawayProb 0.001

class CurrencyModule : public Module {
private:
	struct Setting {
		double currency;
		bool isDailyClaimed;
	};

	QMap<snowflake_t, Setting> m_settings;
	std::random_device random_device;
	std::mt19937 random_engine{ random_device() };
	std::bernoulli_distribution randp{ randGiveawayProb };
	QTimer m_timer;
	bool isRandomGiveAwayDone{ false };
	bool allowGiveaway{ false };
	snowflake_t giveawayChannelId{0};
	QString currencyName;
	QString currencySymbol;

	struct selfGamble {
		bool gamble{ false };
		snowflake_t userId{ 0 };
		int randNum = 0;
		snowflake_t channelId{ 0 };
		snowflake_t guildId{ 0 };
	}selfGambleData;

private:
	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;

public:
	CurrencyModule(UmikoBot* client);
	void OnMessage(Discord::Client& client, const Discord::Message& message) override;
};
