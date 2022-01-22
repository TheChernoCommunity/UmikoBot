#include "GuildSettings.h"
#include "modules/LevelModule.h"

#include "Logger.h"

QList<GuildSetting> GuildSettings::s_settings;
QString GuildSettings::s_location;

void GuildSettings::Load(const QString& location)
{
	s_location = "configs/" + location;

	ULog(ulog::Severity::Debug, UFString("Configuration file found, loading from: %s", qPrintable(s_location)));

	QFile file(s_location);
	if (file.open(QIODevice::ReadOnly))
	{
		QByteArray data = file.readAll();

		QJsonDocument doc(QJsonDocument::fromJson(data));
		QJsonObject json = doc.object();
		QStringList guildIds = json.keys();

		for (const QString& id : guildIds)
		{
			QJsonObject current = json.value(id).toObject();
			GuildSetting setting = CreateGuildSetting(id.toULongLong());
			
			if (current.contains("prefix"))
			{
				setting.prefix = current["prefix"].toString();
			}

			if (current.contains("primaryChannel"))
			{
				setting.primaryChannel = current["primaryChannel"].toString().toULongLong();
			}

			if (current.contains("modules"))
			{
				QJsonObject moduleJson = current["modules"].toObject();
				QStringList modules = moduleJson.keys();

				for (const QString& moduleName : modules)
				{
					setting.modules.push_back({ moduleName, moduleJson[moduleName].toBool() });
				}
			}

			if (current.contains("levelModule"))
			{
				QJsonObject levelModuleJson = current["levelModule"].toObject();
				if (levelModuleJson.contains("maximumLevel"))
					setting.maximumLevel = levelModuleJson["maximumLevel"].toString().toUInt();

				if (levelModuleJson.contains("growthRate"))
					setting.growthRate = levelModuleJson["growthRate"].toString().toFloat();

				if (levelModuleJson.contains("expRequirement"))
					setting.expRequirement = levelModuleJson["expRequirement"].toString().toUInt();

				QJsonObject ranksJson = levelModuleJson["ranks"].toObject();
				QStringList ranks = ranksJson.keys();
				for (const QString& rankName : ranks)
					setting.ranks.push_back({ rankName, ranksJson[rankName].toString().toUInt() });

				qSort(setting.ranks.begin(), setting.ranks.end(),
					[](const LevelRank& v1, const LevelRank& v2) -> bool
				{
					return v1.minimumLevel < v2.minimumLevel;
				});
			}

			if (current.contains("behaviourModule"))
			{
				QJsonObject behaviourModuleJson = current["behaviourModule"].toObject();

				if (behaviourModuleJson.contains("levelBehaviourChannels"))
				{
					QJsonObject levelBehaviourChannelsJson = behaviourModuleJson["levelBehaviourChannels"].toObject();
					QStringList levelBehaviourChannels = levelBehaviourChannelsJson.keys();
					for (const QString& channel : levelBehaviourChannels)
						if (levelBehaviourChannelsJson[channel].toBool() == true)
							setting.levelWhitelistedChannels.push_back(channel.toULongLong());
						
						else 
							setting.levelBlacklistedChannels.push_back(channel.toULongLong());
				}

				if (behaviourModuleJson.contains("outputBehaviourChannels"))
				{
					QJsonObject outputBehaviourChannelsJson = behaviourModuleJson["outputBehaviourChannels"].toObject();
					QStringList outputBehaviourChannels = outputBehaviourChannelsJson.keys();
					for (const QString& channel : outputBehaviourChannels)
						if (outputBehaviourChannelsJson[channel].toBool() == true)
							setting.outputWhitelistedChannels.push_back(channel.toULongLong());
						else
							setting.outputBlacklistedChannels.push_back(channel.toULongLong());
				}
			}

			s_settings.push_back(setting);
		}

		file.close();
	}
}

