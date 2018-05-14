#pragma once

#include <functional>

#include <QMap>
#include <QTimer>

#include "core/Module.h"

class LevelModule : public Module
{
public:
	LevelModule();

	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;

	void OnMessage(Discord::Client& client, const Discord::Message& message) const override;

private:
	struct GuildLevelData {
		snowflake_t    user;
		int            exp;
		int            messageCount;
	};

	GuildLevelData GetData(snowflake_t guild, snowflake_t user);

	mutable QMap<snowflake_t, QList<GuildLevelData>> m_exp;
	QTimer m_timer;
};
