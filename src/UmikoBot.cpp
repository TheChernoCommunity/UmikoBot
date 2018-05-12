#include "UmikoBot.h"

UmikoBot::UmikoBot(QObject* parent)
	: Client("umiko-bot", parent)
{
}

void UmikoBot::onMessageCreate(const Discord::Message& message)
{
	qDebug("%s", qPrintable(message.content()));
}
