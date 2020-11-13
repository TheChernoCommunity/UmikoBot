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

using namespace Discord;
EventModule::EventModule(UmikoBot* client) : Module("event", true)
{
	RegisterCommand(Commands::EVENT, "event", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');
		auto& config = getServerEventData(channel.guildId());
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

			Embed embed;
			embed.setColor(15844367);
			embed.setTitle("**HighRiskHighReward** event");
			embed.setDescription("The steal chance is **decreased** but if you succeed you will get **BONUS " + currencyConfig.currencySymbol + "**!\n"
								"Event **ends** in `" + time + "`");

			client.createMessage(message.channelId(), embed);
			return;

		}
		if (config.eventLowRiskLowRewardRunning)
		{
			int num = config.eventTimer->remainingTime();
			QString time = utility::StringifyMilliseconds(num);

			Embed embed;
			embed.setColor(15844367);
			embed.setTitle("**LowRiskLowReward** event");
			embed.setDescription("The steal chance is **increased** but you have to give a **PENALTY** if you succeed!\n"
								"Event **ends** in `" + time + "`");

			client.createMessage(message.channelId(), embed);
			return;
		}
		if (config.eventRaffleDrawRunning)
		{
			int num = config.eventTimer->remainingTime();
			QString time = utility::StringifyMilliseconds(num);

			Embed embed;
			embed.setColor(15844367);
			embed.setTitle("**RAFFLE DRAW** event");
			embed.setDescription("You can buy tickets and earn cool rewards if you win!\n"
								"The results of the draw is announced when the event expires.\n"
								"Use `!buyticket <amount>` to buy ticket for participating.\n"
								"Buying more tickets increases your success chance!\n"
								"Event **ends** in `" + time + "`");

			client.createMessage(message.channelId(), embed);
		}

	});
	RegisterCommand(Commands::EVENT_GIVE_NEW_ACCESS, "give-new-event-access", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');
		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, &client, channel, message, args]()
			{
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
	RegisterCommand(Commands::EVENT_TAKE_NEW_ACCESS, "take-new-event-access", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, &client, channel, message, args]()
			{
				snowflake_t guild = channel.guildId();
				auto& roles = message.mentionRoles();

				if (roles.isEmpty())
				{
					UmikoBot::Instance().createMessage(channel.id(), "**Wrong Usage of Command!\n**Please mention some role(s) so that I can remove them.");
					return;
				}

				for (auto& role : roles)
				{
					m_EventWhitelist[guild].removeOne(role);
				}

				UmikoBot::Instance().createMessage(channel.id(), "**The roles have been removed.**\nPeople with the role(s) can no longer launch or end events!");
			});

	});
	RegisterCommand(Commands::EVENT_LAUNCH, "launch", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		client.getGuildMember(channel.guildId(), message.author().id())
			.then([=](const GuildMember& member)
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
						//UmikoBot::Instance().createMessage(channel.id(), "**You don't have permissions to launch events.**");
						//return;
					}

					auto& config = getServerEventData(channel.guildId());
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

					if (args.at(1) != "HighRiskHighReward" && args.at(1) != "LowRiskLowReward" && args.at(1) != "RaffleDraw")
					{
						UmikoBot::Instance().createMessage(message.channelId(), "`" + args.at(1) + "` is not an event");
						return;
					}
					QString eventEmoji = QString(utility::consts::emojis::REGIONAL_INDICATOR_E) + " " +QString(utility::consts::emojis::REGIONAL_INDICATOR_V) + " " +
													QString(utility::consts::emojis::REGIONAL_INDICATOR_E) + " " + QString(utility::consts::emojis::REGIONAL_INDICATOR_N) + " " +
															QString(utility::consts::emojis::REGIONAL_INDICATOR_T);

					if (args.at(1) == "HighRiskHighReward")
					{
						QString num = QString::number(time);

						Embed embed;
						embed.setColor(15844367);
						embed.setTitle(eventEmoji);
						embed.setDescription("**HighRiskHighReward** event has been launched for `" + num + "` hour(s)!\nGive command `!event` to see what event is running and its changes!");

						config.isEventRunning = true;
						config.eventHighRiskHighRewardRunning = true;
						UmikoBot::Instance().createMessage(message.channelId(), embed);
					}
					if (args.at(1) == "LowRiskLowReward")
					{
						QString num = QString::number(time);

						Embed embed;
						embed.setColor(15844367);
						embed.setTitle(eventEmoji);
						embed.setDescription("**LowRiskLowReward** event has been launched for `" + num + "` hour(s)!\nUse `!event` to see what event is running and its changes!");

						config.isEventRunning = true;
						config.eventLowRiskLowRewardRunning = true;
						UmikoBot::Instance().createMessage(message.channelId(), embed);
					}
					if (args.at(1) == "RaffleDraw")
					{
						auto& configRD = raffleDrawGuildList[channel.guildId()][raffleDrawGetUserIndex(channel.guildId(), message.author().id())];
						QString num = QString::number(time);
						Embed embed;
						embed.setColor(15844367);
						embed.setTitle(eventEmoji);
						embed.setDescription("**RAFFLE DRAW** event has been launched for `" + num + "` hour(s)!\nUse `!event` to see what event is running and its changes!");

						config.isEventRunning = true;
						config.eventRaffleDrawRunning = true;
						UmikoBot::Instance().createMessage(message.channelId(), embed);
					}
					auto guildID = channel.guildId();
					auto chan = message.channelId();
					auto authorId = message.author().id();

					QObject::connect(config.eventTimer, &QTimer::timeout, [this, guildID, chan, authorId, eventEmoji]()
						{
							CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
							auto& currencyConfig = currencyModule->getServerData(guildID);

							auto& config = getServerEventData(guildID);
							if (config.eventHighRiskHighRewardRunning)
							{
								Embed embed;
								embed.setColor(15158332);
								embed.setTitle(eventEmoji);
								embed.setDescription("**HighRiskHighReward** event has **ended**!");

								config.isEventRunning = false;
								config.eventHighRiskHighRewardRunning = false;
								UmikoBot::Instance().createMessage(chan, embed);
							}
							else if (config.eventLowRiskLowRewardRunning)
							{
								Embed embed;
								embed.setColor(15158332);
								embed.setTitle(eventEmoji);
								embed.setDescription("**LowRiskLowReward** event has **ended**!");

								config.isEventRunning = false;
								config.eventLowRiskLowRewardRunning = false;
								UmikoBot::Instance().createMessage(chan, embed);
							}
							if (config.eventRaffleDrawRunning)
							{
								Embed embed;
								embed.setColor(15158332);
								embed.setTitle(eventEmoji);
								embed.setDescription("**RAFFLE DRAW** event has **ended**!");
								config.currentTicketIndex = 0;
								for (auto& user : raffleDrawGuildList[guildID])
								{
									user.m_TicketIds.clear();
								}

								// Announce winner
								config.isEventRunning = false;
								config.eventRaffleDrawRunning = false;
								UmikoBot::Instance().createMessage(chan, embed);
							}
						});
					config.eventTimer->start();
				});
	});
	RegisterCommand(Commands::EVENT_END, "endevent", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		client.getGuildMember(channel.guildId(), message.author().id())
			.then([=](const GuildMember& member)
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
						//UmikoBot::Instance().createMessage(channel.id(), "**You don't have permissions to end events.**");
						//return;
					}
					auto& config = getServerEventData(channel.guildId());
					CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
					auto& currencyConfig = currencyModule->getServerData(channel.guildId());

					QString eventEmoji = QString(utility::consts::emojis::REGIONAL_INDICATOR_E) + " " + QString(utility::consts::emojis::REGIONAL_INDICATOR_V) + " " +
											QString(utility::consts::emojis::REGIONAL_INDICATOR_E) + " " + QString(utility::consts::emojis::REGIONAL_INDICATOR_N) + " " +
												QString(utility::consts::emojis::REGIONAL_INDICATOR_T);

					if (args.size() != 1)
					{
						UmikoBot::Instance().createMessage(message.channelId(), "**Wrong Usage of Command!** ");
						return;
					}
					if (!config.isEventRunning)
					{
						UmikoBot::Instance().createMessage(message.channelId(), "**What do I even end?**\n"
																				"I (unlike you) can properly keep track of things, and no events are going on at the moment.");
						return;
					}
					if (config.eventTimer->isActive())
					{
						if (config.eventHighRiskHighRewardRunning)
						{
							Embed embed;
							embed.setColor(15158332);
							embed.setTitle(eventEmoji);
							embed.setDescription("**HighRiskHighReward** event has **ended**!");

							config.isEventRunning = false;
							config.eventHighRiskHighRewardRunning = false;
							config.eventTimer->stop();

							UmikoBot::Instance().createMessage(message.channelId(), embed);
							return;
						}
						if (config.eventLowRiskLowRewardRunning)
						{
							Embed embed;
							embed.setColor(15158332);
							embed.setTitle(eventEmoji);
							embed.setDescription("**LowRiskLowReward** event has **ended**!");

							config.isEventRunning = false;
							config.eventLowRiskLowRewardRunning = false;
							config.eventTimer->stop();
							UmikoBot::Instance().createMessage(message.channelId(), embed);
						}
						if (config.eventRaffleDrawRunning)
						{
							Embed embed;
							embed.setColor(15158332);
							embed.setTitle(eventEmoji);
							embed.setDescription("**RAFFLE DRAW** event has **ended**!\n");
							config.currentTicketIndex = 0;
							config.isEventRunning = false;
							config.eventRaffleDrawRunning = false;
							for (auto& user : raffleDrawGuildList[channel.guildId()])
							{
								user.m_TicketIds.clear();
								qDebug() << "list cleared";
							}
							config.eventTimer->stop();
							UmikoBot::Instance().createMessage(message.channelId(), embed);
						}
					}

				});
	});

	RegisterCommand(Commands::EVENT_BUY_TICKET, "buyticket", [this](Client& client, const Message& message, const Channel& channel)
		{
			QStringList args = message.content().split(' ');
			auto& config = getServerEventData(channel.guildId());
			if (!config.eventRaffleDrawRunning)
			{
				client.createMessage(message.channelId(), "**RAFFLE DRAW** in not running. You can buy tickets when **RAFFLE DRAW** starts!");
				return;
			}
			if (args.size() != 2)
			{
				client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
				return;
			}
			QRegExp re("[+]?\\d*\\.?\\d+");
			if (!re.exactMatch(args.at(1)))
			{
				client.createMessage(message.channelId(), "**You can't buy ticket(s) in invalid amounts**");
				return;
			}
			unsigned int ticket = args.at(1).toUInt();
			
			if (ticket == 0)
			{
				client.createMessage(message.channelId(), "**BrUh, don't bother me if you don't want to buy ticket..**");
				return;
			}
			snowflake_t authorID = message.author().id();
			snowflake_t chan = message.channelId();
			double totalFee = (static_cast<double>(config.raffleDrawTicketPrice) * ticket);
			CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
			auto& authorCurrency = currencyModule->guildList[channel.guildId()][currencyModule->getUserIndex(channel.guildId(), authorID)];

			if (authorCurrency.currency() <= 0)
			{
				client.createMessage(message.channelId(), "**Haha, you poor people can't buy tickets!**");
				return;
			}
			if (totalFee > authorCurrency.currency())
			{
				client.createMessage(message.channelId(), "**Can you afford that much money?**");
				return;
			}
			auto& configRD = raffleDrawGuildList[channel.guildId()][raffleDrawGetUserIndex(channel.guildId(), authorID)];

			for (int i = 0; i != ticket; i++)
			{
				config.currentTicketIndex++;
				configRD.m_TicketIds.push_back(config.currentTicketIndex);
			}

			authorCurrency.setCurrency(authorCurrency.currency() - totalFee);
			QString ticketEmoji = QString(utility::consts::emojis::TICKETS);
			QString ticketAmount = QString::number(configRD.m_TicketIds.size());
			client.createMessage(message.channelId(), ticketEmoji + " **You own** `" + ticketAmount + "` **ticket(s) now!**\n *(You can buy them more to increase your chance)*");
		});
	RegisterCommand(Commands::EVENT_TICKET, "ticket", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');
		if (args.size() != 1)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}
		auto& configRD = raffleDrawGuildList[channel.guildId()][raffleDrawGetUserIndex(channel.guildId(), message.author().id())];
		auto& config = getServerEventData(channel.guildId());
		if (configRD.m_TicketIds.size() == 0)
		{
			client.createMessage(message.channelId(), "**You have no tickets.\nYou can buy them by using** `!buyticket <amount>`");
			return;
		}
		QString name = UmikoBot::Instance().GetName(channel.guildId(), message.author().id());
		QString totalTicket = QString::number(configRD.m_TicketIds.size());
		QString ticketEmoji = QString(utility::consts::emojis::TICKETS);
		QString ticketAmount = QString::number(configRD.m_TicketIds.size());
		QString txt = QString("**Total Ticket(s) of `%1`: `%2`**\n**Your ticket numbers are:**\n").arg(name, totalTicket);

		for (int tickets : configRD.m_TicketIds)
		{
			txt += QString("|`%1`").arg(tickets);
		}
		txt += QString("|");
		client.createMessage(message.channelId(), txt);
	});
	RegisterCommand(Commands::EVENT_SET_TICKET_PRICE, "setraffledrawticketprice", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
			{
				auto& config = getServerEventData(channel.guildId());
				config.raffleDrawTicketPrice = args.at(1).toInt();
				client.createMessage(message.channelId(), "Raffle Draw ticket price set to **" + QString::number(config.raffleDrawTicketPrice) + "**");
			});
	});
}

