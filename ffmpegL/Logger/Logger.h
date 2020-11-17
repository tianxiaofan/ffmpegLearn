#pragma once
#include <QDebug>
#include <atomic>
#include <chrono>
#include <thread>
#include <sstream>
namespace Logger
{
using namespace std;
static atomic_int gCount(0);

static QString currentThreadId()
{

    stringstream ss;
    ss << this_thread::get_id();
    return QString::fromStdString(ss.str());
}
#define LOG_DEBUG qDebug() << __FILE__ << __FUNCTION__ << __LINE__
#define LOG_INFO qInfo() << __FILE__ << __FUNCTION__ << __LINE__
#define LOG_WARN qWarning() << __FILE__ << __FUNCTION__ << __LINE__
#define LOG_CRIT qCritical() << __FILE__ << __FUNCTION__ << __LINE__

#define LOG_DEBUG_THREAD qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << Logger::currentThreadId()
#define LOG_INFO_THREAD qInfo() << __FILE__ << __FUNCTION__ << __LINE__ << Logger::currentThreadId()
#define LOG_WARN_THREAD qWarning() << __FILE__ << __FUNCTION__ << __LINE__ << Logger::currentThreadId()
#define LOG_CRIT_THREAD qCritical() << __FILE__ << __FUNCTION__ << __LINE__ << Logger::currentThreadId()

void initLog(bool isFile = false, const QString& logPath = QStringLiteral("Log"), int logMaxCount = 1024, bool async = true);

} // namespace Logger
