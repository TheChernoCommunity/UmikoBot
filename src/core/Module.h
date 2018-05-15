#pragma once
#include <functional>

#include <Discord/Client.h>
#include <QtCore/QString>
#include "GuildSettings.h"

struct Command
{
	using Callback = std::function<void(Discord::Client&, const Discord::Message&, const Discord::Channel&)>;
	
	QString name;
	QString briefDesc;
	QString fullDesc;
	Callback callback;
};

class Module
{
public:
	virtual void OnMessage(Discord::Client& client, const Discord::Message& message) const;
	inline bool IsEnabledByDefault() { return m_enabledByDefault; }

	void Save() const;
	void Load();

protected:
	Module(const QString& name, bool enabledByDefault);

	void RegisterCommand(const QString& name, const QString& briefDescription, const QString& fullDescription, Command::Callback callback);

	virtual void OnSave(QJsonDocument& doc) const { Q_UNUSED(doc) };
	virtual void OnLoad(const QJsonDocument& doc) { Q_UNUSED(doc) };

private:
	QString m_name;
	const bool m_enabledByDefault;
	QList<Command> m_commands;
};
