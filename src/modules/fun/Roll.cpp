#include "modules/fun/FunModule.h"
#include <random>

using namespace Discord;

void FunModule::initiateRoll()
{
	RegisterCommand(Commands::FUN_ROLL, "roll", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		if (args.size() < 2 || args.size() > 3)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}
		if (args.size() == 2)
		{
			double min = 0;
			double max = args.at(1).toDouble();
			QRegExp re("[+-]?\\d*\\.?\\d+");
			if (!re.exactMatch(args.at(1)))
			{
				client.createMessage(message.channelId(), "**You must roll with numbers!**");
				return;
			}
			if (max > 2147483647 || max < -2147483647)
			{
				client.createMessage(message.channelId(), "**You can't roll that number!**");
				return;
			}
			std::random_device rand_device;
			std::mt19937 gen(rand_device());

			if (max < min)
				std::swap(min, max);

			std::uniform_int_distribution<> dist(min, max);

			QString text = QString("My Value was: **" + QString::number(dist(gen)) + "**");
			client.createMessage(message.channelId(), text);
			return;
		}

		if (args.size() == 3)
		{
			double min = args.at(1).toDouble();
			double max = args.at(2).toDouble();
			QRegExp re("[+-]?\\d*\\.?\\d+");
			if (!re.exactMatch(args.at(1)) || !re.exactMatch(args.at(2)))
			{
				client.createMessage(message.channelId(), "**You must roll with numbers!**");
				return;
			}
			if (max > 2147483647 || min > 2147483647 || max < -2147483647 || min < -2147483647)
			{
				client.createMessage(message.channelId(), "**You can't roll that number!**");
				return;
			}
			if (args.at(1) == args.at(2))
			{
				client.createMessage(message.channelId(), "My Value was: **" + args.at(1) + "**");
				return;
			}

			std::random_device rand_device;
			std::mt19937 gen(rand_device());

			if (max < min)
				std::swap(min, max);

			std::uniform_int_distribution<> dist(min, max);

			QString text = QString("My Value was: **" + QString::number(dist(gen)) + "**");
			client.createMessage(message.channelId(), text);
		}
	});
}
