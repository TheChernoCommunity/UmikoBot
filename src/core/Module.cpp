#include "Module.h"
#include "core/Permissions.h"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>

Module::Module(const QString& name, bool enabledByDefault)
	: m_name(name)
	, m_enabledByDefault(enabledByDefault)
{
}

Module::~Module()
{
	
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
	if (file.open(QFile::ReadWrite | QFile::Truncate))
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

void Module::AddAdminCommand(Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel, unsigned int requiredNumberOfArgs, const QStringList& args, bool argumentShouldBeANumber, std::function<void()> callback)
{
	Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN, [this, args, &client, message, channel, requiredNumberOfArgs, argumentShouldBeANumber, callback](bool result) 
	{
		if (!result) 
		{
			client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
			return;
		}
			
		if (args.size() != requiredNumberOfArgs) 
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() > 1 && argumentShouldBeANumber)
		{
			bool ok;
			// Should be fine to check all numbers agains double, right?
			args.at(1).toDouble(&ok);

			if (!ok)
			{
				client.createMessage(message.channelId(), "**Argument is not a number!**");
				return;
			}
		}

		callback();
	});
}
