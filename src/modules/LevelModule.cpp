#include "LevelModule.h"


LevelModule::LevelModule()
	: Module("levels", true)
{
	m_timer.setInterval(/*300 */ 1000);
	QObject::connect(&m_timer, &QTimer::timeout, 
		[this]()
	{
		for (auto& it = m_exp.begin(); it != m_exp.end(); it++)
		{
			for (GuildLevelData& data : it.value())
			{
				if (data.messageCount > 0) 
				{
					data.messageCount = 0;
					data.exp += qrand() % 31 + 15;
				}
			}
		}
	});
	m_timer.start();

	QTime now = QTime::currentTime();
	qsrand(now.msec());

	RegisterCommand("status", "retrieves the status", "[name]/[#number] \n[name] - optional, used to retrieve the status of somebody else\n\tEx: !status Timmy\n[#number] - optional, used to retrieve the status of a person at that rank\n\tEx: !status #2", 
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		client.getGuildMember(channel.guildId(), message.author().id()).then(
			[this, message, channel, &client](const Discord::GuildMember& member) 
		{
			Discord::Embed embed;
			QString url = "https://cdn.discordapp.com/avatars/" + QString::number(member.user().id()) + "/" + member.user().avatar() + ".png";
			embed.setAuthor(Discord::EmbedAuthor(member.nick(), url, url));
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Gay");

			GuildLevelData data = GetData(channel.guildId(), message.author().id());

			embed.setDescription("Experience: " + QString::number(data.exp));

			client.createMessage(message.channelId(), embed);
		});
	});

	RegisterCommand("top", "brief", "bleh",
		[this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel)
	{
		QStringList args = message.content().split(' ');
		if (args.size() == 2) {
			qSort(m_exp[channel.guildId()].begin(), m_exp[channel.guildId()].end(),
				[](const LevelModule::GuildLevelData& v1, const LevelModule::GuildLevelData& v2) -> bool
			{
				return v1.exp < v2.exp;
			});

			Discord::Embed embed;
			embed.setColor(qrand() % 16777216);
			embed.setTitle("Top " + args.back());

			QString desc = "";
			for (size_t i = 0; i < args.back().toUInt(); i++) 
			{
				LevelModule::GuildLevelData& curr = m_exp[channel.guildId()][i];
				desc += QString::number(i + 1) + ". ";

				client.getGuildMember(channel.guildId(), curr.user).then(
					[&desc](const Discord::GuildMember& member)
				{
					desc += member.nick();
				});

				desc += " - " + curr.exp;
			}

			embed.setDescription(desc);

			client.createMessage(message.channelId(), embed);
		}
	});
}

void LevelModule::OnSave(QJsonDocument& doc) const
{
	QJsonObject json;
	for (auto it = m_exp.begin(); it != m_exp.end(); it++) {
		QJsonObject level;

		for (const GuildLevelData& user : it.value()) {
			level[QString::number(user.user)] = user.exp;
		}
		json[QString::number(it.key())] = level;
	}

	doc.setObject(json);
}

void LevelModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject json = doc.object();

	QStringList guildIds = json.keys();

	for (const QString& guild : guildIds) 
	{
		snowflake_t guildId = guild.toULongLong();

		QJsonObject levels = json[guild].toObject();

		QStringList userids = levels.keys();

		for (const QString& user : userids)
			m_exp[guildId].append({ user.toULongLong(), levels[user].toInt(), 0 });
	}

}

void LevelModule::OnMessage(Discord::Client& client, const Discord::Message& message) const 
{
	Module::OnMessage(client, message);

	client.getChannel(message.channelId()).then(
		[this, message](const Discord::Channel& channel) 
	{
		for (GuildLevelData& data : m_exp[channel.guildId()]) {
			if (data.user == message.author().id()) {
				data.messageCount++;
				return;
			}
		}

		m_exp[channel.guildId()].append({ message.author().id(), 0, 1 });
	});
}

LevelModule::GuildLevelData LevelModule::GetData(snowflake_t guild, snowflake_t user)
{
	for (GuildLevelData data : m_exp[guild])
	{
		if (data.user == user) {
			return data;
		}
	}
	return { user, 0,0 };
}
