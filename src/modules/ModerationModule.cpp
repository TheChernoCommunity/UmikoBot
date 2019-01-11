#include "ModerationModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

Q_CONSTEXPR char g_addBannedKeyword[] = "addbanword";
Q_CONSTEXPR char g_removeBannedKeyword[] = "rmbanword";
Q_CONSTEXPR char g_listBannedKeywords[] = "listbanwords";
Q_CONSTEXPR char g_setBannedKeywordMinLength[] = "setbanwordminlen";
Q_CONSTEXPR int g_bannedKeywordAbsoluteMinimumLength = 4;

ModerationModule::ModerationModule()
	: Module("moderation", true)
{
	RegisterCommand(Commands::MODERATION_ADD_BANNABLE_KEYWORD, g_addBannedKeyword,
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
		{
			Permissions::MatchesPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[this, &client, channel, message](bool hasPermission)
				{
					if (!hasPermission)
					{
						client.createMessage(channel.id(), "You don't have the permission to use that command");
						return;
					}

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
					if (keywordText.size() < settings.bannedKeywordMinLength)
					{
						client.createMessage(channel.id(), QString("The word '%1' is too short to be banned! Consider using the '%2' command to lower the limit.").arg(keywordText).arg(g_setBannedKeywordMinLength));
						return;
					}

					BannedKeyword kw{};
					kw.text = keywordText;
					kw.deleteMessageDays = deleteMessageDays;
					settings.bannedKeywords.append(kw);
				}
			);
		}
	);

	RegisterCommand(Commands::MODERATION_REMOVE_BANNABLE_KEYWORD, g_removeBannedKeyword,
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
		{
			Permissions::MatchesPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[this, &client, channel, message](bool hasPermission)
				{
					if (!hasPermission)
					{
						client.createMessage(channel.id(), "You don't have the permission to use that command");
						return;
					}

					Settings& settings = m_settings[channel.guildId()];

					QStringList args = message.content().split(' ');
					if (args.size() < 2)
					{
						client.createMessage(channel.id(), "Too few arguments!");
						return;
					}

					const QString& keywordText = args[1];
					for (int i = 0; i < settings.bannedKeywords.size(); ++i)
					{
						if (settings.bannedKeywords[i].text == keywordText)
						{
							settings.bannedKeywords.removeAt(i);
							client.createMessage(channel.id(), QString("Successfully removed '%1' from banned keywords.").arg(keywordText));
							return;
						}
					}

					client.createMessage(channel.id(), QString("'%1' is not a banned keyword.").arg(keywordText));
				}
			);
		}
	);

	RegisterCommand(Commands::MODERATION_LIST_BANNABLE_KEYWORDS, g_listBannedKeywords,
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
		{
			Permissions::MatchesPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[this, &client, channel, message](bool hasPermission)
				{
					if (!hasPermission)
					{
						client.createMessage(channel.id(), "You don't have the permission to use that command");
						return;
					}

					Settings& settings = m_settings[channel.guildId()];

					QString description;
					Q_FOREACH(const BannedKeyword& keyword, settings.bannedKeywords)
					{
						description.append(keyword.text).append('\n');
					}

					Discord::Embed embed;
					embed.setColor(0xffffff);
					embed.setDescription(description);
					embed.setTitle("Banned Words");
					client.createMessage(channel.id(), embed);
				}
			);
		}
	);

	RegisterCommand(Commands::MODERATION_SET_BANNABLE_KEYWORD_MINIMUM_LENGTH, g_setBannedKeywordMinLength,
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
		{
			Permissions::MatchesPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
				[this, &client, channel, message](bool hasPermission)
				{
					if (!hasPermission)
					{
						client.createMessage(channel.id(), "You don't have the permission to use that command");
						return;
					}

					Settings& settings = m_settings[channel.guildId()];

					QStringList args = message.content().split(' ');
					if (args.size() < 2)
					{
						client.createMessage(channel.id(), "Too few arguments!");
						return;
					}

					bool ok = false;
					int bannedKeywordMinimumLength = args[1].toInt(&ok);
					if (!ok || bannedKeywordMinimumLength < g_bannedKeywordAbsoluteMinimumLength || bannedKeywordMinimumLength > 2000)
					{
						client.createMessage(channel.id(), "First argument (banned-keyword-maximum-length) must be an integer between 4 and 2000.");
						return;
					}

					settings.bannedKeywordMinLength = bannedKeywordMinimumLength;
				}
			);
		}
	);
}

void ModerationModule::OnMessage(Discord::Client& client, const Discord::Channel& channel, const Discord::Message& message)
{
	Q_FOREACH(const BannedKeyword& keyword, m_settings[channel.guildId()].bannedKeywords)
	{
		// Guard in case a tiny keyword snuck its way in
		if (keyword.text.size() < g_bannedKeywordAbsoluteMinimumLength)
			continue;

		if (message.content().contains(keyword.text))
		{
			client.deleteMessage(channel.id(), message.id());
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
		obj["banned_keyword_min_length"] = it->bannedKeywordMinLength;

		QJsonArray bannedKeywordsArray;
		Q_FOREACH(const BannedKeyword& keyword, it->bannedKeywords)
		{
			QJsonObject keywordObj;
			keywordObj["text"] = keyword.text;
			keywordObj["delete_message_days"] = keyword.deleteMessageDays;
			bannedKeywordsArray.append(keywordObj);
		}

		obj["banned_keywords"] = bannedKeywordsArray;

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
		settings.bannedKeywordMinLength = obj.value("banned_keyword_min_length").toInt(g_bannedKeywordAbsoluteMinimumLength);

		const QJsonArray keywordsArray = obj.value("banned_keywords").toArray();
		for (QJsonValue keyword : keywordsArray)
		{
			QJsonObject keywordObj = keyword.toObject();

			BannedKeyword kw{};
			kw.text = keywordObj.value("text").toString();
			kw.deleteMessageDays = keywordObj.value("delete_message_days").toInt(-1);

			// Guard in case the saving might have gone wrong
			if (kw.text.isEmpty() || kw.deleteMessageDays < 0)
				continue;

			settings.bannedKeywords.append(kw);
		}
	}
}
