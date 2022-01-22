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
	snowflake_t primaryChannel;
	QList<QPair<QString, bool>> modules;
	
	// Level system related
	QList<LevelRank> ranks;
	unsigned int maximumLevel;
	unsigned int expRequirement;
	float growthRate;
	QList<snowflake_t> levelWhitelistedChannels;
	QList<snowflake_t> levelBlacklistedChannels;

	QList<snowflake_t> outputWhitelistedChannels;
	QList<snowflake_t> outputBlacklistedChannels;
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

	static bool OutputAllowed(snowflake_t guild, snowflake_t channel);
	static bool ExpAllowed(snowflake_t guild, snowflake_t channel);
private:
	static GuildSetting CreateGuildSetting(snowflake_t id);
};
