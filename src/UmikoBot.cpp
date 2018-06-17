#include "UmikoBot.h"
#include "core/Permissions.h"

#include "modules/LevelModule.h"
#include "modules/TimezoneModule.h"
#include "modules/CurrencyModule.h"

UmikoBot::UmikoBot(QObject* parent)
	: Client("umiko-bot", parent)
{
	if (!QDir("configs").exists())
		QDir().mkdir("configs");

	GuildSettings::Load("settings.json");
	
	Load();
	m_modules.push_back(new LevelModule);
	m_modules.push_back(new TimezoneModule);
	m_modules.push_back(new CurrencyModule);

	Q_FOREACH(Module* module, m_modules)
	{
		module->Load();
	}

	m_timer.setInterval(/*60 * 60 */ 1000);
	QObject::connect(&m_timer, &QTimer::timeout, 
		[this]()
	{
		Save();
	});
	m_timer.start();

	connect(this, &Client::onMessageCreate,
		[this](const Discord::Message& message)
	{
		Q_FOREACH(Module* module, m_modules)
		{
			module->OnMessage(*this, message);
		}

		getChannel(message.channelId()).then(
			[this, message](const Discord::Channel& channel)
		{
			GuildSetting setting = GuildSettings::GetGuildSetting(channel.guildId());
			if (channel.guildId() != 0 && !message.author().bot()) // DM
			{
				Q_FOREACH(const Command& command, m_commands)
				{
					if (message.content().startsWith(setting.prefix + command.name))
					{
						command.callback(*this, message, channel);
					}
				}
			}
		});
	
	});

	connect(this, &Client::onGuildCreate,
		[](const Discord::Guild& guild)
	{
		GuildSettings::GetGuildSetting(guild.id()); // only creates if it doesn't exist
	});

	connect(this, &Client::onReady,
		[this]()
	{
		GetGuilds();
	});

	connect(this, &Client::onGuildMemberUpdate,
		[this](snowflake_t guild, const QList<snowflake_t>& roles, const Discord::User& user, const QString& nick)
	{
		if(nick == "")
			m_guildDatas[guild].userdata[user.id()].nickname = user.username();
		else
			m_guildDatas[guild].userdata[user.id()].nickname = nick;
	});

	connect(this, &Client::onGuildRoleUpdate,
		[this](snowflake_t guild_id, const Discord::Role& role)
	{
		for (int i = 0; i < m_guildDatas[guild_id].roles.size(); i++)
			if (role.id() == m_guildDatas[guild_id].roles[i].id()) 
			{
				m_guildDatas[guild_id].roles[i] = role;
				return;
			}
		m_guildDatas[guild_id].roles.push_back(role);
	});

	connect(this, &Client::onGuildRoleDelete,
		[this](snowflake_t guild_id, snowflake_t role_id)
	{
		for (int i = 0; i < m_guildDatas[guild_id].roles.size(); i++)
			if (role_id == m_guildDatas[guild_id].roles[i].id())
			{
				m_guildDatas[guild_id].roles.erase(m_guildDatas[guild_id].roles.begin() + i);
				return;
			}
	});

	connect(this, &Client::onGuildUpdate,
		[this](const Discord::Guild& guild)
	{
		m_guildDatas[guild.id()].ownerId = guild.ownerId();
	});

	m_commands.push_back({Commands::GLOBAL_STATUS, "status",
		[this](Discord::Client& client,const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(" ");
		if (args.size() > 1) 
		{
			if (args.first() != GuildSettings::GetGuildSetting(channel.guildId()).prefix + "status")
				return;
			for (QMap<snowflake_t, UserData>::iterator it = m_guildDatas[channel.guildId()].userdata.begin(); it != m_guildDatas[channel.guildId()].userdata.end(); it++)
			{
				if (it.value().nickname == args.last()) 
				{
					getGuildMember(channel.guildId(), message.author().id()).then(
						[this, message, channel, it](const Discord::GuildMember& member)
					{
						QString status = "";
						Q_FOREACH(Module* module, m_modules)
						{
							module->StatusCommand(status, channel.guildId(), it.key());
						}

						Discord::Embed embed;
						QString icon = "https://cdn.discordapp.com/avatars/" + QString::number(member.user().id()) + "/" + member.user().avatar() + ".png";
						embed.setAuthor(Discord::EmbedAuthor(GetNick(channel.guildId(), it.key()), "", icon));
						embed.setColor(qrand() % 16777216);
						embed.setTitle("Status");
						embed.setDescription(status);

						createMessage(message.channelId(), embed);
					});
				}
			}
		}
		else
			getGuildMember(channel.guildId(), message.author().id()).then(
				[this, message, channel](const Discord::GuildMember& member)
			{
				QString status = "";
				Q_FOREACH(Module* module, m_modules)
				{
					module->StatusCommand(status, channel.guildId(), message.author().id());
				}

				Discord::Embed embed;
				QString icon = "https://cdn.discordapp.com/avatars/" + QString::number(member.user().id()) + "/" + member.user().avatar() + ".png";
				embed.setAuthor(Discord::EmbedAuthor(GetNick(channel.guildId(), message.author().id()), "", icon));
				embed.setColor(qrand() % 16777216);
				embed.setTitle("Status");
				embed.setDescription(status);

				createMessage(message.channelId(), embed);
			});
	}});

	m_commands.push_back({Commands::GLOBAL_HELP, "help",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(" ");
		QString prefix = GuildSettings::GetGuildSetting(channel.guildId()).prefix;
		if (args.size() > 1)
		{
			if (args.first() != prefix + "help" || args.size() != 2)
				return;

			QString commandName = args[1];
			if (commandName.startsWith(prefix))
				commandName = QStringRef(&commandName, prefix.size(), commandName.size() - prefix.size()).toString();

			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help " + commandName);

			QString description = GetCommandHelp(commandName, prefix);
			if (description != "")
				embed.setDescription(description);
			else
				embed.setDescription("Command not found!");

			createMessage(message.channelId(), embed);
		}
		else
		{
			QString description = "";
			[this, &prefix, &description]() {
				auto forCommand =
					[this, &description, &prefix](QList<Command> commands) 
				{
					for (const Command& command : commands)
					{
						QString current = "";
						current = prefix + command.name + " - " + m_commandsInfo[command.id].briefDescription + "\n";
						if (description.length() + current.length() < 1900)
							description += current;
						else
							return true;
					}
					return false;
				};

				if (!forCommand(m_commands))
					Q_FOREACH(Module* module, m_modules)
						if (forCommand(module->GetCommands()))
							return;
			}();
			description += "\n**Note:** Use " + prefix + "help <command> to get the help for a specific command";

			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Help");
			embed.setDescription(description);

			createMessage(message.channelId(), embed);
		}
	}});
}

