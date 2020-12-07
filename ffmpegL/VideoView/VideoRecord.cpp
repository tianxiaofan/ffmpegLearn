/**************************************************************************
*   文件名      ：VideoRecode.cpp
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
#include "VideoRecord.h"
#include "DemuxThread.h"
#include "MuxingThread.h"
#include "Logger.h"

#include <chrono>
#include <iomanip>
#include <sstream>


/**
 * @brief getFileName 生成视频文件名
 * @return
 */
static QString getFileName(const QString& path)
{
    //c++11 的时间方式实现,也可用Qt的方式,
    std::stringstream ss;
    auto              t        = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto              time_str = std::put_time(std::localtime(&t), "%Y_%m_%d_%H_%M_%S");
    ss << path.toLatin1().data() << "/video_" << time_str << ".mp4";
    LOG_INFO << ss.str().c_str();
    return ss.str().c_str();
}

VideoRecord::VideoRecord()
{

}

void VideoRecord::run()
{
    DemuxThread  demux;
    MuxingThread remux;
    if (m_rtspUrl.isEmpty())
        return;
    while (!isInterruptionRequested())
    {
        LOG_INFO << m_rtspUrl.toLatin1().data();
        if (demux.open(m_rtspUrl.toLatin1().data()))
        {
            break;
        }
        msleep(10);
        continue;
    }

    demux.start();

    //音视频参数
    auto vpara = demux.copyVideoPara();
    if (!vpara)
    {
        // 需清理demux资源
        LOG_INFO << "copyVideo error!";
        demux.stop();
        return;
    }
    auto apara = demux.copyAudioPara();
    if (!remux.open(getFileName(m_savePath).toLatin1().data(), vpara->para, vpara->timeBase))
    {
        LOG_INFO << "remux error!";
        demux.stop();
        return;
    }
    demux.setNext(&remux);
    remux.start();
    auto cur = getNowMs();
    while (!isInterruptionRequested())
    {
        if (getNowMs() - cur > file_sec * 1000)
        {
            cur = getNowMs();
            remux.stop(); //停止封装
            if (!remux.open(getFileName(m_savePath).toLatin1().data(), vpara->para, vpara->timeBase))
            {
                LOG_INFO << "remux error!";
                break;
            }
            remux.start();
        }
    }
    demux.stop();
    remux.stop();
}
