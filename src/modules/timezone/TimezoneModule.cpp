#include "modules/timezone/TimezoneModule.h"
#include "UmikoBot.h"

using namespace Discord;

TimezoneModule::TimezoneModule()
	: Module("timezone", true)
{
	RegisterCommand(Commands::TIMEZONE_MODULE_TIMEOFFSET, "timeoffset",
		[this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList arguments = message.content().split(' ');
		if (arguments.count() == 2)
		{
			Setting& setting = m_settings[message.author().id()];
			auto offsetResult = UtcOffsetFromString(arguments[1]);
			if (offsetResult.second)
			{
				setting.secondsFromUtc = offsetResult.first;
				client.createMessage(message.channelId(), "Timezone set to " + StringFromUtcOffset(setting.secondsFromUtc));
			}
			else
			{
				client.createMessage(message.channelId(), "Invalid time format");
			}

			qDebug() << m_settings[message.author().id()].secondsFromUtc;
		}
	});
}

void TimezoneModule::StatusCommand(QString& result, snowflake_t guild, snowflake_t user)
{
	const int secondsFromUtc = m_settings[user].secondsFromUtc;
	result += "Time offset: " + StringFromUtcOffset(secondsFromUtc) + "\n";
	result += "Current time: " + QDateTime::currentDateTimeUtc().addSecs(secondsFromUtc).toString("hh':'mm") + "\n";
	result += "\n";
}

void TimezoneModule::OnSave(QJsonDocument& doc) const
{
	QJsonObject docObj;

	for (auto it = m_settings.begin(); it != m_settings.end(); ++it)
	{
		QJsonObject obj;
		obj["timezone"] = it->secondsFromUtc;

		docObj[QString::number(it.key())] = obj;
	}

	doc.setObject(docObj);
}

void TimezoneModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject docObj = doc.object();

	for (auto it = docObj.begin(); it != docObj.end(); ++it)
	{
		const QJsonObject obj = it.value().toObject();
		Setting& setting = m_settings[it.key().toULongLong()];

		setting.secondsFromUtc = obj["timezone"].toInt();
	}
}

QPair<int, bool> TimezoneModule::UtcOffsetFromString(const QString& string)
{
	auto clamp = [](int v, int mini, int maxi) { return (v < mini) ? mini : (v > maxi) ? maxi : v; };

	QStringList units = string.split(':');
	switch (units.size())
	{
		case 1:
		{
			bool ok;
			int h = units[0].toInt(&ok);
			if (!ok)
				return qMakePair(0, false);
			h = clamp(h, -23, 23);

			return qMakePair(h * 3600, true);
		}

		case 2:
		{
			bool ok;
			int h = units[0].toInt(&ok);
			if (!ok)
				return qMakePair(0, false);
			h = clamp(h, -23, 23);

			int m = units[1].toInt(&ok);
			if (!ok || m < 0)
				return qMakePair(h * 3600, true);
			m = clamp(m, 0, 59);
			if (h < 0)
				m *= -1;

			return qMakePair(h * 3600 + m * 60, true);
		}

		default:
			return qMakePair(0, false);
	}
}

QString TimezoneModule::StringFromUtcOffset(int offset)
{
	if (offset < 0)
	{
		offset *= -1;
		return QString::asprintf("UTC-%02d:%02d", offset / 3600, (offset % 3600) / 60);
	}
	else
	{
		return QString::asprintf("UTC+%02d:%02d", offset / 3600, (offset % 3600) / 60);
	}
}
