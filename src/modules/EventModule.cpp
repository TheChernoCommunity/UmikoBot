#include "UmikoBot.h"
#include "EventModule.h"
#include "core/Permissions.h"
#include "core/Utility.h"
#include "CurrencyModule.h"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/qregexp.h>
#include <QtCore/QtMath>

#include <random>

#define eventConfigLoc QString("eventConfig")

EventModule::EventModule(UmikoBot* client) : Module("event", true), m_client(client)
{
	RegisterCommand(Commands::EVENT, "event", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		auto& config = getEventServerData(channel.guildId());
		CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
		auto& currencyConfig = currencyModule->getServerData(channel.guildId());

		if (args.size() != 1)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}
		if (config.isEventRunning == false)
		{
			client.createMessage(message.channelId(), "**No event is active at this moment.**");
			return;
		}
		if (config.eventHighRiskHighRewardRunning)
		{
			int num = config.eventTimer->remainingTime();
			QString time = utility::StringifyMilliseconds(num);

			Discord::Embed embed;
			embed.setColor(15844367);
			embed.setTitle("**HighRiskHighReward** event");
			embed.setDescription("The steal chance is **decreased** but if you succeed you will get **bonus " + currencyConfig.currencySymbol + "**!\n"
				"Event **expire's** in `" + time + "`");

			client.createMessage(message.channelId(), embed);
			return;

		}
		if (config.eventLowRiskLowRewardRunning)
		{
			int num = config.eventTimer->remainingTime();
			QString time = utility::StringifyMilliseconds(num);

			Discord::Embed embed;
			embed.setColor(15844367);
			embed.setTitle("**LowRiskLowReward** event");
			embed.setDescription("The steal chance is **increased** but you have to give a **penalty** if you succeed!\n"
				"Event **expire's** in `" + time + "`");

			client.createMessage(message.channelId(), embed);
			return;
		}

	});
	RegisterCommand(Commands::EVENT_LAUNCH, "launch", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result)
			{

				auto& config = getEventServerData(channel.guildId());
				CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
				auto& currencyConfig = currencyModule->getServerData(channel.guildId());

				if (!result)
				{
					client.createMessage(message.channelId(), "**You don't have permissions launch events.**");
					return;
				}
				if (args.size() != 3)
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}

				if (config.isEventRunning)
				{
					client.createMessage(message.channelId(), "An event is already running. Please stop that to start a new event!");
					return;
				}

				double time = args.at(2).toDouble();

				QRegExp re("[+]?\\d*\\.?\\d+");
				if (!re.exactMatch(args.at(2)))
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!**.");
					return;
				}
				if (time > 24)
				{
					client.createMessage(message.channelId(), "**Currently you can't launch events with an expiry time of more than one day**.");
					return;
				}
				if (config.eventTimer != nullptr) //Delete the previous timer
				{
					delete config.eventTimer;
					config.eventTimer = nullptr;
				}
				config.eventTimer = new QTimer;

				config.eventTimer->setInterval(time * 3600000);
				config.eventTimer->setSingleShot(true);

				if (args.at(1) == "HighRiskHighReward")
				{
					QString num = QString::number(time);

					Discord::Embed embed;
					embed.setColor(15844367);
					embed.setTitle("Hey everyone! **HighRiskHighReward** event is launched for `" + num + "` hour(s)!");
					embed.setDescription("Give command `!event` to see what event is running and it's changes!");

					config.isEventRunning = true;
					config.eventHighRiskHighRewardRunning = true;
					currencyConfig.stealSuccessChance -= highRiskRewardStealDecrease;
					client.createMessage(message.channelId(), embed);
				}
				if (args.at(1) == "LowRiskLowReward")
				{
					QString num = QString::number(time);

					Discord::Embed embed;
					embed.setColor(15844367);
					embed.setTitle("Hey everyone! **LowRiskLowReward** event is launched for `" + num + "` hour(s)!");
					embed.setDescription("Give command `!event` to see what event is running and it's changes!");

					config.isEventRunning = true;
					config.eventLowRiskLowRewardRunning = true;
					currencyConfig.stealSuccessChance += lowRiskRewardStealIncrease;
					client.createMessage(message.channelId(), embed);
				}
				auto guildID = channel.guildId();
				auto chan = message.channelId();

				QObject::connect(config.eventTimer, &QTimer::timeout, [this, &client, guildID, chan]()
					{
						CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
						auto& currencyConfig = currencyModule->getServerData(guildID);

						auto& config = getEventServerData(guildID);
						if (config.eventHighRiskHighRewardRunning)
						{
							Discord::Embed embed;
							embed.setColor(15158332);
							embed.setTitle("Hey everyone! **HighRiskHighReward** event has **ended**!");

							config.isEventRunning = false;
							config.eventHighRiskHighRewardRunning = false;
							currencyConfig.stealSuccessChance += highRiskRewardStealDecrease;
							client.createMessage(chan, embed);
						}
						else if (config.eventLowRiskLowRewardRunning)
						{
							Discord::Embed embed;
							embed.setColor(15158332);
							embed.setTitle("Hey everyone! **LowRiskLowReward** event has **ended**!");

							config.isEventRunning = false;
							config.eventLowRiskLowRewardRunning = false;
							currencyConfig.stealSuccessChance -= lowRiskRewardStealIncrease;
							client.createMessage(chan, embed);
						}
					});
				config.eventTimer->start();
			});
	});
	RegisterCommand(Commands::EVENT_END, "endevent", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');

		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result)
			{
				auto& config = getEventServerData(channel.guildId());
				CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
				auto& currencyConfig = currencyModule->getServerData(channel.guildId());

				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (!result)
				{
					client.createMessage(message.channelId(), "**You don't have permissions end events.**");
					return;
				}
				if (args.size() != 1)
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}
				if (config.isEventRunning == false)
				{
					client.createMessage(message.channelId(), "**No event is running, so you can't end an event!**\n");
					return;
				}
				if (config.eventTimer->isActive())
				{
					if (config.eventHighRiskHighRewardRunning)
					{
						Discord::Embed embed;
						embed.setColor(15158332);
						embed.setTitle("Hey everyone! **HighRiskHighReward** event has **ended**!");

						config.isEventRunning = false;
						config.eventHighRiskHighRewardRunning = false;
						config.eventTimer->stop();
						currencyConfig.stealSuccessChance += highRiskRewardStealDecrease;
						client.createMessage(message.channelId(), embed);
						return;
					}
					if (config.eventLowRiskLowRewardRunning)
					{
						Discord::Embed embed;
						embed.setColor(15158332);
						embed.setTitle("Hey everyone! **LowRiskLowReward** event has **ended**!");

						config.isEventRunning = false;
						config.eventLowRiskLowRewardRunning = false;
						config.eventTimer->stop();
						currencyConfig.stealSuccessChance -= lowRiskRewardStealIncrease;
						client.createMessage(message.channelId(), embed);
					}
				}
			});
	});
	RegisterCommand(Commands::RESTORE, "restore", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
			[this, args, &client, message, channel](bool result)
			{
				auto& config = getEventServerData(channel.guildId());
				CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
				auto& currencyConfig = currencyModule->getServerData(channel.guildId());
				if (!result)
				{
					client.createMessage(message.channelId(), "**You don't have permissions to use this command.**");
					return;
				}
				if (args.size() != 1)
				{
					client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}
				if (config.isEventRunning == false)
				{
					client.createMessage(message.channelId(), "Nothing to restore.");
				}
				if (config.isEventRunning)
				{
					if (config.eventHighRiskHighRewardRunning)
					{
						config.isEventRunning = false;
						config.eventHighRiskHighRewardRunning = false;
						currencyConfig.stealSuccessChance += highRiskRewardStealDecrease;
					}
					if (config.eventLowRiskLowRewardRunning)
					{
						config.isEventRunning = false;
						config.eventLowRiskLowRewardRunning = false;
						currencyConfig.stealSuccessChance -= lowRiskRewardStealIncrease;
					}
					client.createMessage(message.channelId(), "**Restored!**");
				}
			});
	});
}

