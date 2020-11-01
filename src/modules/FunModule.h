#pragma once
// Fun and Utilities!
#include "core/Module.h"
#include "UmikoBot.h"
#include "QtCore/QMap"
#include <memory>
#include <qtimer.h>
#include <Discord/Patches/MessagePatch.h>
#include <QtCore/QtMath>
#include <core/Utility.h>
#include <QtCore/QRegExp>

class FunModule : public Module, public QObject
{
private:
	struct PollOption {
		QString emote = "";
		QString desc = "";
		bool isAnimated = false;
		std::size_t count = 0;
	};
	using PollOptions = QList<PollOption>;

	struct PollSettings;
	using Poll = std::shared_ptr<PollSettings>;
	using ServerPolls = std::shared_ptr<QList<Poll>>;
	using Polls = QMap<snowflake_t, ServerPolls>;

	struct PollSettings {
		PollOptions options;
		//Default: Finish only when the timer ends
		long long maxVotes;
		snowflake_t notifChannel;
		snowflake_t pollMsg;
		QString pollName;
		int pollNum;
		std::shared_ptr<QTimer> timer;
		ServerPolls polls;

		PollSettings(const PollOptions& op, long long maxvotes, snowflake_t chan, const QString& name, int num, double time, const ServerPolls& pollList, snowflake_t msg);
	};
	Polls m_polls;

	//! A list of roles (for each server) that have been given poll 
	//! creation access
	QMap<snowflake_t, QList<snowflake_t>> m_pollWhitelist;
	QNetworkAccessManager m_MemeManager;
	snowflake_t m_memeChannel;
	UmikoBot* m_client;

	void pollReactAndAdd(const PollOptions& options, int pos, const Poll& poll, snowflake_t msg, snowflake_t chan, snowflake_t guild);

public:
	FunModule(UmikoBot* client);

	void onReact(snowflake_t user, snowflake_t channel, snowflake_t message, const Discord::Emoji& emoji) const;
	void onUnReact(snowflake_t user, snowflake_t channel, snowflake_t message, const Discord::Emoji& emoji) const;

	void OnMessage(Discord::Client& client, const Discord::Message& message) override;

	void OnSave(QJsonDocument& doc) const override;
	void OnLoad(const QJsonDocument& doc) override;
};