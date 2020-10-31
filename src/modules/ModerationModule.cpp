#include "ModerationModule.h"
#include "UmikoBot.h"

using namespace Discord;

ModerationModule::ModerationModule()
    : Module("moderation", true)
{
    RegisterCommand(Commands::MODERATION_INVITATION_TOGGLE, "invitations",
		[this](Client& client, const Message& message, const Channel& channel)
    {
		m_invitationModeration ^= true;
		client.createMessage(message.channelId(), m_invitationModeration ? "Invitations will be deleted!" : "Invitations won't be deleted!");
    });
}

void ModerationModule::OnMessage(Client& client, const Message& message)
{
	Module::OnMessage(client, message);
	if (m_invitationModeration)
		if (message.content().contains("https://discord.gg", Qt::CaseInsensitive))
			client.deleteMessage(message.channelId(), message.id());
}


void ModerationModule::OnSave(QJsonDocument& doc) const 
{
	QJsonObject json;
 	QJsonObject moderation;

	moderation["invitationModeration"] = m_invitationModeration;
	json["moderation"] = moderation;

	doc.setObject(moderation);
}

void ModerationModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject json = doc.object();
	QJsonObject moderation = json["moderation"].toObject();
	m_invitationModeration = moderation["moderation"].toBool();
}
