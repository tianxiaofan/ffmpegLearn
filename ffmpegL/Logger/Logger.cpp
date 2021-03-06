﻿#include "Logger.h"
#include "LoggerTemplate.h"

#include <QCoreApplication>

#include <QDateTime>
#include <QDir>
#include <QMutex>
#include <QFile>

#include <string>
#ifdef Q_OS_WIN
#include <Windows.h>
#else
#include <cstdio>
#endif

namespace Logger
{
static QString gLogDir;
static int gLogMaxCount;

static void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);
static void outputMessageAsync(QtMsgType type, const QMessageLogContext& context, const QString& msg);
static void outputMessageAsyncConsole(QtMsgType type, const QMessageLogContext& context, const QString& msg);
void initLog(bool isfile,const QString &logPath, int logMaxCount, bool async)
{
    if (isfile)
    {
        if (async)
        {
            qInstallMessageHandler(outputMessageAsync);
        }
        else
        {
            qInstallMessageHandler(outputMessage);
        }
        gLogDir      = QCoreApplication::applicationDirPath() + "/" + logPath;
        gLogMaxCount = logMaxCount;
        QDir dir(gLogDir);
        if (!dir.exists())
        {
            dir.mkpath(dir.absolutePath());
        }
        QStringList infoList = dir.entryList(QDir::Files, QDir::Name);
        while (infoList.size() > gLogMaxCount)
        {
            dir.remove(infoList.first());
            infoList.removeFirst();
        }
    }
    else
    {
        qInstallMessageHandler(outputMessageAsyncConsole);
    }
}
static void outputMessageAsync(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static const QString messageTemp = QString("<div class=\"%1\">%2</div>\r\n");
    static const char typeList[] = { 'd', 'w', 'c', 'f', 'i' };
    static const QStringList typeString = { "Debug", "Warning", "Critical", "Fatal", "Info", "System" };
    static QMutex mutex;
    static QFile file;
    static QTextStream textStream;
    static uint count = 0;
    static const uint maxCount = 512;
    Q_UNUSED(context);
    QDateTime dt = QDateTime::currentDateTime();
    //每小时一个文件
    QString fileNameDt = dt.toString("yyyy-MM-dd_hh");

    //每分钟一个文件
    //QString fileNameDt = dt.toString("yyyy-MM-dd_hh_mm");

    QString contentDt = dt.toString("yyyy-MM-dd hh:mm:ss");
    QString message = QString("%1:%2 %3").arg(typeString.at(static_cast<int>(type))).arg(contentDt).arg(msg);
    QString htmlMessage = messageTemp.arg(typeList[static_cast<int>(type)]).arg(message);
    QString newfileName = QString("%1/%2_log.html").arg(gLogDir).arg(fileNameDt);
    mutex.lock();
    if (file.fileName() != newfileName)
    {
        if (file.isOpen())
        {
            file.close();
        }
        file.setFileName(newfileName);
        bool exist = file.exists();
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        textStream.setDevice(&file);
        textStream.setCodec("UTF-8");
        if (!exist)
        {
            textStream << logTemplate << "\r\n";
        }
    }
    textStream << htmlMessage;
    textStream.flush();
    count += static_cast<uint>(htmlMessage.length());
    if (count >= maxCount)
    {
        file.close();
        file.open(QIODevice::WriteOnly | QIODevice::Append);
    }
    mutex.unlock();
#ifdef Q_OS_WIN
    ::OutputDebugString(message.toStdWString().data());
    ::OutputDebugString(L"\r\n");
#else
    fprintf(stderr, message.toStdString().data());
#endif
}
static void outputMessageAsyncConsole(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(context);
    Q_UNUSED(msg);
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    static const QStringList typeString = {"Debug", "Warning", "Critical", "Fatal", "Info", "System"};
    QDateTime                dt         = QDateTime::currentDateTime();
    //每小时一个文件


    QString contentDt = dt.toString("yyyy-MM-dd hh:mm:ss");
    QString message   = QString("%1:%2 %3").arg(typeString.at(static_cast<int>(type))).arg(contentDt).arg(msg);
#ifdef Q_OS_WIN
    ::OutputDebugString(message.toStdWString().data());
    ::OutputDebugString(L"\r\n");
#else
    fprintf(stderr, message.toStdString().data());
    fprintf(stderr, "\r\n");
#endif
}
static void outputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    static QMutex mutex;
    static const QString messageTemp = QString("<div class=\"%1\">%2</div>\r\n");
    static const char typeList[] = { 'd', 'w', 'c', 'f', 'i' };

    Q_UNUSED(context);
    QDateTime dt = QDateTime::currentDateTime();

    //每小时一个文件
    QString fileNameDt = dt.toString("yyyy-MM-dd_hh");

    //每分钟一个文件
    //QString fileNameDt = dt.toString("yyyy-MM-dd_hh_mm");

    QString contentDt = dt.toString("yyyy-MM-dd hh:mm:ss");
    QString message = QString("%1 %2").arg(contentDt).arg(msg);
    QString htmlMessage = messageTemp.arg(typeList[static_cast<int>(type)]).arg(message);
    QFile file(QString("%1/%2_log.html").arg(gLogDir).arg(fileNameDt));
    mutex.lock();

    bool exist = file.exists();
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&file);
    textStream.setCodec("UTF-8");
    if (!exist)
    {
        textStream << logTemplate << "\r\n";
    }
    textStream << htmlMessage;
    file.close();
    mutex.unlock();
#ifdef Q_OS_WIN
    ::OutputDebugString(message.toStdWString().data());
    ::OutputDebugString(L"\r\n");
#else
    fprintf(stderr, message.toStdString().data());
#endif
}
} // namespace Logger
