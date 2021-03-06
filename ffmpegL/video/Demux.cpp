/**************************************************************************
*   文件名      ：Demux.cpp
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
#include "Demux.h"

extern "C"
{
#include <libavformat/avformat.h>
}
#include "Logger.h"
#include "Codec.h"
#include "CTools.h"

using namespace std;

Demux::Demux()
{

}

AVFormatContext* Demux::open(const char* url)
{
    AVFormatContext* c  = nullptr;
    AVDictionary*    opts = nullptr;
    av_dict_set(&opts,"rtsp_transport","udp",0);
    av_dict_set(&opts,"stimeout","1000000",0);//连接超时1s
    auto re = avformat_open_input(&c, url, nullptr, &opts);
    if (re != 0)
    {
        LOG_WARN << printError(re).c_str();
        return nullptr;
    }

    if(opts)
        av_dict_free(&opts);

    re = avformat_find_stream_info(c, nullptr);
    if (re < 0)
    {
        LOG_WARN << printError(re).c_str();
        avformat_close_input(&c);
        return nullptr;
    }

    av_dump_format(c, 0, url, 0);
    return c;
}

bool Demux::read(AVPacket* pkt)
{
    unique_lock<mutex> lock(m_mutex);
    if (!m_c)
        return false;

    auto re = av_read_frame(m_c, pkt);
    if (re != 0)
    {
        LOG_WARN << printError(re).c_str();
        return false;
    }
    //刷新计时
    m_last_time = getNowMs();

    return true;
}

bool Demux::seek(long long pts, int stream_index)
{
    unique_lock<mutex> lock(m_mutex);
    if (!m_c)
        return false;
    auto re = av_seek_frame(m_c, stream_index, pts, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    if (re < 0)
    {
        LOG_WARN << printError(re).c_str();
        return false;
    }
    return true;
}
