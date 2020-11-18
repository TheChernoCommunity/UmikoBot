#pragma once
#include <qtimer.h>
#include "core/Module.h"
#include "UmikoBot.h"

class EventModule : public Module
{
public:
	struct RaffleDraw
	{
	public:
		snowflake_t m_UserId;
		QList<unsigned int> m_TicketIds;
		RaffleDraw(snowflake_t userID)
			:m_UserId(userID){}
	};

	struct EventConfig
	{
		QTimer* eventTimer{ nullptr };
		QTimer* raffleDrawRewardClaimTimer{ nullptr };
		bool isEventRunning{ false };
		bool eventHighRiskHighRewardRunning{ false };
		bool eventLowRiskLowRewardRunning{ false };
		bool claimedReward = true;
		bool eventRaffleDrawRunning{ false };
		unsigned int currentTicketIndex{ 0 };
		int raffleDrawTicketPrice{ 50 };
		int maxTicketOfUser{ 20 };
		snowflake_t luckyUser;
	};
	EventModule(UmikoBot* client);
	void OnMessage(Discord::Client& client, const Discord::Message& message) override;

	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;
private:
	QMap<snowflake_t, QList<RaffleDraw>> raffleDrawGuildList;
	QMap<snowflake_t, QList<snowflake_t>> m_EventWhitelist;
	QMap<snowflake_t, EventConfig>serverEventConfig;

	snowflake_t raffleDrawGetUserIndex(snowflake_t guild, snowflake_t id)
	{
		for (auto it = raffleDrawGuildList[guild].begin(); it != raffleDrawGuildList[guild].end(); ++it)
		{
			if (it->m_UserId == id)
			{
				return std::distance(raffleDrawGuildList[guild].begin(), it);
			}
		}
		raffleDrawGuildList[guild].append(RaffleDraw{ id });
		return std::distance(raffleDrawGuildList[guild].begin(), std::prev(raffleDrawGuildList[guild].end()));
	}
public:
	RaffleDraw getUserRaffleDrawData(snowflake_t guild, snowflake_t id)
	{
		for (auto user : raffleDrawGuildList[guild])
		{
			if (user.m_UserId == id)
			{
				return user;
			}
		}
		RaffleDraw user{ id };
		raffleDrawGuildList[guild].append(user);
		return raffleDrawGuildList[guild].back();
	}
	EventConfig& getServerEventData(snowflake_t guild)
	{
		return serverEventConfig[guild];
	}
};
