#include "Module.h"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>

Module::Module(const QString& name, bool enabledByDefault)
	: m_name(name)
	, m_enabledByDefault(enabledByDefault)
{
}

Module::~Module()
{
	Save();
}

void Module::OnMessage(Discord::Client& client, const Discord::Message& message)
{
	client.getChannel(message.channelId()).then(
		[this, message, &client](const Discord::Channel& channel)
	{
		GuildSetting setting = GuildSettings::GetGuildSetting(channel.guildId());
		if (channel.guildId() != 0 && !message.author().bot()) // DM
			if (GuildSettings::IsModuleEnabled(channel.guildId(), m_name, m_enabledByDefault) && GuildSettings::OutputAllowed(channel.guildId(), channel.id()))
			{
				for (const Command& command : m_commands)
				{
					if (message.content().startsWith(setting.prefix + command.name))
					{
						command.callback(client, message, channel);
					}
				}
			}
	});
}

void Module::RegisterCommand(unsigned int id, const QString& name, Command::Callback callback)
{
	m_commands.push_back({id, name, callback});
}

void Module::Save() const
{
	QFile file("configs/" + m_name + ".json");
	if (file.open(QFile::ReadWrite))
	{
		QJsonDocument doc;
		OnSave(doc);

		file.write(doc.toJson(QJsonDocument::Indented));
		file.close();
	}
}

void Module::Load()
{
	QFile file("configs/" + m_name + ".json");
	if (file.open(QFile::ReadOnly))
	{
		OnLoad(QJsonDocument::fromJson(file.readAll()));
		file.close();
	}
}
