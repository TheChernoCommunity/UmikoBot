#pragma once

#include <functional>

#include <QString>

#include <Discord/Client.h>

typedef std::function<void(const Discord::Message&)> CommandFunction;

struct Command {
	QString name;
	QString briefDesc;
	QString fullDesc;
	CommandFunction func;
};

class Module {
protected:
	QString m_name;
	bool m_enabled;

	Module(const QString& name, bool enabled) 
		: m_name(name), m_enabled(enabled)
	{ }

	QList<Command> m_commands;

	void RegisterCommand(const QString& name, const QString& briefDescription, const QString& fullDescription, CommandFunction command) {
		m_commands.push_back({ name, briefDescription, fullDescription, command });
	}

public:
	void Parse(const Discord::Message& message) {

	}

	bool IsEnabledByDefault() { return m_enabled; }
};