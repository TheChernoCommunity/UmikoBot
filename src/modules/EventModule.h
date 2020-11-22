#pragma once
#include <qtimer.h>
#include "core/Module.h"
#include "UmikoBot.h"

//Used for the events (Needed in the EventModule.cpp and CurrencyModule.cpp)
#define highRiskRewardBonus 50 // In percentage
#define lowRiskRewardPenalty 40 //In percentage

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
		bool isHighRiskHighRewardRunning{ false };
		bool isLowRiskLowRewardRunning{ false };
		bool claimedReward = true;
		bool eventRaffleDrawRunning{ false };
		unsigned int numTicketsBought{ 0 };
		int raffleDrawTicketPrice{ 50 };
		int maxUserTickets{ 20 };
		snowflake_t luckyUser;
	};
	EventModule(UmikoBot* client);
	void OnMessage(Discord::Client& client, const Discord::Message& message) override;

	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;
private:
	QList<QString> eventNamesAndCodes = {"HRHR", "LRLR", "RaffleDraw"};
	QMap<snowflake_t, QList<RaffleDraw>> raffleDrawGuildList;
	QMap<snowflake_t, QList<snowflake_t>> m_EventWhitelist;
	QMap<snowflake_t, EventConfig>serverEventConfig;
	
	int raffleDrawGetUserIndex(snowflake_t guild, snowflake_t id)
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
	void EndEvent(const snowflake_t& channelID, const snowflake_t& guildID, const snowflake_t& authorID, bool isInQObjectConnect, QString eventNameOrCode);
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