void EventModule::OnMessage(Discord::Client& client, const Discord::Message& message)
{
	Module::OnMessage(client, message);
}

void EventModule::OnSave(QJsonDocument& doc) const
{
	QJsonObject docObj;
	//! Server Data (Config)
	QFile eventConfigfile("configs/" + eventConfigLoc + ".json");
	if (eventConfigfile.open(QFile::ReadWrite | QFile::Truncate))
	{
		QJsonDocument doc;
		QJsonObject serverList;
		for (auto server : serverEventConfig.keys())
		{
			QJsonObject obj;
			auto config = serverEventConfig[server];
			obj["isEventRunning"] = config.isEventRunning;
			obj["eventHighRiskHighRewardRunning"] = config.eventHighRiskHighRewardRunning;
			obj["eventLowRiskLowRewardRunning"] = config.eventLowRiskLowRewardRunning;
			serverList[QString::number(server)] = obj;
		}
		doc.setObject(serverList);
		eventConfigfile.write(doc.toJson());
		eventConfigfile.close();
	}
}

void EventModule::OnLoad(const QJsonDocument& doc)
{
	QFile eventConfigfile("configs/" + eventConfigLoc + ".json");
	if (eventConfigfile.open(QFile::ReadOnly))
	{
		QJsonDocument d = QJsonDocument::fromJson(eventConfigfile.readAll());
		QJsonObject rootObj = d.object();

		serverEventConfig.clear();
		auto servers = rootObj.keys();
		EventConfig config;

		for (const auto& server : servers)
		{
			auto serverObj = rootObj[server].toObject();
			config.isEventRunning = serverObj["isEventRunning"].toBool();
			config.eventHighRiskHighRewardRunning = serverObj["eventHighRiskHighRewardRunning"].toBool();
			config.eventLowRiskLowRewardRunning = serverObj["eventLowRiskLowRewardRunning"].toBool();
			auto guildId = server.toULongLong();

			serverEventConfig.insert(guildId, config);
		}
		eventConfigfile.close();
	}
}
