#include "CurrencyModule.h"

CurrencyModule::CurrencyModule()
	: Module("currency", false)
{
}

void CurrencyModule::OnMessage(Discord::Client& client, const Discord::Message& message)
{
	m_settings[message.author().id()].currency += 5 + qrand() % 5;

	Module::OnMessage(client, message);
}

void CurrencyModule::StatusCommand(QString& result, snowflake_t guild, snowflake_t user)
{
	result += "$ " + QString::number(m_settings[user].currency) + "\n";
	result += "\n";
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
		const QJsonObject obj = it.value().toObject();
		Setting& setting = m_settings[it.key().toULongLong()];
		setting.currency = obj["currency"].toInt();
	}
}
