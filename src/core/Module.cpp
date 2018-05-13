#include "Module.h"

Module::Module(const QString& name, bool enabledByDefault)
	: m_name(name)
	, m_enabledByDefault(enabledByDefault)
{
}

void Module::RegisterCommand(const QString& name, const QString& briefDescription, const QString& fullDescription, Command::Callback callback)
{
	m_commands.push_back({name, briefDescription, fullDescription, callback});
}

void Module::OnMessage(Discord::Client& client, const Discord::Message& message) const
{
	client.getChannel(message.channelId()).then(
		[this, message](const Discord::Channel& channel)
	{
		GuildSetting setting = GuildSettings::GetGuildSetting(channel.guildId());

		if (GuildSettings::IsModuleEnabled(channel.guildId(), m_name, m_enabledByDefault))
		{
			for (const Command& command : m_commands)
			{
				if (message.content().startsWith(setting.prefix + command.name))
				{
					command.callback(message);
				}
			}
		}
	});
}
