#include "modules/fun/FunModule.h"

#define EMBED_BAR_MAX_WIDTH 15.0

using namespace Discord;

FunModule::PollSettings::PollSettings(const PollOptions& op, long long maxvotes, snowflake_t chan, const QString& name, int num, double time, const ServerPolls& pollList, snowflake_t msg) 
	: options(op), maxVotes(maxvotes), notifChannel(chan), 
	pollName(name), pollNum(num), polls(pollList), pollMsg(msg) 
{
	timer = std::make_shared<QTimer>();
	timer->setInterval(time * 1000);
	timer->setSingleShot(true);
	timer->start();

	QObject::connect(timer.get(), &QTimer::timeout,
	[this]() 
	{
		auto poll_num = this->pollNum;
		auto serverPolls = this->polls;
		//! Create a message with the results
		auto settings = serverPolls->at(this->pollNum);
		auto poll_name = settings->pollName;
		auto notif_chan = settings->notifChannel;
		auto poll_msg = settings->pollMsg;

		std::size_t total = 0;
		for (auto& option : settings->options) 
		{
			total += option.count;
		}

		if (total == 0) total = 1;

		QList<EmbedField> fields;

		for (auto& option : settings->options) 
		{
			double val = static_cast<double>(option.count) / static_cast<double>(total) * EMBED_BAR_MAX_WIDTH;
			double percentage = static_cast<double>(option.count) / static_cast<double>(total) * 100.0;
			int num = qFloor(val);

			QString str{ utility::consts::ZERO_WIDTH_SPACE };

			for (int i = 0; i < num; i++) 
			{
				str += utility::consts::emojis::GREEN_BLOCK;
			}

			for (int i = 0; i < EMBED_BAR_MAX_WIDTH - num; i++) 
			{
				str += utility::consts::emojis::BLACK_BLOCK;
			}

			EmbedField field;
			QRegExp reg{ ".+:\\d+" };

			QString desc = "";
			QString emoji = option.emote;

			if (option.desc != "") 
			{
				desc = " (" + option.desc + ") ";
			}

			if (reg.exactMatch(option.emote)) 
			{
				if (option.isAnimated) 
				{
					emoji.prepend("<a:");
				}
				else emoji.prepend("<:");
				emoji.append(">");
			}
			field.setName(emoji + desc + ": " + QString::number(percentage) + "%");
			field.setValue(str);

			fields.push_back(field);
		}

		Embed embed;
		embed.setColor(qrand() % 16777216);
		embed.setTitle("Results for Poll#" + QString::number(poll_num) + " " + poll_name);
		embed.setFields(fields);


		UmikoBot::Instance().createMessage(notif_chan, embed).then([poll_num, serverPolls](const Message& message) 
		{

			auto settings = serverPolls->at(poll_num);
			auto poll_name = settings->pollName;
			auto notif_chan = settings->notifChannel;
			auto poll_msg = settings->pollMsg;

			//! Get the id of the resulting message

			auto resultMsg = message.id();

			//! This part is used to get the guild id

			UmikoBot::Instance().getChannel(notif_chan).then([poll_num, resultMsg, serverPolls](const Channel& chan) 
			{

				auto settings = serverPolls->at(poll_num);
				auto poll_name = settings->pollName;
				auto notif_chan = settings->notifChannel;
				auto poll_msg = settings->pollMsg;

				MessagePatch patch;

				Embed embed;
				embed.setColor(qrand() % 16777216);
				embed.setTitle("Poll#" + QString::number(poll_num) + " " + poll_name);
				embed.setDescription("This poll has ended. See the results [here.](https://discordapp.com/channels/"
					+ QString::number(chan.guildId()) + "/"
					+ QString::number(notif_chan) + "/"
					+ QString::number(resultMsg) + ")");

				patch.setEmbed(embed);
				UmikoBot::Instance().editMessage(notif_chan, poll_msg, patch);
				UmikoBot::Instance().deleteAllReactions(chan.id(), poll_msg);

				//! Remove the poll from our pending list
				bool removedOne = false;
				for (int i = 0; i < serverPolls->size(); i++) 
				{
					if (serverPolls->at(i)->pollNum == poll_num) 
					{
						serverPolls->removeAt(i);
						removedOne = true;
						if (serverPolls->empty()) break;
					}
					if (removedOne) 
					{
						serverPolls->at(i)->pollNum--;
					}
				}
				
			});
		});
	});
}
