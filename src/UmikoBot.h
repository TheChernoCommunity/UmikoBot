#pragma once
#include <Discord/Client.h>
#include "core/GuildSettings.h"
#include "core/Module.h"

namespace Commands {
	enum {
		GLOBAL_STATUS,
		GLOBAL_HELP,
		GLOBAL_SET_PREFIX,
		GLOBAL_MODULE,
		GLOBAL_OUTPUT,

		LEVEL_MODULE_TOP,
		LEVEL_MODULE_RANK,
		LEVEL_MODULE_MAX_LEVEL,
		LEVEL_MODULE_EXP_REQUIREMENT,
		LEVEL_MODULE_EXP_GROWTH_RATE,
		LEVEL_MODULE_EXP_GIVE,
		LEVEL_MODULE_EXP_TAKE,
		LEVEL_MODULE_BLOCK_EXP,

		TIMEZONE_MODULE_TIMEOFFSET,

		MODERATION_INVITATION_TOGGLE,

		CURRENCY_WALLET,
		CURRENCY_DAILY,
		CURRENCY_GAMBLE,
		CURRENCY_CLAIM,
		CURRENCY_SET_PRIZE_CHANNEL,
		CURRENCY_SET_NAME,
		CURRENCY_SET_SYMBOL,
		CURRENCY_SET_DAILY,
		CURRENCY_SET_PRIZE,
		CURRENCY_SET_GAMBLE_LOSS,
		CURRENCY_SET_GAMBLE_REWARD,
		CURRENCY_SET_GAMBLE_MIN_GUESS,
		CURRENCY_SET_GAMBLE_MAX_GUESS,
		CURRENCY_SET_PRIZE_PROB,
		CURRENCY_SET_PRIZE_EXPIRY,
		CURRENCY_RICH_LIST,
		CURRENCY_DONATE,

		UFUN_MEME
	};
}

struct UserData {
	QString nickname;
	QString username;
};

struct GuildData {
	QMap<snowflake_t, UserData> userdata;
	QList<Discord::Role> roles;
	snowflake_t ownerId;
};

struct CommandInfo {
	QString briefDescription;
	QString usage;
	QString additionalInfo;
	bool adminPermission;
};

class UmikoBot : public Discord::Client 
{
public:
	static UmikoBot& Instance();
	UmikoBot(const UmikoBot&) = delete;
	void operator=(const UmikoBot&) = delete;
	~UmikoBot();

	QString GetNick(snowflake_t guild, snowflake_t user);
	QString GetUsername(snowflake_t guild, snowflake_t user);
	QString GetName(snowflake_t guild, snowflake_t user);

	snowflake_t GetUserFromArg(snowflake_t guild, QStringList args, int startIndex);

	const QList<Discord::Role>& GetRoles(snowflake_t guild);
	bool IsOwner(snowflake_t guild, snowflake_t user);

	QString GetCommandHelp(QString commandName, QString prefix);
	QList<Command> GetAllCommands();

private slots:
	void OnDisconnected();

private:
	UmikoBot(QObject* parent = nullptr);

	void Save();
	void Load();
	void GetGuilds(snowflake_t after = 0);
	void GetGuildMemberInformation(snowflake_t guild, snowflake_t after = 0);

	QList<Module*> m_modules;
	QTimer m_timer;

	QMap<snowflake_t, GuildData> m_guildDatas;
	QList<Command> m_commands;
	QMap<unsigned int, CommandInfo> m_commandsInfo;
};
