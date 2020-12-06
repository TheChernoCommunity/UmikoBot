#pragma once

#include "core/Module.h"

class ModerationModule: public Module 
{
public:
	ModerationModule();

	void OnMessage(Discord::Client& client, const Discord::Message& message) override;

	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;
private:
	bool m_invitationModeration = true;
};