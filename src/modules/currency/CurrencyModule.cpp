#include "modules/currency/CurrencyModule.h"
#include "UmikoBot.h"

//! Currency Config Location
#define currenConfigLoc QString("currencyConfig")

using namespace Discord;

CurrencyModule::CurrencyModule(UmikoBot* client) : Module("currency", true), m_client(client)
{
	initiateGiveaways(client);
	initiateStatus();
	initiateEarnCommands();
	initiateAdminCommands();
}

void CurrencyModule::StatusCommand(QString& result, snowflake_t guild, snowflake_t user) 
{
	QString creditScore = QString::number((double)getUserData(guild, user).currency());
	auto& config = getServerData(guild);

	result += "Wallet: " + creditScore + " " + getServerData(guild).currencySymbol;
	result+='\n';
}

void CurrencyModule::OnMessage(Client& client, const Message& message) 
{
	client.getChannel(message.channelId()).then(
		[this, message, &client](const Channel& channel) 
		{
			if (channel.guildId() != 0 && !message.author().bot()) 
				//! Make sure the message is not a DM
			{
				auto guildId = channel.guildId();
				auto& serverConfig = getServerData(guildId);

				if (!serverConfig.isRandomGiveawayDone && !serverConfig.allowGiveaway) 
				{
						randp.param(std::bernoulli_distribution::param_type(serverConfig.randGiveawayProb));
						bool outcome = randp(random_engine);
						if (outcome) {
							client.createMessage(serverConfig.giveawayChannelId, "Hey everyone! **FREEBIE** available now! Go `!claim` some juicy coins!");
							serverConfig.allowGiveaway = true;
						}
				
				}


				//! If the message is a number, continue with the gambling mech
				QRegExp re("\\d*");
				if (re.exactMatch(message.content())) 
				{
		
					auto gambleAllowed = gambleData[guildId].gamble;
					auto isbotAuthor = message.author().bot();

					auto isGambleChannel = message.channelId() == gambleData[guildId].channelId;

					auto isUserGambler = message.author().id() == gambleData[guildId].userId;

					if (gambleAllowed && !isbotAuthor && isGambleChannel && isUserGambler)
					{
						auto guess = message.content().toInt();

						if (guess > serverConfig.maxGuess || guess < serverConfig.minGuess)
						{

							client.createMessage(message.channelId(), "**Your guess is out of range!** \nTry a number between " + QString::number(serverConfig.minGuess) + " and " + QString::number(serverConfig.maxGuess) + " (inclusive). ");
							return;

						}
						auto playerWon = guess == gambleData[guildId].randNum;
						if (playerWon) 
						{

							Currency prize = gambleData[guildId].doubleOrNothing ? gambleData[guildId].betAmount * 2.0 : serverConfig.gambleReward;

							auto index = getUserIndex(guildId, 
								message.author().id());
							guildList[guildId][index].setCurrency(guildList[guildId][index].currency() + prize);
							client.createMessage(message.channelId(),
								"**You guessed CORRECTLY!**\n(**" +
								QString::number((double)prize) +
								serverConfig.currencySymbol + 
								"** have been added to your wallet!)");

						}
						else 
						{

							auto loss = gambleData[guildId].doubleOrNothing ? gambleData[guildId].betAmount : serverConfig.gambleLoss;
							auto symbol = serverConfig.currencySymbol;

							auto index = getUserIndex(guildId, message.author().id());
							guildList[guildId][index].setCurrency(guildList[guildId][index].currency() - loss);
							client.createMessage(message.channelId(), "**Better Luck next time! The number was `" + QString::number(gambleData[guildId].randNum) +"`**\n*(psst! I took **" + QString::number((double)loss) + symbol + "** from your wallet for my time...)*");

						}

						gambleData[guildId].gamble = false;
						gambleData[guildId].doubleOrNothing = false;
						delete gambleData[guildId].timer;
					}
				}
			}

		});

	Module::OnMessage(client, message);
}

