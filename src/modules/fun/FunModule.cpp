#include "modules/fun/FunModule.h"

using namespace Discord;

FunModule::FunModule(UmikoBot* client) : Module("funutil", true), m_memeChannel(0), m_client(client)
{
	initiateMeme();
	initiatePoll();
	initiateGithub();
	initiateRoll();
}

void FunModule::OnMessage(Client& client, const Message& message) 
{
	for (auto& usr : message.mentions()) 
	{
		if (usr.bot()) 
		{
			snowflake_t chan = message.channelId();
			UmikoBot::Instance().createReaction(chan, message.id(), utility::consts::emojis::reacts::ANGRY_PING)
				.then([chan]
			{
				UmikoBot::Instance().triggerTypingIndicator(chan);
			});
			return;
		}
	}

	Module::OnMessage(client, message);
}


