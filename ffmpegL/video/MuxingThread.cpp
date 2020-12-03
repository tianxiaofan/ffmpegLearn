/**************************************************************************
*   文件名      ：MuxingThread.cpp
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-12-2
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
#include "MuxingThread.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

using namespace std;

MuxingThread::MuxingThread()
{

}

bool MuxingThread::open(const char*        url,
                        AVCodecParameters* video_para,
                        AVRational*        video_time_base,
                        AVCodecParameters* audio_para,
                        AVRational*        audio_time_base)
{
    auto c = remux.open(url, video_para, audio_para);
    if (!c)
        return false;
    remux.setContext(c);
    remux.set_src_video_timebase(video_time_base);
    remux.set_src_audio_timebase(audio_time_base);
    return true;
}

void MuxingThread::threadRun()
{
    remux.writeHead();
    while (!m_isExit)
    {
        unique_lock<mutex> lock(m_reRemux);
        auto               pkt = m_pktList.pop();
        if (!pkt)
        {
            customSleep(1);
            continue;
        }
        remux.writeAVPacket(pkt);
        av_packet_free(&pkt);
    }
    remux.writeEnd();
    remux.setContext(nullptr);
}

void MuxingThread::todo(AVPacket *pkt)
{
    m_pktList.push(pkt);
    next(pkt);
}
