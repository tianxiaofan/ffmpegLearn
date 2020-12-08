/**************************************************************************
*   文件名      ：Format.cpp
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
#include "Format.h"
#include "Codec.h"
#include "Logger.h"
extern "C"
{
#include <libavformat/avformat.h>
}

using namespace std;

static int TimeoutCallback(void* para)
{
    auto format = (Format*) para;
    if(format->isTimeout())
        return 1; //退出read_frame
    return 0; //正常阻塞
}

Format::Format()
{

}

void Format::setContext(AVFormatContext* c)
{
    unique_lock<mutex> lock(m_mutex);
    if (m_c)
    {
        if (m_c->oformat) //输出上下文
        {
            if (m_c->pb)
                avio_closep(&m_c->pb);
            avformat_free_context(m_c);
        }
        else if (m_c->iformat) //输入上下文
        {
            avformat_close_input(&m_c);
        }
        else
        {
            avformat_free_context(m_c);
        }
        m_c = nullptr;
    }
    m_c = c;

    if (!m_c)
    {
        m_isConnect = false;
        return;
    }
    m_isConnect = true;

    //计时用于超时判断
    m_last_time = getNowMs();

    //设置超时回调函数
    if (m_timeout > 0)
    {
        AVIOInterruptCB callback = {TimeoutCallback, this};
        m_c->interrupt_callback  = callback;
    }

    //区分音视频
    m_audioIndex = -1;
    m_videoIndex = -1;
    for (int i = 0; i < m_c->nb_streams; i++)
    {
        if (m_c->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            m_audioIndex        = i;
            m_audioTimeBase.den = m_c->streams[i]->time_base.den;
            m_audioTimeBase.num = m_c->streams[i]->time_base.num;
        }
        else if (m_c->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_videoIndex = i;
            m_videoTimeBase.den = m_c->streams[i]->time_base.den;
            m_videoTimeBase.num = m_c->streams[i]->time_base.num;
            m_videoCodecID      = m_c->streams[i]->codecpar->codec_id;
            m_width             = m_c->streams[i]->codecpar->width;
            m_height            = m_c->streams[i]->codecpar->height;
        }
    }
}

bool Format::copyParame(int stream_index, AVCodecParameters* dst)
{
    unique_lock<mutex> lock(m_mutex);
    if (!m_c)
        return false;
    if (stream_index < 0 || stream_index >= m_c->nb_streams )
        return false;

    auto re = avcodec_parameters_copy(dst, m_c->streams[stream_index]->codecpar);
    if (re < 0)
    {
        return false;
    }
    return true;
}

bool Format::copyParame(int stream_index, AVCodecContext* dst)
{
    if (!dst)
        return false;

    unique_lock<mutex> lock(m_mutex);
    if (!m_c)
        return false;
    if (stream_index < 0 || stream_index >= m_c->nb_streams)
        return false;

    auto re = avcodec_parameters_to_context(dst, m_c->streams[stream_index]->codecpar);
    if (re < 0)
    {
        return false;
    }
    return true;
}

std::shared_ptr<VAPara> Format::copyVideoPara()
{
    int                index = videoIndex();
    shared_ptr<VAPara> re;
    unique_lock<mutex> lock(m_mutex);
    if (index < 0 || !m_c)
        return re;
    re.reset(VAPara::create());
    *re->timeBase = m_c->streams[index]->time_base;
    avcodec_parameters_copy(re->para,m_c->streams[index]->codecpar);

    return re;
}

std::shared_ptr<VAPara> Format::copyAudioPara()
{
    int                index = audioIndex();
    shared_ptr<VAPara> re;
    unique_lock<mutex> lock(m_mutex);
    if (index < 0 || !m_c)
        return re;
    re.reset(VAPara::create());
    *re->timeBase = m_c->streams[index]->time_base;
    avcodec_parameters_copy(re->para,m_c->streams[index]->codecpar);

    return re;
}

bool Format::rescaleTime(AVPacket* pkt, long long offset_pts, Rational time_base)
{
    if (!pkt)
        return false;

    AVRational in_time_base{time_base.num, time_base.den};
    return rescaleTime(pkt,offset_pts,&in_time_base);
}

bool Format::rescaleTime(AVPacket *pkt, long long offset_pts, AVRational *time_base)
{
    if (!pkt || !time_base)
        return false;

    unique_lock<mutex> lock(m_mutex);
    if (!m_c)
        return false;
    auto       out_stream = m_c->streams[pkt->stream_index];
    pkt->pts      = av_rescale_q_rnd(pkt->pts - offset_pts,
                                *time_base,
                                out_stream->time_base,
                                (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
    pkt->dts      = av_rescale_q_rnd(pkt->dts - offset_pts,
                                *time_base,
                                out_stream->time_base,
                                (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
    pkt->duration = av_rescale_q(pkt->duration, *time_base, out_stream->time_base);
    pkt->pos      = -1;
    return true;
}

long long Format::rescaleToMs(long long pts, int index)
{
    unique_lock<mutex> lock(m_mutex);
    if (!m_c || index < 0 || index >= m_c->nb_streams)
        return -0;
    auto       in_timebase  = m_c->streams[index]->time_base;
    AVRational out_timebase = {1,1000}; //输出timebase
    return av_rescale_q(pts, in_timebase, out_timebase);
}


void Format::setTimeoutMs(int ms)
{
    unique_lock<mutex> lock(m_mutex);
    m_timeout = ms;

    //设置回调函数,处理超时退出
    if (m_c)
    {
        AVIOInterruptCB callback = {TimeoutCallback, this};
        m_c->interrupt_callback = callback;
    }
}

