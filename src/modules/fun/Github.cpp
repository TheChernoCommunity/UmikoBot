#include "modules/fun/FunModule.h"

#include <QtNetwork>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <random>

using namespace Discord;

void FunModule::initiateGithub()
{
    QObject::connect(&m_GithubManager, &QNetworkAccessManager::finished,
	                this, [this](QNetworkReply* reply)
    {
		auto& client = UmikoBot::Instance();

		if (reply->error())
        {
			qDebug() << reply->errorString();
			client.createMessage(m_GithubChannel, reply->errorString());

			return;
		}

		QString in = reply->readAll();

		QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());
		auto obj = doc.object();

		QJsonArray items = obj["items"].toArray();

		std::random_device device;
		std::mt19937 rng(device());
		std::uniform_int_distribution<std::mt19937::result_type> dist(0, items.size());

		QJsonObject repo = items[dist(rng)].toObject();

		QString repo_fullname = repo["full_name"].toString();
		QString repo_url = repo["html_url"].toString();
		QString repo_language = repo["language"].toString();
		int repo_stars = repo["stargazers_count"].toInt();

		Embed embed;
		embed.setTitle(repo_fullname);
		embed.setDescription("\nStars: " + QString::number(repo_stars) +
							 "\nLanguage: " + repo_language + "\n" +
							 repo_url);

		client.createMessage(m_GithubChannel, embed);
	});

	RegisterCommand(Commands::FUN_GITHUB, "github", [this](Client& client, const Message& message, const Channel& channel) 
	{
		QStringList args = message.content().split(' ');

		m_GithubChannel = channel.id();

		if(args.size() != 1)
		{
			UmikoBot::Instance().createMessage(m_GithubChannel, "**Wrong Usage of Command!**");

			return;
		}

		std::random_device device;
		std::mt19937 rng(device());
		std::uniform_int_distribution<std::mt19937::result_type> ch('A', 'Z');

		m_GithubManager.get(QNetworkRequest(QUrl("https://api.github.com/search/repositories?q=" + QString(QChar((char)ch(rng))))));
	});
}
