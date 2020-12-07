/**************************************************************************
*   文件名      ：CThread.cpp
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
#include "CTools.h"
#include "Logger.h"

using namespace std;

extern "C"
{
#include <libavformat/avformat.h>
}

std::string printError(int err)
{
    char buff[1024] = {0};
    av_strerror(err,buff,sizeof (buff) -1);
    return buff;
}


void customSleep(unsigned int m)
{
    auto begin = clock();
    for (int i = 0; i < m; i++)
    {
        std::this_thread::sleep_for(1ms);
        if ((clock() - begin) / (CLOCKS_PER_SEC / 1000) >= m)
        {
            break;
        }
    }
}
long long getNowMs()
{
    return clock() / (CLOCKS_PER_SEC / 1000); //兼容跨平台
}

bool CThread::start()
{
    unique_lock<mutex> lock(m_mux);
    static int         i = 0;
    ++i;
    m_threadIndex = i;
    m_isExit      = false;

    m_thread = thread(&CThread::threadRun, this);
    LOG_INFO << "thread:" << m_threadIndex << " start!";
    return true;
}

bool CThread::stop()
{
    LOG_INFO << "thread stop: " << m_threadIndex << "...";
    m_isExit = true;
    if (m_thread.joinable()) //判断线程是否可以等待
    {
        m_thread.join(); //等待子线程退出
    }
    LOG_INFO << "thread stop: " << m_threadIndex << "success!";
    return true;
}

VAPara::~VAPara()
{
    if (para)
        avcodec_parameters_free(&para);
    if (timeBase)
    {
        delete timeBase;
        timeBase = nullptr;
    }
}

VAPara* VAPara::create()
{
    return new VAPara();
}

VAPara::VAPara()
{
    para     = avcodec_parameters_alloc();
    timeBase = new AVRational();
}

PtrAVFrame::~PtrAVFrame()
{
    if (frame)
    {
        av_frame_free(&frame);
    }
}

PtrAVFrame* PtrAVFrame::create()
{
    return new PtrAVFrame;
}

PtrAVFrame::PtrAVFrame()
{
    frame = av_frame_alloc();
}


AVPacket* AVPktList::pop()
{
    unique_lock<mutex> lock(m_listMutex);
    if (m_pkt_list.empty())
        return nullptr;
    auto pkt = m_pkt_list.front();
    m_pkt_list.pop_front();
    return pkt;
}

void AVPktList::push(AVPacket* pkt)
{
    unique_lock<mutex> lock(m_listMutex);
    //生成新的AVPacket对象,引用计数+1
    auto p = av_packet_alloc();
    //引用计数加,减少数据复制,线程安全
    av_packet_ref(p, pkt);
    m_pkt_list.push_back(p);
    //超出最大空间,清理数据到关键帧位置
    if (m_pkt_list.size() > max_packet_size)
    {
        //如果第一帧是关键帧,先清理关键帧
        if (m_pkt_list.front()->flags & AV_PKT_FLAG_KEY)
        {
            av_packet_free(&m_pkt_list.front()); //清理list中的avpack
            m_pkt_list.pop_front(); //清理list,出队
        }

        //清理非关键帧,直到遇到关键帧,停止清理
        while (!m_pkt_list.empty())
        {
            if (m_pkt_list.front()->flags & AV_PKT_FLAG_KEY)
            {
                return;
            }
            av_packet_free(&m_pkt_list.front());
            m_pkt_list.pop_front();
        }
    }
}

