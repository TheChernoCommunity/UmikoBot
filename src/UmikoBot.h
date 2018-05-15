#pragma once
#include <Discord/Client.h>
#include "core/GuildSettings.h"
#include "core/Module.h"

class UmikoBot : public Discord::Client
{
public:
	UmikoBot(QObject* parent = nullptr);
	~UmikoBot();

private:
	void onMessageCreate(const Discord::Message& message) override;
	void onGuildCreate(const Discord::Guild& guild) override;

	void Save();

	QList<Module*> m_modules;
	QTimer m_timer;
};