UmikoBot::~UmikoBot()
{
	Save();

	for (Module* module : m_modules)
		delete module;
}

QString UmikoBot::GetNick(snowflake_t guild, snowflake_t user)
{
	return m_guildDatas[guild].userdata[user].nickname;
}

const QList<Discord::Role>& UmikoBot::GetRoles(snowflake_t guild)
{
	return m_guildDatas[guild].roles;
}

bool UmikoBot::IsOwner(snowflake_t guild, snowflake_t user)
{
	return m_guildDatas[guild].ownerId == user;
}

QString UmikoBot::GetCommandHelp(QString commandName, QString prefix)
{
	QString description = "";
	auto forCommand =
		[this, &description, &prefix, &commandName](QList<Command> commands)
	{
		for (const Command& command : commands)
		{
			if (command.name == commandName)
			{
				CommandInfo& info = m_commandsInfo[command.id];
				description += "**Command name**: " + commandName + "\n\n";
				description += info.briefDescription + "\n\n";

				QStringList usages = info.usage.split("\n");
				description += "**Usage**: \n";
				if (usages.size() == 0)
					description += "\t" + prefix + info.usage + "\n";
				else
					for (const QString& usage : usages)
						description += "\t" + prefix + usage + "\n";

				description += "\n" + info.additionalInfo;
				return true;
			}
		}
		return false;
	};

	if (!forCommand(m_commands))
		Q_FOREACH(Module* module, m_modules)
		if (forCommand(module->GetCommands()))
			break;
	return description;
}

void UmikoBot::Save()
{
	GuildSettings::Save();

	Q_FOREACH(const Module* module, m_modules)
	{
		module->Save();
	}
}

void UmikoBot::Load()
{
#define Command(x) {#x, x}
	using namespace Commands;
	static QMap<QString, unsigned int> commandIds
	{
		Command(GLOBAL_STATUS),
		Command(GLOBAL_HELP),

		Command(LEVEL_MODULE_TOP),
		Command(LEVEL_MODULE_RANK),

		Command(TIMEZONE_MODULE_TIMEOFFSET)
	};

	QFile file("commands.json");
	if (file.open(QIODevice::ReadOnly))
	{
		QByteArray data = file.readAll();

		QJsonDocument doc(QJsonDocument::fromJson(data));
		QJsonObject json = doc.object();
		QStringList commandsIdentifier = json.keys();
		for (const QString& identifier : commandsIdentifier)
		{
			QJsonObject current = json.value(identifier).toObject();
			CommandInfo info;
			info.briefDescription = current["brief"].toString();
			info.usage = current["usage"].toString();
			info.additionalInfo = current["additional"].toString();

			m_commandsInfo[commandIds[identifier]] = info;
		}
	}
	else
	{
		qDebug("%s", "Could not open commands.json");
		// Decide if we should generate them on the fly or something
		// or just terminate the bot
	}
}

void UmikoBot::GetGuilds(snowflake_t after)
{
	auto processGuilds = [this](const QList<Discord::Guild>& guilds)
	{
		for (int i = 0; i < guilds.size(); i++)
		{
			getGuildRoles(guilds[i].id()).then(
				[this, guilds, i](const QList<Discord::Role>& roles)
			{
				m_guildDatas[guilds[i].id()].roles = roles;
				GetGuildMemberInformation(guilds[i].id());
			});

			getGuild(guilds[i].id()).then(
				[this](const Discord::Guild& guild)
			{
				m_guildDatas[guild.id()].ownerId = guild.ownerId();
			});
		}

		if (guilds.size() == 100) //guilds size is equal to the limit
		{
			GetGuilds(guilds[guilds.size() - 1].id());
		}
		qDebug("Guild count: %llu", guilds.size());
	};

	if (after == 0)
		getCurrentUserGuilds().then(processGuilds);
	else
		getCurrentUserGuilds(0, after).then(processGuilds);
}

void UmikoBot::GetGuildMemberInformation(snowflake_t guild, snowflake_t after)
{
	auto processMembers = [this, guild](const QList<Discord::GuildMember>& members)
	{
		for (int i = 0; i < members.size(); i++)
		{
			QString name = members[i].nick();
			if (name == "")
				name = members[i].user().username();
			m_guildDatas[guild].userdata[members[i].user().id()].nickname = name;
		}

		if (members.size() == 1000) //guilds size is equal to the limit
		{
			GetGuildMemberInformation(guild, members[members.size() - 1].user().id());
		}
		qDebug("Guild ID: %llu, Member count: %i", guild, members.size());
	};

	if (after == 0)
		listGuildMembers(guild, 1000).then(processMembers);
	else
		listGuildMembers(guild, after, 1000).then(processMembers);
}
