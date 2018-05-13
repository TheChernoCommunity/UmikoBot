#include "UmikoBot.h"
#include "modules/LevelModule.h"

UmikoBot::UmikoBot(QObject* parent)
	: Client("umiko-bot", parent)
{
	GuildSettings::Load("settings.json");

	m_modules.append(new LevelModule());
}

UmikoBot::~UmikoBot()
{
	Save();

	for (Module* module : m_modules)
		delete module;
}

void UmikoBot::onMessageCreate(const Discord::Message& message)
{
	Q_FOREACH(const Module* module, m_modules)
	{
		module->OnMessage(*this, message);
	}
}

void UmikoBot::onGuildCreate(const Discord::Guild& guild)
{
	GuildSettings::AddGuild(guild.id());
}

void UmikoBot::Save()
{
	GuildSettings::Save();

	Q_FOREACH(const Module* module, m_modules)
	{
		module->Save();
	}
}
