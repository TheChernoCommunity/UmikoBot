#pragma once
#include "core/Module.h"

class CurrencyModule : public Module
{
public:
	CurrencyModule();

	void OnMessage(Discord::Client& client, const Discord::Message& message) override;

	void StatusCommand(QString& result, snowflake_t guild, snowflake_t user) override;

private:
	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;

	struct Setting
	{
		int currency;
	};

	QMap<snowflake_t, Setting> m_settings;
};