void GuildSettings::Save()
{
	QFile file(s_location);
	file.open(QIODevice::WriteOnly);

	QJsonObject json;

	for (const GuildSetting& setting : s_settings) 
	{
		QJsonObject current;
		if(setting.prefix != "!")
			current["prefix"] = setting.prefix;

		if (setting.primaryChannel != 0)
			current["primaryChannel"] = QString::number(setting.primaryChannel);
		
		if (setting.modules.size() > 0) 
		{
			QJsonObject moduleSettings;
			bool hasModuleSettings = false;
			for (const QPair<QString, bool>& module : setting.modules)
				moduleSettings[module.first] = module.second;

			current["modules"] = moduleSettings;
		}

		bool levelModuleDefault = true;
		QJsonObject levelModule;
		QJsonObject ranks;
		if (setting.ranks.size() > 0)
		{
			levelModuleDefault = false;

			for (const LevelRank& rank : setting.ranks)
				ranks[rank.name] = QString::number(rank.minimumLevel);
			
			levelModule["ranks"] = ranks;
		}

		if (setting.maximumLevel != LEVELMODULE_MAXIMUM_LEVEL)
		{
			levelModuleDefault = false;
			levelModule["maximumLevel"] = QString::number(setting.maximumLevel);
		}
		
		if (setting.growthRate != LEVELMODULE_EXP_GROWTH)
		{
			levelModuleDefault = false;
			levelModule["growthRate"] = QString::number(setting.growthRate);
		}

		if (setting.expRequirement != LEVELMODULE_EXP_REQUIREMENT)
		{
			levelModuleDefault = false;
			levelModule["expRequirement"] = QString::number(setting.expRequirement);
		}

		if(!levelModuleDefault)
			current["levelModule"] = levelModule;

		bool behaviourModuleDefault = true;
		QJsonObject behaviourModule;
		if (setting.levelWhitelistedChannels.size() > 0 || setting.levelBlacklistedChannels.size() > 0 || setting.outputBlacklistedChannels.size() > 0 || setting.outputWhitelistedChannels.size() > 0) 
		{
			QJsonObject levelBehaviourChannels;
			QJsonObject outputBehaviourChannels;
			
			for (snowflake_t channel : setting.levelWhitelistedChannels)
				levelBehaviourChannels[QString::number(channel)] = true;

			for (snowflake_t channel : setting.levelBlacklistedChannels)
				levelBehaviourChannels[QString::number(channel)] = false;

			for (snowflake_t channel : setting.outputWhitelistedChannels)
				outputBehaviourChannels[QString::number(channel)] = true;

			for (snowflake_t channel : setting.outputBlacklistedChannels)
				outputBehaviourChannels[QString::number(channel)] = false;

			behaviourModule["levelBehaviourChannels"] = levelBehaviourChannels;
			behaviourModule["outputBehaviourChannels"] = outputBehaviourChannels;
			behaviourModuleDefault = false;
		}

		if (!behaviourModuleDefault)
			current["behaviourModule"] = behaviourModule;

		json[QString::number(setting.id)] = current;
	}

	QJsonDocument doc(json);

	QString result = doc.toJson(QJsonDocument::Indented);
	file.write(qPrintable(result));
	file.close();
}

GuildSetting& GuildSettings::GetGuildSetting(snowflake_t id)
{
	for (GuildSetting& setting : s_settings) 
	{
		if (setting.id == id)
			return setting;
	}

	s_settings.append(CreateGuildSetting(id));
	return s_settings[s_settings.size() - 1];
}

void GuildSettings::AddGuild(snowflake_t id)
{
	s_settings.push_back(CreateGuildSetting(id));
}

bool GuildSettings::IsModuleEnabled(snowflake_t guild, const QString& moduleName, bool isDefault)
{
	GuildSetting& setting = GetGuildSetting(guild);
	for (const QPair<QString, bool>& module : setting.modules)
	{
		if (module.first == moduleName)
			return module.second;
	}

	// Not adding the actual module to the list of existing modules means that it's already
	// using the default enabled setting, thus the only modules who are going to be stored 
	// in the module list of the guild settings are only going to be modules which don't have
	// their usual default enabled setting

	return isDefault;
}

void GuildSettings::ToggleModule(snowflake_t guild, const QString& moduleName, bool enabled, bool isDefault)
{
	GuildSetting& setting = GetGuildSetting(guild);
	
	QList<QPair<QString, bool>>& modules = setting.modules;

	for (int i = 0; i < modules.size(); i++)
	{
		if (modules[i].first == moduleName)
			if (enabled == isDefault)
				modules.removeAt(i);
	}
	if (enabled != isDefault)
		modules.append({ moduleName, enabled });

}

bool GuildSettings::OutputAllowed(snowflake_t guild, snowflake_t channel)
{
	GuildSetting s = GuildSettings::GetGuildSetting(guild);
	if (s.outputWhitelistedChannels.size() > 0) {
		for (int i = 0; i < s.outputWhitelistedChannels.size(); i++)
			if (s.outputWhitelistedChannels[i] == channel)
				return true;
		return false;
	}
	if (s.outputBlacklistedChannels.size() > 0)
		for (int i = 0; i < s.outputBlacklistedChannels.size(); i++)
			if (s.outputBlacklistedChannels[i] == channel)
				return false;
	return true;
}

bool GuildSettings::ExpAllowed(snowflake_t guild, snowflake_t channel)
{
	GuildSetting s = GuildSettings::GetGuildSetting(guild);
	if (s.levelWhitelistedChannels.size() > 0) {
		for (int i = 0; i < s.levelWhitelistedChannels.size(); i++)
			if (s.levelWhitelistedChannels[i] == channel)
				return true;
		return false;
	}
	if (s.levelBlacklistedChannels.size() > 0)
		for (int i = 0; i < s.levelBlacklistedChannels.size(); i++)
			if (s.levelBlacklistedChannels[i] == channel)
				return false;
	return true;
}


GuildSetting GuildSettings::CreateGuildSetting(snowflake_t id)
{
	GuildSetting s;
	s.id               = id;
	s.maximumLevel     = LEVELMODULE_MAXIMUM_LEVEL;
	s.expRequirement   = LEVELMODULE_EXP_REQUIREMENT;
	s.growthRate       = LEVELMODULE_EXP_GROWTH;
	s.prefix           = "!";
	s.primaryChannel   = 0;
	return s;
}
