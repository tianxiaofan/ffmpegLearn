/**************************************************************************
*   文件名      ：VideoRecode.h
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-12-4
*   邮   箱    ：499131808@qq.com
*   QQ    		：499131808
*   功能描述    ：
*   使用说明    ：
*   ======================================================================
*   修改者      ：
*   修改日期    ：
*   修改内容    ：
*   ======================================================================
*
***************************************************************************/

#pragma once
#include <QString>
#include <QThread>


class VideoRecord: public QThread
{
    Q_OBJECT
public:
    VideoRecord();

    void setRtspUrl(const QString& url) { m_rtspUrl = url; }
    void setSavePath(const QString& path) { m_savePath = path; }

    void run() override;

private:
    QString m_rtspUrl;
    QString m_savePath;
    int     file_sec = 20;
};

