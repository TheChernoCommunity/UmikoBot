#pragma once
// Fun and Utilities!
#include "core/Module.h"
#include "UmikoBot.h"

class FunModule : public Module, public QObject 
{
private:
	QNetworkAccessManager m_MemeManager;
	snowflake_t m_memeChannel;
public:
	FunModule();
};