void CurrencyModule::OnSave(QJsonDocument& doc) const 
{
	QJsonObject docObj;

	//! User Data
	for (auto server : guildList.keys()) 
	{
		QJsonObject serverJSON;
		
		for (auto user = guildList[server].begin(); user != guildList[server].end(); user++)
		{
			QJsonObject obj;
			obj["currency"] = (double)user->currency();
			obj["maxCurrency"] = (double)user->maxCurrency;
			obj["isDailyClaimed"] = user->isDailyClaimed;
			obj["dailyStreak"] = (int) user->dailyStreak;
			obj["numberOfDailysClaimed"] = (int) user->numberOfDailysClaimed;
			obj["numberOfGiveawaysClaimed"] = (int) user->numberOfGiveawaysClaimed;
			
			serverJSON[QString::number(user->userId)] = obj;
		}

		docObj[QString::number(server)] = serverJSON;
	}

	doc.setObject(docObj);
	

	//! Server Data (Config)
	QFile currenConfigfile("configs/" + currenConfigLoc + ".json");
	if (currenConfigfile.open(QFile::ReadWrite | QFile::Truncate)) 
	{
		QJsonDocument doc;
		QJsonObject serverList;
		for (auto server : serverCurrencyConfig.keys())
		{
			QJsonObject obj;
			
			auto config = serverCurrencyConfig[server];
			obj["name"] = config.currencyName;
			obj["symbol"] = config.currencySymbol;
			obj["freebieChannelId"] = QString::number(config.giveawayChannelId);
			obj["dailyReward"] = QString::number((double)config.dailyReward);
			obj["freebieReward"] = QString::number((double)config.freebieReward);
			obj["gambleLoss"] = QString::number((double)config.gambleLoss);
			obj["gambleReward"] = QString::number((double)config.gambleReward);
			obj["gambleMinGuess"] = QString::number(config.minGuess);
			obj["gambleMaxGuess"] = QString::number(config.maxGuess);
			obj["freebieProb"] = QString::number(config.randGiveawayProb);
			obj["freebieExpireTime"] = QString::number(config.freebieExpireTime);
			obj["dailyBonusAmount"] = QString::number((double)config.dailyBonusAmount);
			obj["dailyBonusPeriod"] = QString::number(config.dailyBonusPeriod);
			obj["stealSuccessChance"] = QString::number(config.stealSuccessChance);
			obj["stealFinePercent"] = QString::number(config.stealFinePercent);
			obj["stealVictimBonusPercent"] = QString::number(config.stealVictimBonusPercent);
			obj["stealFailedJailTime"] = QString::number(config.stealFailedJailTime);
			obj["bribeMaxAmount"] = QString::number((double)config.bribeMaxAmount);
			obj["bribeLeastAmount"] = QString::number((double)config.bribeLeastAmount);
			obj["bribeSuccessChance"] = QString::number(config.bribeSuccessChance);

			obj["lowRiskRewardStealSuccessChance"] = QString::number(config.lowRiskRewardStealSuccessChance);
			obj["highRiskRewardStealSuccessChance"] = QString::number(config.highRiskRewardStealSuccessChance);

			serverList[QString::number(server)] = obj;
			
		}
		doc.setObject(serverList);
		currenConfigfile.write(doc.toJson());
		currenConfigfile.close();
	}
}

void CurrencyModule::OnLoad(const QJsonDocument& doc) 
{
	QJsonObject docObj = doc.object();

	QStringList servers = docObj.keys();

	guildList.clear();

	//!User Data (Currency)
	for (auto server : servers) 
	{
		auto guildId = server.toULongLong();
		auto obj = docObj[server].toObject();
		QStringList users = obj.keys();

		QList<UserCurrency> list;
		for (auto user : users) {
			UserCurrency currencyData {
				user.toULongLong(),
				obj[user].toObject()["currency"].toDouble(),
				obj[user].toObject()["maxCurrency"].toDouble(),
				obj[user].toObject()["isDailyClaimed"].toBool(),
				(unsigned int) obj[user].toObject()["dailyStreak"].toInt(),
				(unsigned int) obj[user].toObject()["numberOfDailysClaimed"].toInt(),
				(unsigned int) obj[user].toObject()["numberOfGiveawaysClaimed"].toInt(),
				false,
			};

			list.append(currencyData);
		}
		guildList.insert(guildId, list);
		
	}
	QFile currenConfigfile("configs/" + currenConfigLoc + ".json");
	if (currenConfigfile.open(QFile::ReadOnly)) 
	{
		QJsonDocument d = QJsonDocument::fromJson(currenConfigfile.readAll());
		QJsonObject rootObj = d.object();

		serverCurrencyConfig.clear();
		auto servers = rootObj.keys();
		for (const auto& server : servers) {
			CurrencyConfig config;
			auto serverObj = rootObj[server].toObject();
			config.currencyName = serverObj["name"].toString();
			config.currencySymbol = serverObj["symbol"].toString();
			config.giveawayChannelId = serverObj["freebieChannelId"].toString().toULongLong();
			config.dailyReward = serverObj["dailyReward"].toString().toInt();
			config.freebieReward = serverObj["freebieReward"].toString().toInt();
			config.gambleLoss = serverObj["gambleLoss"].toString().toInt();
			config.gambleReward = serverObj["gambleReward"].toString().toInt();
			config.minGuess = serverObj["gambleMinGuess"].toString().toInt();
			config.maxGuess = serverObj["gambleMaxGuess"].toString().toInt();
			config.randGiveawayProb = serverObj["freebieProb"].toString().toDouble();
			config.freebieExpireTime = serverObj["freebieExpireTime"].toString().toUInt();
			config.dailyBonusAmount = serverObj["dailyBonusAmount"].toString("50").toUInt();
			config.dailyBonusPeriod = serverObj["dailyBonusPeriod"].toString("3").toUInt();
			config.stealSuccessChance = serverObj["stealSuccessChance"].toString("30").toUInt();
			config.stealFinePercent = serverObj["stealFinePercent"].toString("50").toUInt();
			config.stealVictimBonusPercent = serverObj["stealVictimBonusPercent"].toString("25").toUInt();
			config.stealFailedJailTime = serverObj["stealFailedJailTime"].toString("3").toUInt();
			config.bribeSuccessChance = serverObj["bribeSuccessChance"].toString("68").toUInt();
			config.bribeMaxAmount = serverObj["bribeMaxAmount"].toString("150").toInt();
			config.bribeLeastAmount = serverObj["bribeLeastAmount"].toString("20").toInt();

			config.highRiskRewardStealSuccessChance = serverObj["highRiskRewardStealSuccessChance"].toString("30").toInt();
			config.lowRiskRewardStealSuccessChance = serverObj["lowRiskRewardStealSuccessChance"].toString("50").toInt();

			auto guildId = server.toULongLong();
			serverCurrencyConfig.insert(guildId, config);
		}

		
		currenConfigfile.close();
	}
}
