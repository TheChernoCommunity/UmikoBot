#include "ModerationModule.h"
#include "UmikoBot.h"

#include <QStringView>

Q_CONSTEXPR char g_addBannableKeyword[] = "addbanword";
Q_CONSTEXPR char g_removeBannableKeyword[] = "rmbanword";
Q_CONSTEXPR char g_listBannableKeywords[] = "listbanwords";
Q_CONSTEXPR char g_setBannableKeywordMinLength[] = "setbanwordminlen";
Q_CONSTEXPR int g_bannableKeywordAbsoluteMinimumLength = 4;

ModerationModule::ModerationModule()
	: Module("moderation", true)
{
	RegisterCommand(Commands::MODERATION_ADD_BANNABLE_KEYWORD, g_addBannableKeyword,
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
		{
			Settings& settings = m_settings[channel.guildId()];

			QStringList args = message.content().split(' ');
			if (args.size() < 3)
			{
				client.createMessage(channel.id(), "Too few arguments!");
				return;
			}

			bool ok = false;
			int deleteMessageDays = args[1].toInt(&ok);
			if (!ok || deleteMessageDays < 0 || deleteMessageDays > 7)
			{
				client.createMessage(channel.id(), "First argument (delete-message-days) must be an integer between 0 and 7");
				return;
			}

			const QString& keywordText = args[2];
			if (keywordText.size() < settings.bannableKeywordMinLength)
			{
				client.createMessage(channel.id(), QString("The word '%1' is too short to be bannable! Consider using the '%2' command to lower the limit.").arg(keywordText).arg(g_setBannableKeywordMinLength));
				return;
			}

			BannableKeyword kw{};
			kw.text = keywordText;
			kw.deleteMessageDays = deleteMessageDays;
			settings.bannableKeywords.append(kw);
		}
	);

	RegisterCommand(Commands::MODERATION_REMOVE_BANNABLE_KEYWORD, g_removeBannableKeyword,
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
		{
			Settings& settings = m_settings[channel.guildId()];

			QStringList args = message.content().split(' ');
			if (args.size() < 2)
			{
				client.createMessage(channel.id(), "Too few arguments!");
				return;
			}

			const QString& keywordText = args[1];
			for (int i = 0; i < settings.bannableKeywords.size(); ++i)
			{
				if (settings.bannableKeywords[i].text == keywordText)
				{
					settings.bannableKeywords.removeAt(i);
					client.createMessage(channel.id(), QString("Successfully removed '%1' from bannable keywords.").arg(keywordText));
					return;
				}
			}

			client.createMessage(channel.id(), QString("'%1' is not a bannable keyword.").arg(keywordText));
		}
	);

	RegisterCommand(Commands::MODERATION_LIST_BANNABLE_KEYWORDS, g_listBannableKeywords,
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
		{
			Settings& settings = m_settings[channel.guildId()];

			QString description;
			Q_FOREACH(const BannableKeyword& keyword, settings.bannableKeywords)
			{
				description.append(keyword.text).append('\n');
			}

			Discord::Embed embed;
			embed.setColor(0xffffff);
			embed.setDescription(description);
			embed.setTitle("Bannable Words");
			client.createMessage(channel.id(), embed);
		}
	);

	RegisterCommand(Commands::MODERATION_SET_BANNABLE_KEYWORD_MINIMUM_LENGTH, g_setBannableKeywordMinLength,
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
		{
			Settings& settings = m_settings[channel.guildId()];

			QStringList args = message.content().split(' ');
			if (args.size() < 2)
			{
				client.createMessage(channel.id(), "Too few arguments!");
				return;
			}

			bool ok = false;
			int bannableKeywordMinimumLength = args[1].toInt(&ok);
			if (!ok || bannableKeywordMinimumLength < g_bannableKeywordAbsoluteMinimumLength || bannableKeywordMinimumLength > 2000)
			{
				client.createMessage(channel.id(), "First argument (bannable-keyword-maximum-length) must be an integer between 4 and 2000.");
				return;
			}

			settings.bannableKeywordMinLength = bannableKeywordMinimumLength;
		}
	);
}

void ModerationModule::OnMessage(Discord::Client& client, const Discord::Channel& channel, const Discord::Message& message)
{
	Q_FOREACH(const BannableKeyword& keyword, m_settings[channel.guildId()].bannableKeywords)
	{
		// Guard in case a tiny keyword snuck its way in
		if (keyword.text.size() < g_bannableKeywordAbsoluteMinimumLength)
			continue;

		if (message.content().contains(keyword.text))
		{
			const QString reason = QString("User sent a message containing the banned keyword '%1'").arg(keyword.text);
			client.createGuildBan(channel.guildId(), message.author().id(), keyword.deleteMessageDays, reason);
		}
	}

	Module::OnMessage(client, channel, message);
}

void ModerationModule::OnSave(QJsonDocument& doc) const
{
	QJsonObject docObj;

	for (auto it = m_settings.begin(); it != m_settings.end(); ++it)
	{
		QJsonObject obj;
		obj["bannable_keyword_min_length"] = it->bannableKeywordMinLength;

		QJsonArray bannableKeywordsArray;
		Q_FOREACH(const BannableKeyword& keyword, it->bannableKeywords)
		{
			QJsonObject keywordObj;
			keywordObj["text"] = keyword.text;
			keywordObj["delete_message_days"] = keyword.deleteMessageDays;
			bannableKeywordsArray.append(keywordObj);
		}

		obj["bannable_keywords"] = bannableKeywordsArray;

		docObj[QString::number(it.key())] = obj;
	}

	doc.setObject(docObj);
}

void ModerationModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject docObj = doc.object();

	for (auto it = docObj.begin(); it != docObj.end(); ++it)
	{
		const QJsonObject obj = it.value().toObject();
		Settings& settings = m_settings[it.key().toULongLong()];
		settings.bannableKeywordMinLength = obj.value("bannable_keyword_min_length").toInt(g_bannableKeywordAbsoluteMinimumLength);

		const QJsonArray keywordsArray = obj.value("bannable_keywords").toArray();
		for (QJsonValue keyword : keywordsArray)
		{
			QJsonObject keywordObj = keyword.toObject();

			BannableKeyword kw{};
			kw.text = keywordObj.value("text").toString();
			kw.deleteMessageDays = keywordObj.value("delete_message_days").toInt(-1);

			// Guard in case the saving might have gone wrong
			if (kw.text.isEmpty() || kw.deleteMessageDays < 0)
				continue;

			settings.bannableKeywords.append(kw);
		}
	}
}
