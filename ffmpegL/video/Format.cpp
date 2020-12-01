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

    if(!m_c)
        return;

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

bool Format::copyParame(int stream_index, AVCodecContext *dst)
{
    if(!dst)
        return false;

    unique_lock<mutex> lock(m_mutex);
    if (!m_c)
        return false;
    if (stream_index < 0 || stream_index >= m_c->nb_streams )
        return false;

    auto re = avcodec_parameters_to_context(dst, m_c->streams[stream_index]->codecpar);
    if (re < 0)
    {
        return false;
    }
    return true;
}

bool Format::rescaleTime(AVPacket* pkt, long long offset_pts, Rational time_base)
{
    if (!pkt)
        return false;

    unique_lock<mutex> lock(m_mutex);
    if (!m_c)
        return false;
    auto       out_stream = m_c->streams[pkt->stream_index];
    AVRational in_time_base{time_base.num, time_base.den};
    pkt->pts      = av_rescale_q_rnd(pkt->pts - offset_pts,
                                in_time_base,
                                out_stream->time_base,
                                (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
    pkt->dts      = av_rescale_q_rnd(pkt->dts - offset_pts,
                                in_time_base,
                                out_stream->time_base,
                                (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
    pkt->duration = av_rescale_q(pkt->duration, in_time_base, out_stream->time_base);
    pkt->pos      = -1;
    return true;
}

