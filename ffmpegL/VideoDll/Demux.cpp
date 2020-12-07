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

AVFormatContext* Demux::open(const char* url,std::vector<DictionaryOpt> o)
{
    AVFormatContext* c  = nullptr;
    AVDictionary*    opts = nullptr;
    if (o.size() > 0)
    {
        for (auto op : o)
        {
            av_dict_set(&opts, op.key.c_str(), op.value.c_str(), 0);
        }
    }
    AVDictionaryEntry* tag = NULL;
    while ((tag = av_dict_get(opts, "", tag, AV_DICT_IGNORE_SUFFIX)))
    {
        LOG_INFO << "key:" << tag->key << " value:" << tag->value;
    }
    auto re = avformat_open_input(&c, url, nullptr, &opts);
    if (re != 0)
    {
        LOG_WARN << printError(re).c_str();
        return nullptr;
    }

    if(opts)
        av_dict_free(&opts);

    c->probesize = 10000 * 1024; //限制最大的读取缓存，比如500*1024就是500k，500k可以保证高清视频也可以读取到一帧视频帧，如果太小就会导致读取不到完整视频帧的问题，所以这个大小需要通过实际情况进行控制
    c->max_analyze_duration = 2 * AV_TIME_BASE;//超过这个时间不结束也会结束，我们设置为5秒

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
