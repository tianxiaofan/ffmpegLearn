/**************************************************************************
*   文件名      ：DecodecThread.cpp
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
#include "DecodecThread.h"
#include "Logger.h"

extern "C"
{
#include <libavformat/avformat.h>
}

using namespace std;

void DecodecThread::todo(AVPacket* pkt)
{
    printf("#");
    if (!pkt || pkt->stream_index != 0)
    {
        return;
    }
    m_pktList.push(pkt);
}

bool DecodecThread::open(AVCodecParameters* para)
{
    if (!para)
    {
        LOG_WARN << "para is nullptr!";
        return false;
    }
    unique_lock<mutex> lock(m_decMutex);
    auto               c = m_decodec.create(para->codec_id, false);
    if (!c)
    {
        LOG_WARN << "decodec create: error";
        return false;
    }
    avcodec_parameters_to_context(c, para);
    m_decodec.setContext(c);
    if (!m_decodec.open())
    {
        LOG_WARN << "decodec open: error";
        return false;
    }
    LOG_INFO << "decodec open: success!";
    return true;
}

void DecodecThread::threadRun()
{
    {
        unique_lock<mutex> lock(m_decMutex);
        if (!m_frame)
            m_frame = av_frame_alloc();
    }
    while (!m_isExit)
    {
        auto pkt = m_pktList.pop();
        if (!pkt)
        {
            this_thread::sleep_for(1ms);
            continue;
        }
        bool re = m_decodec.send(pkt);
        av_packet_free(&pkt);
        if (!re)
        {
            this_thread::sleep_for(1ms);
            continue;
        }
        {
            unique_lock<mutex> lock(m_decMutex);
            if (m_decodec.recv(m_frame))
            {
                printf("@");
                m_need_view = true;
            }
        }
        this_thread::sleep_for(1ms);
    }

    {
        unique_lock<mutex> lock(m_decMutex);
        if (m_frame)
            av_frame_free(&m_frame);
    }
}

std::shared_ptr<PtrAVFrame> DecodecThread::getFrame()
{
    shared_ptr<PtrAVFrame> s;
    unique_lock<mutex>     lock(m_decMutex);
    if (!m_need_view || !m_frame || !m_frame->buf[0])
        return s;

    s.reset(PtrAVFrame::create());
    auto re = av_frame_ref(s->frame, m_frame); // 引用加1
    if (re != 0)
    {
        LOG_WARN << printError(re).c_str();
        s.reset();
        return s;
    }
    m_need_view = false;
    return s;
}
