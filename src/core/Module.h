#pragma once
#include <functional>

#include <Discord/Client.h>
#include <QtCore/QString>
#include "GuildSettings.h"

struct Command
{
	using Callback = std::function<void(const Discord::Message&)>;

	QString name;
	QString briefDesc;
	QString fullDesc;
	Callback callback;
};

class Module
{
public:
	void OnMessage(Discord::Client& client, const Discord::Message& message) const;
	inline bool IsEnabledByDefault() { return m_enabledByDefault; }

protected:
	Module(const QString& name, bool enabledByDefault);

	void RegisterCommand(const QString& name, const QString& briefDescription, const QString& fullDescription, Command::Callback callback);

private:
	QString m_name;
	const bool m_enabledByDefault;
	QList<Command> m_commands;
};
