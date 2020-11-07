#pragma once
#include <qtimer.h>
#include "core/Module.h"
#include "UmikoBot.h"
#include <random>

class EventModule : public Module
{
public:
	struct EventConfig
	{
		QTimer* eventTimer { nullptr };
		bool isEventRunning { false };
		bool eventHighRiskHighRewardRunning { false };
		bool eventLowRiskLowRewardRunning { false };
	};
	EventModule(UmikoBot* client);


	void OnMessage(Discord::Client& client, const Discord::Message& message) override;

	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;
	EventConfig& getEventServerData(snowflake_t guild)
	{
		return serverEventConfig[guild];
	}
private:
	QMap<snowflake_t, QList<snowflake_t>> m_EventWhitelist;
	QMap<snowflake_t, EventConfig>serverEventConfig;
};
