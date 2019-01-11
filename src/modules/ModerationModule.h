#pragma once
#include "core/Module.h"

class ModerationModule : public Module
{
public:
	ModerationModule(Discord::Client* client);

	void OnMessage(Discord::Client& client, const Discord::Channel& channel, const Discord::Message& message) override;

private:
	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;

	struct BannedKeyword
	{
		QString text;
		int deleteMessageDays;
	};

	struct Settings
	{
		Settings() : logChannelId(0) {}

		QList<BannedKeyword> bannedKeywords;
		snowflake_t logChannelId;
	};

	QMap<snowflake_t, Settings> m_settings;
};
