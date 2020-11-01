#pragma once

#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <array>
#include <utility>
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

	struct UserSafeMode
	{
		snowflake_t userId = 0;

		bool isIn = false;
		bool HasSafeMode = false;
		QList<int> SafeModeAmts;
		int pricePayed = 0;
	};

	UserSafeMode userSafeModes;

	struct DescriptionData
	{
		bool isBeingUsed = false;
		snowflake_t messageId; // The iam message that started it
		snowflake_t userId = 0;
		QTimer* timer;
		unsigned int questionUpTo = 0;
		
		UserDescription* currentUserDescription;
		UserDescription oldUserDescription;
	};

	QMap<snowflake_t, DescriptionData> guildDescriptionData;
	using questionFunc = void (*)(UserDescription& desc, const QString& value);

#define QUESTION(question, field) qMakePair<QString, questionFunc>(question, [](UserDescription& desc, const QString& value) { desc.field = value; })

	const QList<QPair<QString, questionFunc>> descriptionQuestions = {
		QUESTION("What is your name?", name),
		QUESTION("Where are you from?", location),
		QUESTION("What industry do you work in?", industry),
		QUESTION("What areas of programming are you interested in?", programmingInterests),
		QUESTION("What are you currently working on?", currentlyWorkingOn),
		QUESTION("Link to a GitHub profile:", githubLink),
	};

#undef QUESTION
	
	snowflake_t getUserIndex(snowflake_t guild, snowflake_t id);
	QString formDescriptionMessage(const UserDescription& desc) const;
};
