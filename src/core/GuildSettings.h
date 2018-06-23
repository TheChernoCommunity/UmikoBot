#pragma once

#include <Discord/Client.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFile>
#include <QDir>

struct LevelRank 
{
	QString name;
	unsigned int minimumLevel;
};

struct GuildSetting 
{
	snowflake_t id;

	QString prefix;
	QList<QPair<QString, bool>> modules;
	
	// Level system related
	QList<LevelRank> ranks;
	unsigned int maximumLevel;
	unsigned int expRequirement;
	float growthRate;
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

	static bool IsModuleEnabled(snowflake_t guild, const QString& moduleName, bool isDefault = true);
	static void ToggleModule(snowflake_t guild, const QString& moduleName, bool enabled, bool isDefault = true);
private:
	static GuildSetting CreateGuildSetting(snowflake_t id);
};
