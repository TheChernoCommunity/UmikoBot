#include "GuildSettings.h"

QList<GuildSetting> GuildSettings::s_settings;
QString GuildSettings::s_location;

void GuildSettings::Load(const QString& location)
{
	s_location = QDir::currentPath() + "/" + location;

	qDebug("%s", qPrintable(s_location));

	QFile file(s_location);
	file.open(QIODevice::ReadOnly);

	QByteArray data = file.readAll();

	QJsonDocument doc(QJsonDocument::fromJson(data));
	QJsonObject json = doc.object();
	QStringList guildIds = json.keys();

	for (const QString& id : guildIds) 
	{
		QJsonObject current = json.value(id).toObject();
		GuildSetting setting;
		setting.id = id.toULongLong();
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

		s_settings.push_back(setting);
	}

	file.close();
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

	s_settings.append(GuildSetting());
	s_settings[s_settings.size() - 1].id = id;
	return s_settings[s_settings.size() - 1];
}

void GuildSettings::AddGuild(snowflake_t id)
{
	s_settings.push_back({ id });
}


bool GuildSettings::IsOwner(snowflake_t guild, snowflake_t id) {
	const GuildSetting& setting = GetGuildSetting(guild);
	for (snowflake_t owner : setting.owners)
		if (owner == id)
			return true;
	return false;
}

bool GuildSettings::IsModuleEnabled(snowflake_t guild, const QString& moduleName, bool default)
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

	return default;
}

void GuildSettings::ToggleModule(snowflake_t guild, const QString& moduleName, bool enabled, bool default) {
	GuildSetting& setting = GetGuildSetting(guild);
	
	QList<QPair<QString, bool>>& modules = setting.modules;

	for (size_t i = 0; i < modules.size(); i++)
	{
		if (modules[i].first == moduleName)
			if (enabled == default)
				modules.removeAt(i);
	}
	if (enabled != default)
		modules.append({ moduleName, enabled });

}
