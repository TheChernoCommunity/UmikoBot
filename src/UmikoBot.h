#pragma once
#include <Discord/Client.h>

#include "core/GuildSettings.h"

class UmikoBot : public Discord::Client
{
public:
	UmikoBot(QObject* parent = nullptr);
	~UmikoBot();

private:
	void onMessageCreate(const Discord::Message& message) override;
};
