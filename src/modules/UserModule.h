#pragma once

#include "core/Module.h"

class UmikoBot;

class UserModule : public Module
{
public:
	UserModule();

	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;

private:
	struct UserDescription
	{
		snowflake_t userId;
		QString description;
	};

	QMap<snowflake_t, QList<UserDescription>> userDescriptions;

	snowflake_t getUserIndex(snowflake_t guild, snowflake_t id)
	{
		QList<UserDescription>& guildDescriptions = userDescriptions[guild];

		for (auto it = guildDescriptions.begin(); it != guildDescriptions.end(); ++it)
		{
			if (it->userId == id)
			{
				return std::distance(guildDescriptions.begin(), it);
			}
		}

		// If user is not added to the system, make a new one
		guildDescriptions.append(UserDescription { id, "" });
		return std::distance(guildDescriptions.begin(), std::prev(guildDescriptions.end()));
	}
};
