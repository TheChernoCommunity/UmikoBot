#pragma once
#include <Discord/Client.h>
#include "core/GuildSettings.h"
#include "core/Module.h"

namespace Commands {
	enum {
		GLOBAL_STATUS,
		GLOBAL_HELP,

		LEVEL_MODULE_TOP,

		TIMEZONE_MODULE_TIMEOFFSET,
	};
}

struct CommandInfo {
	QString briefDescription;
	QString usage;
	QString additionalInfo;
};

class UmikoBot : public Discord::Client
{
public:
	UmikoBot(QObject* parent = nullptr);
	~UmikoBot();

	QString GetNick(snowflake_t guild, snowflake_t user);

private:
	void Save();
	void Load();
	void GetGuilds(snowflake_t after = 0);
	void GetGuildsMemberCount(snowflake_t guild, snowflake_t after = 0);

	QList<Module*> m_modules;
	QTimer m_timer;

	QMap<snowflake_t, QMap<snowflake_t, QString>> m_nicknames;
	QList<Command> m_commands;
	QMap<unsigned int, CommandInfo> m_commandsInfo;
};
