#pragma once
#include <QtCore/QTimer>
#include <QtCore/QString>
#include <QtCore/QTime>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <Discord/Objects/Emoji.h>
#include <array>

namespace utility 
{

	namespace consts 
	{
		constexpr QChar ZERO_WIDTH_SPACE = QChar(0x200B);
		namespace emojis 
		{
			namespace reacts 
			{
				constexpr auto ANGRY_PING = "anrgyping:777298312021672038";
				constexpr auto PARTY_CAT = "partyCat:777298761318793217";

				constexpr auto ARROW_FORWARD  = u8"▶️";
				constexpr auto ARROW_BACKWARD = u8"◀️";
				constexpr auto X_CANCEL = u8"❌";

				constexpr auto REGIONAL_INDICATOR_A = u8"🇦";
				constexpr auto REGIONAL_INDICATOR_B = u8"🇧";
				constexpr auto REGIONAL_INDICATOR_C = u8"🇨";
				constexpr auto REGIONAL_INDICATOR_D = u8"🇩";
				constexpr auto REGIONAL_INDICATOR_E = u8"🇪";
				constexpr auto REGIONAL_INDICATOR_F = u8"🇫";
				constexpr auto REGIONAL_INDICATOR_G = u8"🇬";
				constexpr auto REGIONAL_INDICATOR_H = u8"🇭";
				constexpr auto REGIONAL_INDICATOR_I = u8"🇮";
				constexpr auto REGIONAL_INDICATOR_J = u8"🇯";
				constexpr auto REGIONAL_INDICATOR_K = u8"🇰";
				constexpr auto REGIONAL_INDICATOR_L = u8"🇱";
				constexpr auto REGIONAL_INDICATOR_M = u8"🇲";
				constexpr auto REGIONAL_INDICATOR_N = u8"🇳";
				constexpr auto REGIONAL_INDICATOR_O = u8"🇴";
				constexpr auto REGIONAL_INDICATOR_P = u8"🇵";
				constexpr auto REGIONAL_INDICATOR_Q = u8"🇶";
				constexpr auto REGIONAL_INDICATOR_R = u8"🇷";
				constexpr auto REGIONAL_INDICATOR_S = u8"🇸";
				constexpr auto REGIONAL_INDICATOR_T = u8"🇹";
				constexpr auto REGIONAL_INDICATOR_U = u8"🇺";
				constexpr auto REGIONAL_INDICATOR_V = u8"🇻";
				constexpr auto REGIONAL_INDICATOR_W = u8"🇼";
				constexpr auto REGIONAL_INDICATOR_X = u8"🇽";
				constexpr auto REGIONAL_INDICATOR_Y = u8"🇾";
				constexpr auto REGIONAL_INDICATOR_Z = u8"🇿";
				constexpr std::array<const char*, 26> REGIONAL_INDICATORS =
				{
					REGIONAL_INDICATOR_A,
					REGIONAL_INDICATOR_B,
					REGIONAL_INDICATOR_C,
					REGIONAL_INDICATOR_D,
					REGIONAL_INDICATOR_E,
					REGIONAL_INDICATOR_F,
					REGIONAL_INDICATOR_G,
					REGIONAL_INDICATOR_H,
					REGIONAL_INDICATOR_I,
					REGIONAL_INDICATOR_J,
					REGIONAL_INDICATOR_K,
					REGIONAL_INDICATOR_L,
					REGIONAL_INDICATOR_M,
					REGIONAL_INDICATOR_N,
					REGIONAL_INDICATOR_O,
					REGIONAL_INDICATOR_P,
					REGIONAL_INDICATOR_Q,
					REGIONAL_INDICATOR_R,
					REGIONAL_INDICATOR_S,
					REGIONAL_INDICATOR_T,
					REGIONAL_INDICATOR_U,
					REGIONAL_INDICATOR_V,
					REGIONAL_INDICATOR_W,
					REGIONAL_INDICATOR_X,
					REGIONAL_INDICATOR_Y,
					REGIONAL_INDICATOR_Z,
				};
			}
			
			constexpr auto GREEN_BLOCK = "<:green_block:777298339858612255>";
			constexpr auto BLACK_BLOCK = "<:black_block:777298360267833345>";
			constexpr auto WE_SMART = "<:wesmart:777299810658680892>";
			constexpr auto AANGER = "<:aanger:777298666485841921>";
			constexpr auto SHOPPING_BAGS = ":shopping_bags:";
		}
	}

	enum class StringMSFormat 
	{
		DESCRIPTIVE,
		DESCRIPTIVE_COMMA,
		MINIMAL,
		MINIMAL_COMMA
	};

	inline QString StringifyMilliseconds(qint64 milliseconds, StringMSFormat fmt = StringMSFormat::DESCRIPTIVE_COMMA) 
	{	
		QDateTime conv;
		QDateTime start;
		start.setOffsetFromUtc(0);
		start.setMSecsSinceEpoch(0);

		conv.setOffsetFromUtc(0);
		conv.setMSecsSinceEpoch(milliseconds);

		QStringList values;

#define get(x) QString::number(conv.toString(#x).toInt() - start.toString(#x).toInt())
		values.push_back(get(yyyy));	//! years
		values.push_back(get(M));		//! months
		values.push_back(get(d));		//! days
		values.push_back(get(h));		//! hours
		values.push_back(get(m));		//! mins
		values.push_back(get(s));		//! seconds
#undef get
		
		const QStringList descriptiveUnits = {"years", "months", "days", "hrs", "mins", "secs"};
		const QStringList minimalUnits = { "y", "M", "d", "h", "m", "s" };
		QString sep = " ";

		switch (fmt) 
		{
		case StringMSFormat::DESCRIPTIVE_COMMA:
		case StringMSFormat::MINIMAL_COMMA:
			sep = ", ";
		}

		int off = 0;

		for (const auto& value : values) 
		{
			if (value != "0") break;
			off++;
		}

		QString time;

		switch (fmt) 
		{
		case StringMSFormat::MINIMAL:
		case StringMSFormat::MINIMAL_COMMA:
			for (int i = off; i < values.size(); i++) 
			{
				time += values.at(i) + minimalUnits.at(i) + ((i == values.size()-1) ? "" : sep);
			}
			break;
		case StringMSFormat::DESCRIPTIVE:
		case StringMSFormat::DESCRIPTIVE_COMMA:
			for (int i = off; i < values.size(); i++) 
			{
				time += values.at(i) + descriptiveUnits.at(i) + ((i == values.size() - 1) ? "" : sep);
			}
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
