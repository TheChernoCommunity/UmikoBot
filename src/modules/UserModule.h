#pragma once

#include <qtimer.h>
#include <array>
#include "core/Module.h"

class UmikoBot;

class UserModule : public Module
{
public:
	UserModule();

	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;
	void OnMessage(Discord::Client& client, const Discord::Message& message) override;
	
private:
	struct UserDescription
	{
		snowflake_t userId;
		
		QString name;
		QString location;
		QString industry;
		QString programmingInterests;
		QString currentlyWorkingOn;
		QString githubLink;
	};

	QMap<snowflake_t, QList<UserDescription>> userDescriptions;

	struct DescriptionData
	{
		bool isBeingUsed = false;
		snowflake_t userId = 0;
		QTimer* timer;
		unsigned int questionUpTo = 0;
		
		UserDescription* currentUserDescription;
		UserDescription oldUserDescription;
	};

	QMap<snowflake_t, DescriptionData> guildDescriptionData;

	const std::array<QString, 6> descriptionQuestions = {
		"What is your name?",
		"Where are you from?",
		"What industry do you work in?",
		"What areas of programming are you interested in?",
		"What are you currently working on?",
		"Link to a GitHub profile:"
	};
	
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

	QString formDescriptionMessage(const UserDescription& desc) const
	{
		QString msg;

		if (desc.name != "")
			msg += "**Name: **" + desc.name + "\n";
		
		if (desc.location != "")
			msg += "**Location: **" + desc.location + "\n";
		
		if (desc.industry != "")
			msg += "**Industry: **" + desc.industry + "\n";
		
		if (desc.programmingInterests != "")
			msg += "**Programming Interests: **" + desc.programmingInterests + "\n";
		
		if (desc.currentlyWorkingOn != "")
			msg += "**Currently working on: **" + desc.currentlyWorkingOn + "\n";
		
		if (desc.githubLink != "")
			msg += "**GitHub: **" + desc.githubLink + "\n";

		return msg;
	}
};
