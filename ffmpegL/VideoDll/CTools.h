/**************************************************************************
*   文件名      ：CThread.h
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
#pragma once
#include "VideoDll_global.h"
#include <thread>
#include <mutex>
#include <list>

struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct AVCodecParameters;
struct AVRational;
struct AVFormatContext;

struct VIDEODLL_EXPORT Rational{
    int num; ///< Numerator
    int den; ///< Denominator
};

VIDEODLL_EXPORT std::string printError(int err);

/**
 * @brief customSleep 自定义的sleep
 * @param m
 */
VIDEODLL_EXPORT void customSleep(unsigned int m);

/**
 * @brief getNowMs 获取当前毫秒级的时间戳
 * @return
 */
VIDEODLL_EXPORT long long getNowMs();

class VIDEODLL_EXPORT CThread
{
public:
    /**
     * @brief start 启动线程
     * @return
     */
    virtual bool start();

    /**
     * @brief stop 停止线程,设置退出标志,等待线程退出
     * @return
     */
    virtual bool stop();

    /**
     * @brief todo 责任链处理函数
     * @param pkt
     */
    virtual void todo(AVPacket* pkt) {}

    /**
     * @brief next 传递到一下个责任链
     * @param pkt
     */
    virtual void next(AVPacket* pkt)
    {
        std::unique_lock<std::mutex> lock(m_mux);
        if (m_next)
            m_next->todo(pkt);
    }

    /**
     * @brief setNext 设置责任链下一个节点,线程安全
     * @param next
     */
    void setNext(CThread* next)
    {
        std::unique_lock<std::mutex> lock(m_mux);
        m_next = next;
    }

protected:
    virtual void threadRun() = 0;

protected:
    bool m_isExit      = false; //线程是否要退出
    int  m_threadIndex = 0; //线程索引
private:
    std::thread m_thread;
    std::mutex  m_mux;
    CThread*    m_next = nullptr; //责任链的下一个节点
};


/**
 * @brief The VAPara class 音视频参数
 */
class VIDEODLL_EXPORT VAPara
{
public:
    ~VAPara();

    //创建对象
    static VAPara* create();

    AVCodecParameters* para     = nullptr; //音视频参数
    AVRational*        timeBase = nullptr; //时间基数

private:
    //禁止创建栈中对象
    VAPara();
};

/**
 * @brief The PtrAVFrame class 用于智能指针托管AVFrame
 */
class VIDEODLL_EXPORT PtrAVFrame
{
public:
    ~PtrAVFrame();
    static PtrAVFrame* create();
    AVFrame*           frame = nullptr;
private:
    PtrAVFrame();
};

/**
 * @brief The AVPktList class 线程安全的packet list
 * 注意:在ffmpeg内部有AVPacketList,它是c链表,这里是自定义的
 */
class VIDEODLL_EXPORT AVPktList
{
public:
    AVPacket* pop();
    void      push(AVPacket* pkt);

private:
    std::list<AVPacket*> m_pkt_list;
    int                  max_packet_size = 100;
    std::mutex           m_listMutex;
};
