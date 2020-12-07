/**************************************************************************
*   文件名      ：DemuxThread.cpp
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-12-1
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
#include "DemuxThread.h"
#include "Logger.h"
#include <iostream>
using namespace std;

extern "C"
{
#include <libavformat/avformat.h>
}

DemuxThread::DemuxThread()
{

}

void DemuxThread::threadRun()
{
    AVPacket pkt;
    while (!m_isExit)
    {
        if (!m_demux.read(&pkt))
        {
            printf("-");
            if (!m_demux.isConnect())
            {
                open(m_url,m_dic, m_timeout);
            }
            this_thread::sleep_for(1ms);
            continue;
        }
        next(&pkt);
        av_packet_unref(&pkt);
        printf("+");
        this_thread::sleep_for(1ms);
    }

    //停止线程前,清空录像上下文
    m_demux.setContext(nullptr);
}

bool DemuxThread::open(string url,std::vector<DictionaryOpt> d, int timeout)
{
    LOG_DEBUG;
    //断开之前的连接
    m_url     = url;
    m_timeout = timeout;
    m_dic     = d;
    m_demux.setContext(nullptr);
    auto c = m_demux.open(url.c_str(),d);
    if(!c)
        return false;

    m_demux.setContext(c);
    m_demux.setTimeoutMs(timeout);
    return true;
}

