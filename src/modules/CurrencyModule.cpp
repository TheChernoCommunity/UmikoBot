#include "CurrencyModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/qregexp.h>

#define giveawayDeclLoc QString("giveawayAnnouncementId")
#define currenConfigLoc QString("currencyConfig")

CurrencyModule::CurrencyModule(UmikoBot* client)
	: Module("currency", true) {

	m_timer.setInterval(84600 * 1000); //!24hrs = 84600 seconds
	QObject::connect(&m_timer, &QTimer::timeout, [this, client]() {
		
		//!Clear the daily bonus for everyone
		for (auto it = m_settings.begin(); it != m_settings.end(); ++it) {
			it->isDailyClaimed = false;
		}

		if (!isRandomGiveAwayDone) {
			client->createMessage(giveawayChannelId, "@everyone Today's freebie expires in **10 seconds**. `!claim` it now!");

			allowGiveaway = true;

			QTimer timer;
			timer.setInterval(10 * 1000);
			QObject::connect(&timer, &QTimer::timeout, 
				[this, client]() {
					if(isRandomGiveAwayDone)
						isRandomGiveAwayDone = false;

				});
		}
	});

	m_timer.start();

	RegisterCommand(Commands::CURRENCY_WALLET, "wallet", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) {
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "wallet")
			return;
		if (args.size() > 1) {
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}
		if (args.size() == 1) {
			Discord::Embed embed;
			embed.setColor(11777216);
			QString creditScore = QString::number(m_settings[setting->id].currency);
			QString desc = "**Current Credits: ** `" + creditScore + "` **" + currencySymbol + "** (" + currencyName +")";
			embed.setTitle("Your Wallet");
			embed.setDescription(desc);
			client.createMessage(message.channelId(), embed);
		}
		});

	RegisterCommand(Commands::CURRENCY_DAILY, "daily", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) {

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "daily")
			return;
		if (args.size() > 1) {
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}
		if (m_settings[setting->id].isDailyClaimed) {

			int remainingTime = m_timer.remainingTime();

			QTime conv(0, 0, 0);
			conv = conv.addMSecs(remainingTime);
			QString time = QString::number(conv.hour()) + "(hrs) " + QString::number(conv.minute()) + "(mins) " + QString::number(conv.second()) + "(secs)";
			QString desc = "**You have already claimed your daily credits.**\nCome back after `" + time + "` to get more rich!";

			client.createMessage(message.channelId(), desc);
		}
		else {
			m_settings[setting->id].isDailyClaimed = true;
			m_settings[setting->id].currency += 100;

			client.createMessage(message.channelId(), "**You now have 100 more " + currencyName + " in your wallet!**");
		}

		});

	RegisterCommand(Commands::CURRENCY_GAMBLE, "gamble", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) {
		
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;
		
		if (args.first() != prefix + "gamble")
			return;

		if (args.size() > 1) {
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 1) { //! Gamble by self

			if (selfGambleData.gamble) {
				Discord::Embed embed;
				embed.setColor(qrand() % 16777216);
				embed.setTitle("Welcome to Gamble!");
				embed.setDescription("**Sorry but this feature is currently in use by another person. Please try again later!**");
				client.createMessage(message.channelId(), embed);
			}

			selfGambleData.randNum = qrand() % 6;
			selfGambleData.channelId = message.channelId();
			selfGambleData.gamble = true;
			selfGambleData.userId = message.author().id();
			selfGambleData.guildId = channel.guildId();

			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Welcome to Gamble!");
			embed.setDescription("All you need to do is guess a random number between 0 and 5 (inclusive) and if it is the same as the number I guess, you get **50" + currencySymbol + "**!\n\n**What number do you think of?**");
			
			client.createMessage(message.channelId(), embed);
		}

		});

	RegisterCommand(Commands::CURRENCY_CLAIM, "claim", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) {

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "claim")
			return;

		if (args.size() > 1) {
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 1) {
			if (!isRandomGiveAwayDone) {
				if (allowGiveaway) {
					Discord::Embed embed;
					embed.setColor(qrand() % 11777216);
					embed.setTitle("Claim FREEBIE");
					embed.setDescription(":drum: And today's FREEBIE goes to **" + message.author().username() + "**! \n\n Congratulations! You just got **300" + currencySymbol +"**!");

					m_settings[setting->id].currency += 300;

					client.createMessage(message.channelId(), embed);
					isRandomGiveAwayDone = true;
				}
				else {
					client.createMessage(message.channelId(), "**BRUH**, ***yOu CaN't JuSt GeT fReE sTuFf aNyTiMe.***");
				}
			}
			else {
				Discord::Embed embed;
				embed.setColor(qrand()%11777216);
				embed.setTitle("Claim FREEBIE");
				embed.setDescription("Sorry, today's freebie has been claimed (or it expired) :cry: \n\n But you can always try again the next day!");
				client.createMessage(message.channelId(), embed);
			}
		}

		});

	RegisterCommand(Commands::CURRENCY_SET_PRIZE_CHANNEL, "setannouncechan", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) {

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "setannouncechan")
			return;

		if (args.size() > 1) {
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 1) {
			giveawayChannelId = message.channelId();
			client.createMessage(message.channelId(), "**Giveaway announcement channel successfully changed to current channel.**");
		}

		});

	RegisterCommand(Commands::CURRENCY_SET_NAME, "setcurrenname", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) {
		
		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "setcurrenname")
			return;

		if (args.size() > 2) {
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) {
			currencyName = args.at(1);
			client.createMessage(message.channelId(), "**Currency Name set to** " + currencyName);
		}

		});

	RegisterCommand(Commands::CURRENCY_SET_SYMBOL, "setcurrensymb", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) {

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;

		if (args.first() != prefix + "setcurrensymb")
			return;

		if (args.size() > 2) {
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		if (args.size() == 2) {
			currencySymbol = args.at(1);
			client.createMessage(message.channelId(), "**Currency Symbol set to** " + currencySymbol);
		}

		});
}

