#pragma once

#include <functional>

#include <QString>

#include <Discord/Client.h>

#include "GuildSettings.h"

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

	void RegisterCommand(const QString& name, const QString& briefDescription, const QString& fullDescription, CommandFunction command) 
	{
		m_commands.push_back({ name, briefDescription, fullDescription, command });
	}

public:
	void Parse(Discord::Client& client, const Discord::Message& message) 
	{
		client.getChannel(message.channelId()).then([this, message](const Discord::Channel& channel) {
			GuildSetting setting = GuildSettings::GetGuildSetting(channel.guildId());
			
			for (const Command& command : m_commands)
			{
				if (message.content().startsWith(setting.prefix + command.name)) {
					command.func(message);
				}
			}

		});
	}

	bool IsEnabledByDefault() { return m_enabled; }
};