#include "Logger.h"

#include <qtimezone.h>
#include <qdebug.h>

#if defined(_UMIKO_DEBUG)

Logger* Logger::logger = nullptr;

Logger::Logger(const QString& fileLocation)
{
	m_logFile.setFileName(fileLocation);
	assert(m_logFile.open(QIODevice::Append));
	m_lineNumber = 1;
	m_logs.clear();
	m_flag = true;
}

void Logger::run()
{
	do
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		if (m_logs.size())
		{
			if (!m_mutex.try_lock_for(std::chrono::microseconds(50)))
				continue;
			Q_FOREACH(QString log, m_logs)
			{
				m_logFile.write(log.toUtf8());
				qDebug("%s", log.toUtf8().constData());
			}
			m_logs.clear();
			m_mutex.unlock();
		}
	} while (m_flag.testAndSetRelease(true, true) || m_logs.size());
	m_logFile.close();
}

void Logger::StartLogger_(const QString& file_loc)
{
	if (!logger)
		logger = new Logger(file_loc);
	logger->start();
}

void Logger::SetThreadName_(const QString & str)
{
	QThread::currentThread()->setObjectName(str);
}

void Logger::StopLogger_()
{
	logger->m_flag = false;
	logger->quit();
	logger->wait();
	delete logger;
	logger = nullptr;
}

void Logger::Log_(ulog::Severity s, const QString & fm)
{
	QString log = QString::number(logger->m_lineNumber++) + "> ";
	log.append(QTime::currentTime().toString("hh:mm:ss"));
	log.append(" Thread: " + QThread::currentThread()->objectName());

	switch (s)
	{
	case ulog::Severity::Error:
		log.append(" [ERROR]    "); break;
	case ulog::Severity::Warning:
		log.append(" [WARNING]  "); break;
	case ulog::Severity::Debug:
		log.append(" [DEBUG]    "); break;
	}

	log.append(fm);
	//std::lock_guard<std::timed_mutex> lock(logger->m_mutex);
	QMutexLocker lock(&logger->m_mutex);
	logger->m_logs.push_back(log);
	//logger->m_mutex.unlock();
}
#endif