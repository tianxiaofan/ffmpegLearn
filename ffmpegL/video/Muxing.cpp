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

Muxing::Muxing()
{

}

AVFormatContext* Muxing::open(const char* url)
{
    AVFormatContext* c  = nullptr;
    auto             re = avformat_alloc_output_context2(&c, nullptr, nullptr, url);
    if (re < 0)
    {
        LOG_WARN << printError(re).c_str();
        return nullptr;
    }

    //添加视频流,音频流
    auto vs                  = avformat_new_stream(c, nullptr); //视频流
    vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    auto as                  = avformat_new_stream(c, nullptr); //音频流
    as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;

    re = avio_open(&c->pb, url, AVIO_FLAG_WRITE);
    if (re < 0)
    {
        LOG_WARN << printError(re).c_str();
        avformat_free_context(c);
        return nullptr;
    }
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

    av_dump_format(m_c, 0, m_c->url, 1);

    return true;
}

bool Muxing::writeAVPacket(AVPacket* pkt)
{
    unique_lock<mutex> lock(m_mutex);
    if (!m_c)
        return false;
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
