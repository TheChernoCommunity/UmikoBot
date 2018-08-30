#pragma once
#include <qstring.h>
#include <qvector.h>
#include <qfile.h>
#include <qmutex.h>
#include <qatomic.h>
#include <qthread.h>

// Logging Macros
#if defined(_UMIKO_DEBUG)
#define UStartLogger(file) Logger::StartLogger_(file)
#define ULog(severity, msg) Logger::Log_(severity, msg)
#define USetThreadName(name) Logger::SetThreadName_(name)
#define UStopLogger() Logger::StopLogger_()
#else
#define UStartLogger(file) 
#define USetThreadName(name) 
#define ULog(severity, msg) 
#define UStopLogger() 
#endif

template<typename T, typename... Args>
constexpr QString UFString(const QString& fm, T value, Args&&  ... d)
{
	return QString::asprintf(fm.toUtf8().constData(), value, std::forward<Args>(d)...);
}

namespace ulog
{
	enum Severity
	{
		Error, Warning, Debug
	};
}

class Logger : QThread
{
private:
#if defined(_UMIKO_DEBUG) 
	static Logger* logger;

	QFile m_logFile;
	unsigned int m_lineNumber;
	QVector<QString> m_logs;

	QMutex m_mutex;
	QAtomicInt m_flag;

	Logger(const QString& fileLocation);
protected:
	void run();
#endif
public:

#if defined(_UMIKO_DEBUG)
	static void StartLogger_(const QString& file_loc);
	static void SetThreadName_(const QString& str);
	static void StopLogger_();

	static void Log_(ulog::Severity s, const QString&fm);
#endif
};

