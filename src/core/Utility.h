#pragma once
#include <QtCore/QTimer>
#include <QtCore/QString>
#include <QtCore/QTime>
#include <QtCore/QRegExp>
#include <Discord/Objects/Emoji.h>

namespace utility 
{

	namespace consts 
	{
		constexpr QChar ZERO_WIDTH_SPACE = QChar(0x200B);
		namespace emojis 
		{
			namespace reacts 
			{
				constexpr auto ANGRY_PING        = "anrgyping:769293579251613736";
				constexpr auto PARTY_CAT = "partyCat:748191651830169691";
			}

			constexpr auto GREEN_BLOCK = "<:green_block:769176236101861407>";
			constexpr auto BLACK_BLOCK = "<:black_block:769175573317812254>";
			constexpr auto WE_SMART = "<:wesmart:388340133864407043>";
			constexpr auto AANGER = "<:aanger:730377398314467439>";
		}
	}

	enum class StringMSFormat 
	{
		MINIMAL,
		DESCRIPTIVE,
		MINIMAL_COMMA,
		DESCRIPTIVE_COMMA
	};

	inline QString StringifyMilliseconds(int milliseconds, StringMSFormat fmt = StringMSFormat::DESCRIPTIVE_COMMA) 
	{
		QTime conv{ 0, 0, 0 };
		conv = conv.addMSecs(milliseconds);
		QString hours = QString::number(conv.hour());
		QString mins  = QString::number(conv.minute());
		QString secs  = QString::number(conv.second());
		QString time; 
		
		switch (fmt) 
		{
		case StringMSFormat::DESCRIPTIVE:
			time = hours + "hrs " + mins + "mins " + secs + "secs";
			break;
		case StringMSFormat::DESCRIPTIVE_COMMA:
			time = hours + "hrs, " + mins + "mins, " + secs + "secs";
			break;
		case StringMSFormat::MINIMAL:
			time = hours + "h " + mins + "m " + secs + "s";
			break;
		case StringMSFormat::MINIMAL_COMMA:
			time = hours + "h, " + mins + "m, " + secs + "s";
			break;
		}

		return time;
	}

	inline QString stringifyEmoji(const Discord::Emoji& emoji) 
	{
		QString str;
		str = emoji.name();
		str += (emoji.id() == 0) ? "" : ":" + QString::number(emoji.id());
		return str;
	}
}
