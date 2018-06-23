#pragma once
#include <functional>

#include <Discord/Client.h>
#include <QtCore/QString>
#include "GuildSettings.h"

struct Command
{
	using Callback = std::function<void(Discord::Client&, const Discord::Message&, const Discord::Channel&)>;
	
	unsigned int id;
	QString name;
	Callback callback;
};

class Module
{
public:
	virtual void OnMessage(Discord::Client& client, const Discord::Message& message);
	inline bool IsEnabledByDefault() const { return m_enabledByDefault; }

	void Save() const;
	void Load();

	virtual void StatusCommand(QString& result, snowflake_t guild, snowflake_t user) {}

	QList<Command>& GetCommands() const { return m_commands; }

	QString GetName() const { return m_name; }

protected:
	Module(const QString& name, bool enabledByDefault);

	void RegisterCommand(unsigned int id, const QString& name, Command::Callback callback);

	virtual void OnSave(QJsonDocument& doc) const { Q_UNUSED(doc) };
	virtual void OnLoad(const QJsonDocument& doc) { Q_UNUSED(doc) };

private:
	QString m_name;
	const bool m_enabledByDefault;
	QList<Command> m_commands;
};
