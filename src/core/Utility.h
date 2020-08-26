#pragma once
#include <QTCore/QTimer>
#include <QtCore/QString>
#include <QTCore/QTime>

QString StringifyMilliseconds(int milliseconds)
{
	QTime conv { 0, 0, 0 };
	conv = conv.addMSecs(milliseconds);
	QString time = QString::number(conv.hour()) + "hrs, " + QString::number(conv.minute()) + "mins, " + QString::number(conv.second()) + "secs";

	return time;
}
