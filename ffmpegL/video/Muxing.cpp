/**************************************************************************
*   文件名      ：Muxing.cpp
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-27
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
#include "Muxing.h"
extern "C"
{
#include <libavformat/avformat.h>
}
#include "Logger.h"
#include "Codec.h"


using namespace std;

Muxing::Muxing() {}

Muxing::~Muxing()
{
    unique_lock<mutex> lock(m_mutex);

    if (m_src_video_timebase)
    {
        delete m_src_video_timebase;
        m_src_video_timebase = nullptr;
    }

    if (m_src_audio_timebase)
    {
        delete m_src_audio_timebase;
        m_src_audio_timebase = nullptr;
    }
}

AVFormatContext* Muxing::open(const char* url, AVCodecParameters* video_para, AVCodecParameters* audio_para)
{
    AVFormatContext* c  = nullptr;
    auto             re = avformat_alloc_output_context2(&c, nullptr, nullptr, url);
    if (re < 0)
    {
        LOG_WARN << printError(re).c_str();
        return nullptr;
    }

    //添加视频流,音频流
    if(video_para)
    {
        auto vs = avformat_new_stream(c, nullptr); //视频流
        avcodec_parameters_copy(vs->codecpar, video_para);
    }
    if(audio_para)
    {
        auto as = avformat_new_stream(c, nullptr); //音频流
        avcodec_parameters_copy(as->codecpar, audio_para);
    }

    re = avio_open(&c->pb, url, AVIO_FLAG_WRITE);
    if (re < 0)
    {
        LOG_WARN << printError(re).c_str();
        avformat_free_context(c);
        return nullptr;
    }
    av_dump_format(c, 0, url, 1);
    return c;
}

bool Muxing::writeHead()
{
    unique_lock<mutex> lock(m_mutex);
    if (!m_c)
        return false;
    auto re = avformat_write_header(m_c, nullptr);
    if (re < 0)
    {
        LOG_WARN << printError(re).c_str();
        return false;
    }

    //设置为初始值
    m_src_audio_begin_pts = -1;
    m_src_video_begin_pts = -1;

    av_dump_format(m_c, 0, m_c->url, 1);

    return true;
}

bool Muxing::writeAVPacket(AVPacket* pkt)
{
    if(!pkt)
        return false;

    unique_lock<mutex> lock(m_mutex);
    if (!m_c)
        return false;
    //没读取到pts
    if (pkt->pts == AV_NOPTS_VALUE)
    {
        pkt->pts = 0;
        pkt->dts = 0;
    }
    if (pkt->stream_index == videoIndex())
    {
        if (m_src_video_begin_pts < 0)
            m_src_video_begin_pts = pkt->pts;
        lock.unlock();
        rescaleTime(pkt, m_src_video_begin_pts, m_src_video_timebase);
        lock.lock();
    }
    else if (pkt->stream_index == audioIndex()) {
        if (m_src_audio_begin_pts < 0)
            m_src_audio_begin_pts = pkt->pts;
        lock.unlock();
        rescaleTime(pkt, m_src_video_begin_pts, m_src_audio_timebase);
        lock.lock();
    }

    printf("w>%lld", pkt->pts);
    auto re = av_interleaved_write_frame(m_c, pkt); //pkt为null时,可写入缓冲
    if (re != 0)
    {
        LOG_WARN << printError(re).c_str();
        return false;
    }
    return true;
}

bool Muxing::writeEnd()
{
    unique_lock<mutex> lock(m_mutex);
    if (!m_c)
        return false;
    av_interleaved_write_frame(m_c, nullptr); //可写入缓冲数据
    auto re = av_write_trailer(m_c);
    if (re != 0)
    {
        LOG_WARN << printError(re).c_str();
        return false;
    }
    return true;
}

void Muxing::set_src_video_timebase(AVRational* ra)
{
    if(!ra)
        return;

    unique_lock<mutex> lock(m_mutex);
    if (!m_src_video_timebase)
        m_src_video_timebase = new AVRational();
    *m_src_video_timebase = *ra;
}

void Muxing::set_src_audio_timebase(AVRational* ra)
{
    if(!ra)
        return;

    unique_lock<mutex> lock(m_mutex);
    if (!m_src_audio_timebase)
        m_src_audio_timebase = new AVRational();
    *m_src_audio_timebase = *ra;
}
