#include "UmikoBot.h"
#include "EventModule.h"
#include "core/Permissions.h"
#include "core/Utility.h"
#include "CurrencyModule.h"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/qregexp.h>
#include <QtCore/QtMath>

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
		if (config.isHighRiskHighRewardRunning)
		{
			int num = config.eventTimer->remainingTime();
			QString time = utility::StringifyMilliseconds(num);

			Embed embed;
			embed.setColor(15844367);
			embed.setTitle("High Risk High Reward event");
			embed.setDescription("The steal chance is **decreased** but if you succeed you will get **BONUS " + currencyConfig.currencySymbol + "**!\n"
								"Event ends in `" + time + "`");

			client.createMessage(message.channelId(), embed);
			return;

		}
		if (config.isLowRiskLowRewardRunning)
		{
			int num = config.eventTimer->remainingTime();
			QString time = utility::StringifyMilliseconds(num);

			Embed embed;
			embed.setColor(15844367);
			embed.setTitle("Low Risk Low Reward event");
			embed.setDescription("The steal chance is **increased** but you have to pay a **PENALTY** if you succeed!\n"
								"Event ends in `" + time + "`");

			client.createMessage(message.channelId(), embed);
			return;
		}
		if (config.eventRaffleDrawRunning)
		{
			int num = config.eventTimer->remainingTime();
			QString time = utility::StringifyMilliseconds(num);

			Embed embed;
			embed.setColor(15844367);
			embed.setTitle("Raffle Draw event");
			embed.setDescription("You can buy tickets and earn cool rewards if you win!\n"
								"The result of the draw is announced when the event expires.\n"
								"Use `!buytickets <amount>` to buy ticket for participating.\n"
								"Buying more tickets increases your success chance!\n"
								"Event ends in `" + time + "`");

			client.createMessage(message.channelId(), embed);
		}

	});
	RegisterCommand(Commands::EVENT_GIVE_NEW_ACCESS, "give-new-event-access", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');
		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, channel, message]()
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

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, false, [this, channel, message]()
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
						UmikoBot::Instance().createMessage(channel.id(), "**You don't have permissions to launch events.**");
						return;
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
					if (time > static_cast<double>(24 * 24))
					{
						UmikoBot::Instance().createMessage(message.channelId(), "**Events cannot last longer than 24 days!**.");
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

					bool foundEvent = (std::find(eventNamesAndCodes.begin(), eventNamesAndCodes.end(), args.at(1)) != eventNamesAndCodes.end());

					if (!foundEvent)
					{
						UmikoBot::Instance().createMessage(message.channelId(), "`" + args.at(1) + "` is not an event name or code!");
						return;
					}
					QString eventEmoji = QString(utility::consts::emojis::REGIONAL_INDICATOR_E) + " " +QString(utility::consts::emojis::REGIONAL_INDICATOR_V) + " " +
													QString(utility::consts::emojis::REGIONAL_INDICATOR_E) + " " + QString(utility::consts::emojis::REGIONAL_INDICATOR_N) + " " +
															QString(utility::consts::emojis::REGIONAL_INDICATOR_T);

					if (args.at(1) == "HRHR")
					{
						QString num = QString::number(time);

						Embed embed;
						embed.setColor(15844367);
						embed.setTitle(eventEmoji);
						embed.setDescription("**HighRiskHighReward** event has been launched for `" + num + "` hour(s)!\nGive command `!event` to see what event is running and its changes!");

						config.isEventRunning = true;
						config.isHighRiskHighRewardRunning = true;
						UmikoBot::Instance().createMessage(message.channelId(), embed);
					}
					if (args.at(1) == "LRLR")
					{
						QString num = QString::number(time);

						Embed embed;
						embed.setColor(15844367);
						embed.setTitle(eventEmoji);
						embed.setDescription("**LowRiskLowReward** event has been launched for `" + num + "` hour(s)!\nUse `!event` to see what event is running and its changes!");

						config.isEventRunning = true;
						config.isLowRiskLowRewardRunning = true;
						UmikoBot::Instance().createMessage(message.channelId(), embed);
					}
					if (args.at(1) == "RaffleDraw")
					{
						if(config.claimedReward)
						{
							if (config.raffleDrawRewardClaimTimer != nullptr) //Delete the previous timer
							{
								delete config.raffleDrawRewardClaimTimer;
								config.raffleDrawRewardClaimTimer = nullptr;
							}
							auto& configRD = raffleDrawGuildList[channel.guildId()][raffleDrawGetUserIndex(channel.guildId(), message.author().id())];
							QString num = QString::number(time);
							Embed embed;
							embed.setColor(15844367);
							embed.setTitle(eventEmoji);
							embed.setDescription("**RAFFLE DRAW** event has been launched for `" + num + "` hour(s)!\nUse `!event` to see what event is running and its changes!");
							config.claimedReward = false;
							config.isEventRunning = true;
							config.eventRaffleDrawRunning = true;
							UmikoBot::Instance().createMessage(message.channelId(), embed);
						}
						else
						{
							auto& config = getServerEventData(channel.guildId());
							int timeLeft = config.raffleDrawRewardClaimTimer->remainingTime();
							QString time = utility::StringifyMilliseconds(timeLeft);
							UmikoBot::Instance().createMessage(message.channelId(), "**The winner of the previous RaffleDraw hasn't claimed their rewards yet.**\n"
																					"Please wait `"+ time +"` to start RaffleDraw again.");
							return;
						}
					
					}
					auto guildID = channel.guildId();
					auto chan = message.channelId();
					auto authorId = message.author().id();

					QObject::connect(config.eventTimer, &QTimer::timeout, [this, guildID, chan, authorId, eventEmoji]()
						{
							CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
							auto& currencyConfig = currencyModule->getServerData(guildID);

							auto& config = getServerEventData(guildID);
							if (config.isHighRiskHighRewardRunning)
							{
								EndEvent(chan, guildID, authorId, true, "HRHR");
							}
							else if (config.isLowRiskLowRewardRunning)
							{
								EndEvent(chan, guildID, authorId, true, "LRLR");
							}
							if (config.eventRaffleDrawRunning)
							{
								EndEvent(chan, guildID, authorId, true, "RaffleDraw");
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
						UmikoBot::Instance().createMessage(channel.id(), "**You don't have permissions to end events.**");
						return;
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
						if (config.isHighRiskHighRewardRunning)
						{
							EndEvent(message.channelId(), channel.guildId(), message.author().id(), false, "HRHR");
						}
						if (config.isLowRiskLowRewardRunning)
						{
							EndEvent(message.channelId(), channel.guildId(), message.author().id(), false, "LRLR");
						}
						if (config.eventRaffleDrawRunning)
						{
							EndEvent(message.channelId(), channel.guildId(), message.author().id(), false, "RaffleDraw");
						}
					}

				});
	});
	RegisterCommand(Commands::EVENT_GET_REWARD, "getreward", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');
		if (args.size() != 1)
		{
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}
		auto& config = getServerEventData(channel.guildId());
		if (config.isEventRunning)
		{
			client.createMessage(message.channelId(), "**You can't get rewards anytime!**");
			return;
		}
		auto& configRD = raffleDrawGuildList[channel.guildId()][raffleDrawGetUserIndex(channel.guildId(), message.author().id())];
		if (!config.claimedReward)
		{
			if(message.author().id() == config.luckyUser)
			{
				if (config.raffleDrawRewardClaimTimer->isActive())
				{
					CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
					auto& currencyConfig = currencyModule->getServerData(channel.guildId());
					auto& authorCurrency = currencyModule->guildList[channel.guildId()][currencyModule->getUserIndex(channel.guildId(), message.author().id())];
					QString authorName = UmikoBot::Instance().GetName(channel.guildId(), message.author().id());
					config.claimedReward = true;
					Embed embed;
					embed.setColor(15844367);
					authorCurrency.setCurrency(authorCurrency.currency() + 400.0);
					embed.setTitle("**Raffle Draw Reward goes to " + authorName + "!**");
					embed.setDescription("Congratulations **" + authorName + "**!\n"
										"You just got `400 " + currencyConfig.currencySymbol + "`");
					config.raffleDrawRewardClaimTimer->stop();
					client.createMessage(message.channelId(), embed);
					return;
				}
			}
			else if (config.raffleDrawRewardClaimTimer->isActive())
			{
				client.createMessage(message.channelId(), "**BRUH, you don't have the lucky ticket!**");
				return;
			}
		}
		else
		{
			client.createMessage(message.channelId(), "**You can't get rewards anytime!**");
		}
	});
	RegisterCommand(Commands::EVENT_BUY_TICKETS, "buytickets", [this](Client& client, const Message& message, const Channel& channel)
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
		snowflake_t authorID = message.author().id();
		auto& configRD = raffleDrawGuildList[channel.guildId()][raffleDrawGetUserIndex(channel.guildId(), authorID)];

		if ((ticket > (config.maxUserTickets - configRD.m_TicketIds.size())) || (configRD.m_TicketIds.size() >= config.maxUserTickets))
		{
			QString num = QString::number(config.maxUserTickets);
			client.createMessage(message.channelId(), "**Don't be selfish!**\nLet others also have a go at some tickets. You can only have " + num + " tickets at max!");
			return;
		}
		if (ticket == 0)
		{
			client.createMessage(message.channelId(), "**BrUh, don't bother me if you don't want to buy ticket..**");
			return;
		}
		snowflake_t chan = message.channelId();
		double totalFee = (static_cast<double>(config.raffleDrawTicketPrice) * ticket);
		CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
		auto& authorCurrency = currencyModule->guildList[channel.guildId()][currencyModule->getUserIndex(channel.guildId(), authorID)];

		if (authorCurrency.currency() <= 0.0)
		{
			client.createMessage(message.channelId(), "**Haha you poor bruh. Git gud enough to afford tickets.**");
			return;
		}
		if (totalFee > (double)authorCurrency.currency())
		{
			client.createMessage(message.channelId(), "**Can you afford that many tickets?**");
			return;
		}
		for (int i = 0; i != ticket; i++)
		{
			config.numTicketsBought++;
			configRD.m_TicketIds.push_back(config.numTicketsBought);
		}

		authorCurrency.setCurrency(authorCurrency.currency() - totalFee);
		QString ticketEmoji = QString(utility::consts::emojis::TICKETS);
		QString ticketAmount = QString::number(configRD.m_TicketIds.size());
		client.createMessage(message.channelId(), ticketEmoji + " **You own** `" + ticketAmount + "` **ticket(s) now!**\n *(Buy more of them to increase your chance)*");
	});
	RegisterCommand(Commands::EVENT_TICKET, "ticket", [this](Client& client, const Message& message, const Channel& channel)
	{
		GuildSetting setting = GuildSettings::GetGuildSetting(channel.guildId());
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
			client.createMessage(message.channelId(), "**You have no tickets.**\nYou can buy them by using `" + setting.prefix + "buytickets <amount>`");
			return;
		}
		QString name = UmikoBot::Instance().GetName(channel.guildId(), message.author().id());
		QString totalTicket = QString::number(configRD.m_TicketIds.size());
		QString ticketEmoji = QString(utility::consts::emojis::TICKETS);
		QString ticketAmount = QString::number(configRD.m_TicketIds.size());
		QString txt = QString(ticketEmoji + "Total Ticket(s) belonging to **%1**: `%2`\nYour ticket number(s) are:\n").arg(name, totalTicket);

		for (auto ticket : configRD.m_TicketIds)
		{
			if (ticket == configRD.m_TicketIds.last())
			{
				txt += QString("%1").arg(ticket);
				client.createMessage(message.channelId(), txt);
				return;
			}
			txt += QString("%1, ").arg(ticket);
		}
	});
	RegisterCommand(Commands::EVENT_SET_TICKET_PRICE, "setticketprice", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
			{
				auto& config = getServerEventData(channel.guildId());
				config.raffleDrawTicketPrice = args.at(1).toDouble();
				client.createMessage(message.channelId(), "Raffle Draw ticket price set to **" + QString::number(config.raffleDrawTicketPrice) + "**");
			});
	});
	RegisterCommand(Commands::EVENT_SET_USER_MAX_TICKET, "setusermaxticket", [this](Client& client, const Message& message, const Channel& channel)
	{
		QStringList args = message.content().split(' ');

		UmikoBot::VerifyAndRunAdminCmd(client, message, channel, 2, args, true, [this, &client, channel, message, args]()
			{
				auto& config = getServerEventData(channel.guildId());
				config.maxUserTickets = args.at(1).toDouble();
				client.createMessage(message.channelId(), "Max ticket set to **" + QString::number(config.maxUserTickets) + "**");
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
			obj["maxUserTickets"] = QString::number(config.maxUserTickets);
			obj["currentTicketIndex"] = QString::number(config.numTicketsBought);
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

		for (const auto& server : servers)
		{
			EventConfig config;
			auto serverObj = rootObj[server].toObject();
			config.raffleDrawTicketPrice = serverObj["raffleDrawTicketPrice"].toString("50").toInt();
			config.numTicketsBought = serverObj["currentTicketIndex"].toString("0").toInt();
			config.maxUserTickets = serverObj["maxUserTickets"].toString("20").toInt();
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

void EventModule::EndEvent(const snowflake_t& channelID, const snowflake_t& guildID, const snowflake_t& authorID, bool isInQObjectConnect, QString eventNameOrCode)
{
	QString eventEmoji = QString(utility::consts::emojis::REGIONAL_INDICATOR_E) + " " + QString(utility::consts::emojis::REGIONAL_INDICATOR_V) + " " +
		QString(utility::consts::emojis::REGIONAL_INDICATOR_E) + " " + QString(utility::consts::emojis::REGIONAL_INDICATOR_N) + " " +
		QString(utility::consts::emojis::REGIONAL_INDICATOR_T);

	auto& config = getServerEventData(guildID);
	CurrencyModule* currencyModule = static_cast<CurrencyModule*>(UmikoBot::Instance().GetModuleByName("currency"));
	auto& currencyConfig = currencyModule->getServerData(guildID);

	if (eventNameOrCode == "HRHR")
	{
		Embed embed;
		embed.setColor(15158332);
		embed.setTitle(eventEmoji);
		embed.setDescription("**HighRiskHighReward** event has **ended**!");
		config.isEventRunning = false;
		config.isHighRiskHighRewardRunning = false;
		if(!isInQObjectConnect)
			config.eventTimer->stop();

		UmikoBot::Instance().createMessage(channelID, embed);
		return;
	}
	if (eventNameOrCode == "LRLR")
	{
		Embed embed;
		embed.setColor(15158332);
		embed.setTitle(eventEmoji);
		embed.setDescription("**LowRiskLowReward** event has **ended**!");

		config.isEventRunning = false;
		config.isLowRiskLowRewardRunning = false;
		if (!isInQObjectConnect)
			config.eventTimer->stop();
		UmikoBot::Instance().createMessage(channelID, embed);
		return;
	}
	if(eventNameOrCode == "RaffleDraw")
	{
		Embed embed;
		embed.setColor(15844367);
		embed.setTitle(eventEmoji);
	
		if (config.numTicketsBought == 0)
		{
			embed.setDescription("**RAFFLE DRAW** event has **ended**!\n"
				"**There was no draw because no one bought tickets**");
			config.claimedReward = true;
			config.isEventRunning = false;
			config.eventRaffleDrawRunning = false;
			config.eventTimer->stop();
			UmikoBot::Instance().createMessage(channelID, embed);
			return;
		}
	
		//Announce Winner
		int lastTicket = static_cast<int>(config.numTicketsBought);
		std::random_device randomDevice;
		std::mt19937 gen(randomDevice());
		std::uniform_int_distribution<> dist(1, lastTicket);
		int luckyTicketId = dist(gen);
	
		QString tiketId = QString::number(luckyTicketId);
		embed.setDescription("**RAFFLE DRAW** event has **ended**!\n"
			"**The lucky ticket which wins the draw is `" + tiketId + "`**\n"
			"**The owner can claim their rewards within 24 hours**");
		config.claimedReward = false;
		config.numTicketsBought = 0;
	
		//Get the user who has the lucky ticket
		for (auto& user : raffleDrawGuildList[guildID])
		{
			for (auto& userTicket : user.m_TicketIds)
			{
				if (luckyTicketId == userTicket)
				{
					config.luckyUser = user.m_UserId;
				}
			}
		}
	
		//Clear all the tickets
		for (auto& user : raffleDrawGuildList[guildID])
			user.m_TicketIds.clear();
	
		//Set the reward claiming time (24 hours)
		config.raffleDrawRewardClaimTimer = new QTimer;
		config.raffleDrawRewardClaimTimer->setInterval(24 * 3600000);
		config.raffleDrawRewardClaimTimer->setSingleShot(true);
	
		QObject::connect(config.raffleDrawRewardClaimTimer, &QTimer::timeout, [this, guildID, channelID, authorID]()
			{
				auto& config = getServerEventData(guildID);
				config.claimedReward = true;
				UmikoBot::Instance().createMessage(channelID, "The owner of the price in the RaffleDraw didn't get their rewards!\n"
					"The role(s) who have permission can start this event again!");
			});
		config.raffleDrawRewardClaimTimer->start();
		config.isEventRunning = false;
		config.eventRaffleDrawRunning = false;
		if(!isInQObjectConnect)
			config.eventTimer->stop(); // Forcefully stop the timer.
	
		UmikoBot::Instance().createMessage(channelID, embed);
	}
}

int EventModule::raffleDrawGetUserIndex(snowflake_t guild, snowflake_t id)
{
	for (auto it = raffleDrawGuildList[guild].begin(); it != raffleDrawGuildList[guild].end(); ++it)
	{
		if (it->m_UserId == id)
		{
			return std::distance(raffleDrawGuildList[guild].begin(), it);
		}
	}
	raffleDrawGuildList[guild].append(RaffleDraw { id });
	return std::distance(raffleDrawGuildList[guild].begin(), std::prev(raffleDrawGuildList[guild].end()));
}

EventModule::RaffleDraw EventModule::getUserRaffleDrawData(snowflake_t guild, snowflake_t id)
{
	for (auto user : raffleDrawGuildList[guild])
	{
		if (user.m_UserId == id)
		{
			return user;
		}
	}
	RaffleDraw user { id };
	raffleDrawGuildList[guild].append(user);
	return raffleDrawGuildList[guild].back();
}
