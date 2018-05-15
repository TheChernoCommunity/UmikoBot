#include "UmikoBot.h"
#include "modules/LevelModule.h"

#include "modules/TimezoneModule.h"
#include "modules/CurrencyModule.h"

UmikoBot::UmikoBot(QObject* parent)
	: Client("umiko-bot", parent)
{
	if (!QDir("configs").exists())
		QDir().mkdir("configs");

	GuildSettings::Load("settings.json");

	m_modules.append(new LevelModule());
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
	});

	connect(this, &Client::onGuildCreate,
		[](const Discord::Guild& guild)
	{
		GuildSettings::AddGuild(guild.id());
	});
}

UmikoBot::~UmikoBot()
{
	Save();

	for (Module* module : m_modules)
		delete module;
}

void UmikoBot::Save()
{
	GuildSettings::Save();

	Q_FOREACH(const Module* module, m_modules)
	{
		module->Save();
	}
}
