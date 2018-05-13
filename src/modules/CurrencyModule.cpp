#include "CurrencyModule.h"

CurrencyModule::CurrencyModule()
	: Module("currency", true)
{
	RegisterCommand("$", "brief", "full",
		[this](Discord::Client& client, const Discord::Message& message)
	{
		const Setting& setting = m_settings[message.author().id()];

		Discord::Embed embed;
		embed.setDescription("$ " + QString::number(setting.currency));

		client.createMessage(message.channelId(), embed);
	});
}

void CurrencyModule::OnMessage(Discord::Client& client, const Discord::Message& message)
{
	m_settings[message.author().id()].currency += 5 + qrand() % 5;

	Module::OnMessage(client, message);
}

void CurrencyModule::OnSave(QJsonDocument& doc) const
{
	QJsonObject docObj;

	for (auto it = m_settings.begin(); it != m_settings.end(); ++it)
	{
		QJsonObject obj;
		obj["currency"] = it->currency;

		docObj[QString::number(it.key())] = obj;
	}

	doc.setObject(docObj);
}

void CurrencyModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject docObj = doc.object();

	for (auto it = docObj.begin(); it != docObj.end(); ++it)
	{
		const QJsonObject obj = it->toObject();
		Setting& setting = m_settings[it.key().toULongLong()];
		setting.currency = obj["currency"].toInt();
	}
}
