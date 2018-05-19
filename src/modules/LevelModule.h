#pragma once

#include <functional>

#include <QMap>
#include <QTimer>

#include "core/Module.h"

class LevelModule : public Module
{
public:
	LevelModule(Discord::Client& client);

	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;

	void OnMessage(Discord::Client& client, const Discord::Message& message) override;

private:
	struct GuildLevelData {
		snowflake_t    user;
		int            exp;
		int            messageCount;
	};

	GuildLevelData GetData(snowflake_t guild, snowflake_t user);

	void GetGuilds(Discord::Client& client, snowflake_t after = 0);
	void GetGuildsMemberCount(Discord::Client& client, snowflake_t guild, snowflake_t after = 0);

	mutable QMap<snowflake_t, QList<GuildLevelData>> m_exp;
	QTimer m_timer;

	QMap<snowflake_t, QMap<snowflake_t, QString>> m_nicknames;
};