void EventModule::OnMessage(Client& client, const Message& message)
{
	Module::OnMessage(client, message);
}

void EventModule::OnSave(QJsonDocument& doc) const
{
	QJsonObject docObj;

	for (auto server : raffleDrawGuildList.keys())
	{
		QJsonObject serverJSON;

		for (auto user = raffleDrawGuildList[server].begin(); user != raffleDrawGuildList[server].end(); user++)
		{
			QJsonObject obj;
			QJsonArray list;

			for (auto ticket : user->m_TicketIds)
			{
				list.push_back(QString::number(ticket));
			}
			obj["tickets"] = list;
			serverJSON[QString::number(user->m_UserId)] = obj;
		}

		docObj[QString::number(server)] = serverJSON;
	}
	doc.setObject(docObj);

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
			obj["raffleDrawTicketPrice"] = QString::number(config.raffleDrawTicketPrice);
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
	QJsonObject docObj = doc.object();
	QStringList servers = docObj.keys();

	raffleDrawGuildList.clear();

	for (auto server : servers)
	{
		auto guildId = server.toULongLong();
		auto obj = docObj[server].toObject();
		QStringList users = obj.keys();

		QList<RaffleDraw> list;
		for (auto user : users)
		{
			QJsonArray ticketList = obj[user].toObject()["tickets"].toArray();
			RaffleDraw raffleDrawData
			{
				user.toULongLong()
			};
			for (auto ticket : ticketList)
			{
				unsigned int ticketId = ticket.toString().toUInt();
				raffleDrawData.m_TicketIds.push_back(ticketId);
			} 
			
			list.append(raffleDrawData);
		}
		raffleDrawGuildList.insert(guildId, list);

	}
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
			config.eventRaffleDrawRunning = false;
			config.raffleDrawTicketPrice = serverObj["raffleDrawTicketPrice"].toString("50").toInt();
			config.currentTicketIndex = 0;
			m_EventWhitelist.clear();

			snowflake_t guild = server.toULongLong();
			auto list = serverObj["event-whitelist"].toArray();

			for (auto role : list)
			{
				snowflake_t roleId = role.toString().toULongLong();
				m_EventWhitelist[guild].push_back(roleId);
			}
			
			auto guildId = server.toULongLong();
			serverEventConfig.insert(guildId, config);
		}
		eventConfigfile.close();
	}
}
