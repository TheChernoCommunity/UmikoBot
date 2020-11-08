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

EventModule::EventModule(UmikoBot* client) : Module("event", true)
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
		if (!config.isEventRunning)
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
				"Event **expires** in `" + time + "`");

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
				"Event **expires** in `" + time + "`");

			client.createMessage(message.channelId(), embed);
			return;
		}

	});
	RegisterCommand(Commands::EVENT_GIVE_NEW_ACCESS, "give-new-event-access", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
		{
			QStringList args = message.content().split(' ');

			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN, [this, args, message, channel](bool result)
				{
					GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
					if (!result)
					{
						UmikoBot::Instance().createMessage(message.channelId(), "**You don't have permissions to use this command.**");
						return;
					}
					if (args.size() < 2)
					{
						UmikoBot::Instance().createMessage(channel.id(), "**Wrong Usage of Command!**");
						return;
					}
					snowflake_t guild = channel.guildId();

					auto& roles = message.mentionRoles();

					if (roles.isEmpty())
					{
						UmikoBot::Instance().createMessage(channel.id(), "**Wrong Usage of Command!\n**Please mention some role(s) so that I can add them.");
						return;
					}
					for (auto& role : roles)
					{
						if (m_EventWhitelist[guild].contains(role)) continue;
						m_EventWhitelist[guild].push_back(role);
					}

					UmikoBot::Instance().createMessage(channel.id(), "**The roles have been added.**\nPeople with the role(s) can now launch & end events!");

				});
		});
	RegisterCommand(Commands::EVENT_TAKE_NEW_ACCESS, "take-new-event-access", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
		{
			QStringList args = message.content().split(' ');

			Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN, [this, args, message, channel](bool result)
				{
					GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
					if (!result)
					{
						UmikoBot::Instance().createMessage(message.channelId(), "**You don't have permissions to use this command.**");
						return;
					}

					if (args.size() < 2) {
						UmikoBot::Instance().createMessage(channel.id(), "**Wrong Usage of Command!**");
						return;
					}

					snowflake_t guild = channel.guildId();

					auto& roles = message.mentionRoles();

					if (roles.isEmpty())
					{
						UmikoBot::Instance().createMessage(channel.id(), "**Wrong Usage of Command!\n**Please mention some role(s) so that I can do the needed.");
						return;
					}

					for (auto& role : roles)
					{
						m_EventWhitelist[guild].removeOne(role);
					}

					UmikoBot::Instance().createMessage(channel.id(), "**The roles have been removed.**\nPeople with the role(s) can no longer launch or end events!");

				});
		});
	RegisterCommand(Commands::EVENT_LAUNCH, "launch", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');

		client.getGuildMember(channel.guildId(), message.author().id())
			.then([=](const Discord::GuildMember& member)
			{
					bool found = false;

					for (auto& allowedRole : m_EventWhitelist[channel.guildId()])
					{
						if (member.roles().contains(allowedRole))
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						UmikoBot::Instance().createMessage(channel.id(), "**You don't have permissions to launch events.**");
						return;
					}

					auto& config = getEventServerData(channel.guildId());
					CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
					auto& currencyConfig = currencyModule->getServerData(channel.guildId());

					if (args.size() != 3)
					{
						UmikoBot::Instance().createMessage(message.channelId(), "**Wrong Usage of Command!** ");
						return;
					}

					if (config.isEventRunning)
					{
						UmikoBot::Instance().createMessage(message.channelId(), "An event is already running. Please stop that to start a new event!");
						return;
					}

					double time = args.at(2).toDouble();

					QRegExp re("[+]?\\d*\\.?\\d+");
					if (!re.exactMatch(args.at(2)))
					{
						UmikoBot::Instance().createMessage(message.channelId(), "**Wrong Usage of Command!**.");
						return;
					}
					if (time > 24)
					{
						UmikoBot::Instance().createMessage(message.channelId(), "**Currently you can't launch events with an expiry time of more than one day**.");
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

					if (args.at(1) != "HighRiskHighReward" && args.at(1) != "LowRiskLowReward")
					{
						UmikoBot::Instance().createMessage(message.channelId(), "`" + args.at(1) + "` is not an event");
						return;
					}
					if (args.at(1) == "HighRiskHighReward")
					{
						QString num = QString::number(time);

						Discord::Embed embed;
						embed.setColor(15844367);
						embed.setTitle("**HighRiskHighReward** event has been launched for `" + num + "` hour(s)!");
						embed.setDescription("Give command `!event` to see what event is running and its changes!");

						config.isEventRunning = true;
						config.eventHighRiskHighRewardRunning = true;
						UmikoBot::Instance().createMessage(message.channelId(), embed);
					}
					if (args.at(1) == "LowRiskLowReward")
					{
						QString num = QString::number(time);

						Discord::Embed embed;
						embed.setColor(15844367);
						embed.setTitle("**LowRiskLowReward** event has been launched for `" + num + "` hour(s)!");
						embed.setDescription("Give command `!event` to see what event is running and its changes!");

						config.isEventRunning = true;
						config.eventLowRiskLowRewardRunning = true;
						UmikoBot::Instance().createMessage(message.channelId(), embed);
					}
					auto guildID = channel.guildId();
					auto chan = message.channelId();

					QObject::connect(config.eventTimer, &QTimer::timeout, [this, guildID, chan]()
						{
							CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
							auto& currencyConfig = currencyModule->getServerData(guildID);

							auto& config = getEventServerData(guildID);
							if (config.eventHighRiskHighRewardRunning)
							{
								Discord::Embed embed;
								embed.setColor(15158332);
								embed.setTitle("**HighRiskHighReward** event has **ended**!");

								config.isEventRunning = false;
								config.eventHighRiskHighRewardRunning = false;
								UmikoBot::Instance().createMessage(chan, embed);
							}
							else if (config.eventLowRiskLowRewardRunning)
							{
								Discord::Embed embed;
								embed.setColor(15158332);
								embed.setTitle("**LowRiskLowReward** event has **ended**!");

								config.isEventRunning = false;
								config.eventLowRiskLowRewardRunning = false;
								UmikoBot::Instance().createMessage(chan, embed);
							}
						});
				config.eventTimer->start();
			});
	});
	RegisterCommand(Commands::EVENT_END, "endevent", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');

		client.getGuildMember(channel.guildId(), message.author().id())
			.then([=](const Discord::GuildMember& member)
			{
				bool found = false;

				for (auto& allowedRole : m_EventWhitelist[channel.guildId()])
				{
					if (member.roles().contains(allowedRole))
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					UmikoBot::Instance().createMessage(channel.id(), "**You don't have permissions to end events.**");
					return;
				}
				auto& config = getEventServerData(channel.guildId());
				CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
				auto& currencyConfig = currencyModule->getServerData(channel.guildId());

				GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
				if (args.size() != 1)
				{
					UmikoBot::Instance().createMessage(message.channelId(), "**Wrong Usage of Command!** ");
					return;
				}
				if (!config.isEventRunning)
				{
					UmikoBot::Instance().createMessage(message.channelId(), "**No event is running, so you can't end an event!**\n");
					return;
				}
				if (config.eventTimer->isActive())
				{
					if (config.eventHighRiskHighRewardRunning)
					{
						Discord::Embed embed;
						embed.setColor(15158332);
						embed.setTitle("**HighRiskHighReward** event has **ended**!");

						config.isEventRunning = false;
						config.eventHighRiskHighRewardRunning = false;
						config.eventTimer->stop();

						UmikoBot::Instance().createMessage(message.channelId(), embed);
						return;
					}
					if (config.eventLowRiskLowRewardRunning)
					{
						Discord::Embed embed;
						embed.setColor(15158332);
						embed.setTitle("**LowRiskLowReward** event has **ended**!");

						config.isEventRunning = false;
						config.eventLowRiskLowRewardRunning = false;
						config.eventTimer->stop();
						UmikoBot::Instance().createMessage(message.channelId(), embed);
					}
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
			QJsonArray list;
			auto config = serverEventConfig[server];
			for (auto& server : m_EventWhitelist.keys())
			{
				for (auto& roleId : m_EventWhitelist[server])
				{
					list.push_back(QString::number(roleId));
				}
				obj["event-whitelist"] = list;

			}
			serverList[QString::number(server)] = obj;
		}
		
		doc.setObject(docObj);

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
			config.isEventRunning = false;
			config.eventHighRiskHighRewardRunning = false;
			config.eventLowRiskLowRewardRunning = false;

			m_EventWhitelist.clear();
			auto servers = rootObj.keys();

			for (auto& server : servers)
			{
				snowflake_t guild = server.toULongLong();
				auto serverObj = rootObj[server].toObject();
				auto list = serverObj["event-whitelist"].toArray();

				for (auto role : list)
				{
					snowflake_t roleId = role.toString().toULongLong();
					m_EventWhitelist[guild].push_back(roleId);
				}
			}
			auto guildId = server.toULongLong();
			serverEventConfig.insert(guildId, config);
		}
		eventConfigfile.close();
	}
}
