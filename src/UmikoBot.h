#pragma once
#include <Discord/Client.h>

class UmikoBot : public Discord::Client
{
public:
	UmikoBot(QObject* parent = nullptr);

private:
	void onMessageCreate(const Discord::Message& message) override;
};
