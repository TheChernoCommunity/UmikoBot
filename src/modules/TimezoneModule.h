#pragma once
#include <QtCore/QTimeZone>
#include "core/Module.h"

class TimezoneModule : public Module
{
public:
	TimezoneModule();

	void StatusCommand(QString& result, snowflake_t guild, snowflake_t user) override;

private:
	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;

	static QPair<int, bool> UtcOffsetFromString(const QString& string);
	static QString StringFromUtcOffset(int offset);

	struct Setting
	{
		int secondsFromUtc;
	};

	QMap<snowflake_t, Setting> m_settings;
};
