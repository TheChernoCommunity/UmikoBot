#pragma once

#include <Discord/Client.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFile>
#include <QDir>

struct GuildSetting 
{
	snowflake_t id;
	QList<snowflake_t> owners;

	QString prefix = "!";
	QList<QPair<QString, bool>> modules;
};

class GuildSettings 
{
	static QList<GuildSetting> s_settings;
	static QString s_location;

public:
	static void Load(const QString& location);
	static void Save();

	static GuildSetting& GetGuildSetting(snowflake_t id);

	static void AddGuild(snowflake_t id);

	static bool IsOwner(snowflake_t guild, snowflake_t id);

	static bool IsModuleEnabled(snowflake_t guild, const QString& moduleName, bool default = true);
	static void ToggleModule(snowflake_t guild, const QString& moduleName, bool enabled, bool default = true);
};
