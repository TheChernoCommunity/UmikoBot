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
	Q_FOREACH(const Module& module, m_modules)
	{
		module.OnMessage(*this, message);
	}
}

void UmikoBot::onGuildCreate(const Discord::Guild& guild)
{
	GuildSettings::AddGuild(guild.id());
}
