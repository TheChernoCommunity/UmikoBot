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
