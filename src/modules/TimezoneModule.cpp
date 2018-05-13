#include "TimezoneModule.h"

TimezoneModule::TimezoneModule()
	: Module("timezone", true)
{
	RegisterCommand("timeoffset", "brief", "full",
		[this](/*Discord::Client& client, */const Discord::Message& message)
	{
		QStringList arguments = message.content().split(' ');
		if (arguments.count() == 2)
		{
			Setting& setting = m_settings[message.author().id()];
			if (TimeFromString(arguments[1], &setting.time))
			{
				// Send error message
			}

			qDebug() << m_settings[message.author().id()].time.offsetFromUtc();
		}
	});

	RegisterCommand("status", "brief", "full",
		[this](/*Discord::Client& client, */const Discord::Message& message)
	{
		qDebug() << message.author().username() << "has timezone" << m_settings[message.author().id()].time.offsetFromUtc();
	});

	QString t("+56");
	quint32 i = t.toUInt();
	qDebug("%d", i);
}

void TimezoneModule::OnSave(QJsonDocument& doc) const
{
	QJsonObject docObj;

	for (auto it = m_settings.begin(); it != m_settings.end(); ++it)
	{
		QJsonObject obj;
		obj["timezone"] = it->time.toString("hh':'mm");

		docObj[QString::number(it.key())] = obj;
	}

	doc.setObject(docObj);
}

void TimezoneModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject docObj = doc.object();

	for (auto it = docObj.begin(); it != docObj.end(); ++it)
	{
		const QJsonObject obj = it->toObject();
		Setting& setting = m_settings[it.key().toULongLong()];

		TimeFromString(obj["timezone"].toString(), &setting.time);
	}
}

bool TimezoneModule::TimeFromString(const QString& string, QDateTime* outTime) const
{
	QStringList units = string.split(':');
	int offset = 0;
	bool sign = false;
	bool ok = true;

	// Hours and sign
	if (units.count() > 0)
	{
		sign = units[0][0] == '-';
		offset += (units[0].toUInt(&ok) * 3600);
	}

	// Minutes
	if (units.count() > 1)
		offset += (units[1].toUInt(&ok) * 60);

	// Seconds
	if (units.count() > 2)
		offset += (units[2].toUInt(&ok));

	if (ok)
	{
		outTime->setTimeSpec(Qt::UTC);
		outTime->setUtcOffset(sign ? -offset : offset);
		return true;
	}

	return false;
}
