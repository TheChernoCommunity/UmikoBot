#include "UmikoBot.h"

UmikoBot::UmikoBot(QObject* parent)
	: Client("umiko-bot", parent)
{
	GuildSettings::Load("settings.json");
}

UmikoBot::~UmikoBot()
{
	GuildSettings::Save();
}


void UmikoBot::onMessageCreate(const Discord::Message& message)
{
	qDebug("%s", qPrintable(message.content()));
}

void UmikoBot::onGuildCreate(const Discord::Guild& guild) {
	GuildSettings::AddGuild(guild.id);
}
