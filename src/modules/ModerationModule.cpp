#include "ModerationModule.h"
#include "UmikoBot.h"


ModerationModule::ModerationModule()
    : Module("moderation", true)
{
    RegisterCommand(Commands::MODERATION_INVITATION_TOGGLE, "invitations",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
    {
		m_invitationModeration ^= true;
		client.createMessage(message.channelId(), m_invitationModeration ? "Invitations will be deleted!" : "Invitations won't be deleted!");
    });
}

void ModerationModule::OnMessage(Discord::Client& client, const Discord::Message& message)
{
	Module::OnMessage(client, message);
	if (m_invitationModeration)
		if (message.content().startsWith("https://discord.gg"))
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