void CurrencyModule::OnMessage(Discord::Client& client, const Discord::Message& message) {

	if (!isRandomGiveAwayDone) {
		static int pingCount = 0;
		if (pingCount == 0) {
			bool outcome = randp(random_engine);
			if (outcome) {
				client.createMessage(giveawayChannelId, "@everyone **FREEBIE** available now! Go `!claim` some juicy coins!");
				allowGiveaway = true;
				pingCount++;
			}
		}
	}

	if (selfGambleData.gamble && !message.author().bot() && message.channelId() == selfGambleData.channelId) {
		if (message.author().id() == selfGambleData.userId) {

			//Check if the message is a number
			QRegExp re("\\d*");
			if (!re.exactMatch(message.content()))
				return;

			GuildSetting* setting = &GuildSettings::GetGuildSetting(selfGambleData.guildId);
			if (message.content().toInt() > 5 || message.content().toInt() < 0) {

				client.createMessage(message.channelId(), "**Your guess is out of range!** \nTry a number between 0 and 5 (inclusive)");
				return;

			}
			if (message.content().toInt() == selfGambleData.randNum) {
				m_settings[setting->id].currency += 50;
				client.createMessage(message.channelId(), "**You guessed CORRECTLY!**\n(**50" + currencySymbol  + "** have been added to your wallet!)");
			}
			else {
				m_settings[setting->id].currency -= 10;
				client.createMessage(message.channelId(), "**Better Luck next time!**\n*(psst! I took **10CC** from your wallet for my time...)*");
			}
		}
		selfGambleData.gamble = false;
	}

	Module::OnMessage(client, message);
}

void CurrencyModule::OnSave(QJsonDocument& doc) const {
	QJsonObject docObj;

	for (auto it = m_settings.begin(); it != m_settings.end(); ++it) {
		QJsonObject obj;
		obj["currency"] = it->currency;
		obj["isDailyClaimed"] = it->isDailyClaimed;

		docObj[QString::number(it.key())] = obj;
	}

	doc.setObject(docObj);

	QFile file("configs/" + giveawayDeclLoc + ".json");
	if (file.open(QFile::ReadWrite)) {
		QJsonDocument doc;
		QJsonObject obj;
		obj["channelId"] = QString::number(giveawayChannelId);
		doc.setObject(obj);
		file.write(doc.toJson(QJsonDocument::Indented));
		file.close();
	}

	QFile currenConfigfile("configs/" + currenConfigLoc + ".json");
	if (currenConfigfile.open(QFile::ReadWrite)) {
		QJsonDocument doc;
		QJsonObject obj;
		obj["name"] = currencyName;
		obj["symbol"] = currencySymbol;
		doc.setObject(obj);
		currenConfigfile.write(doc.toJson());
		currenConfigfile.close();
	}
}

void CurrencyModule::OnLoad(const QJsonDocument& doc) {
	QJsonObject docObj = doc.object();

	for (auto it = docObj.begin(); it != docObj.end(); ++it) {
		const QJsonObject obj = it.value().toObject();
		Setting& setting = m_settings[it.key().toULongLong()];
		setting.currency = obj["currency"].toDouble();
		setting.isDailyClaimed = obj["isDailyClaimed"].toBool();
	}

	QFile file("configs/" + giveawayDeclLoc +".json");
	if (file.open(QFile::ReadOnly)) {
		QJsonDocument d = QJsonDocument::fromJson(file.readAll());
		QJsonObject root_object = d.object();
		QJsonValue id = root_object.value("channelId");
		giveawayChannelId = id.toString().toULongLong();
	}

	QFile currenConfigfile("configs/" + currenConfigLoc + ".json");
	if (currenConfigfile.open(QFile::ReadOnly)) {
		QJsonDocument d = QJsonDocument::fromJson(currenConfigfile.readAll());
		QJsonObject root_object = d.object();
		
		//Name
		currencyName = root_object["name"].toString();

		//Symbol
		currencySymbol = root_object["symbol"].toString();
	}
}