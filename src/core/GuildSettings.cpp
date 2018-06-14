#include "GuildSettings.h"
#include "modules/LevelModule.h"

QList<GuildSetting> GuildSettings::s_settings;
QString GuildSettings::s_location;

void GuildSettings::Load(const QString& location)
{
	s_location = "configs/" + location;

	qDebug("%s", qPrintable(s_location));

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
			QJsonArray owners = current["owners"].toArray();

			for (const QJsonValue& owner : owners)
			{
				setting.owners.push_back(owner.toString().toULongLong());
			}

			if (current.contains("prefix"))
			{
				setting.prefix = current["prefix"].toString();
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

			if (current.contains("levelmodule"))
			{
				QJsonObject levelModuleJson = current["levelmodule"].toObject();
				if (levelModuleJson.contains("maximumLevel"))
					setting.maximumLevel = levelModuleJson["maximumLevel"].toString().toUInt();

				QJsonObject ranksJson = levelModuleJson["ranks"].toObject();
				QStringList ranks = ranksJson.keys();
				for (const QString& rankName : ranks)
					setting.ranks.push_back({ rankName, ranksJson[rankName].toString().toUInt() });
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
		QJsonArray owners;
		for (snowflake_t owner : setting.owners)
		{
			owners.append(QString::number(owner));
		}
		current["owners"] = owners;
		if(setting.prefix != "!")
			current["prefix"] = setting.prefix;

		if (setting.modules.size() > 0) 
		{
			QJsonObject moduleSettings;
			bool hasModuleSettings = false;
			for (const QPair<QString, bool>& module : setting.modules)
			{
				moduleSettings[module.first] = module.second;
			}

			current["modules"] = moduleSettings;

			
		}
		bool levelModuleDefault = true;
		QJsonObject levelModule;
		if (setting.ranks.size() > 0)
		{
			levelModuleDefault = false;
			QJsonObject ranks;

			for (const LevelRank& rank : setting.ranks)
				ranks[rank.name] = QString::number(rank.minimumLevel);
			
			levelModule["ranks"] = ranks;
		}

		if (setting.maximumLevel != LEVELMODULE_MAXIMUM_LEVEL)
		{
			levelModuleDefault = false;
			levelModule["maxmimumLevel"] = QString::number(setting.maximumLevel);
		}

		if(!levelModuleDefault)
			current["levelModule"] = levelModule;

		json[QString::number(setting.id)] = current;
	}

	QJsonDocument doc(json);

	file.write(doc.toJson(QJsonDocument::Indented));
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


bool GuildSettings::IsOwner(snowflake_t guild, snowflake_t id) 
{
	const GuildSetting& setting = GetGuildSetting(guild);
	for (snowflake_t owner : setting.owners)
		if (owner == id)
			return true;
	return false;
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

GuildSetting GuildSettings::CreateGuildSetting(snowflake_t id)
{
	GuildSetting s;
	s.id = id;
	s.maximumLevel = LEVELMODULE_MAXIMUM_LEVEL;
	s.prefix = "!";
	return s;
}
