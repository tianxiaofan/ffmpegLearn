/**************************************************************************
*   文件名      ：Format.h
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
#pragma once
#include<mutex>


struct AVFormatContext;
struct AVCodecParameters;
struct AVPacket;

struct Rational{
    int num; ///< Numerator
    int den; ///< Denominator
};


/**
 * @brief The Format class 封装和解封装的基类
 */
class Format
{
public:
    Format();

    /**
     * @brief setContext 设置上下文,并且清理上次设置值,如果传递nullptr,可关闭并清理上下文
     * @param c
     */
    void setContext(AVFormatContext* c);

    int      videoIndex() { return m_videoIndex; }
    int      audioIndex() { return m_audioIndex; }
    Rational videoTimeBase() { return m_videoTimeBase; }
    Rational audioTimeBase() { return m_audioTimeBase; }

    /**
     * @brief copyParame 拷贝stream_index对应的AVCodecParameters
     * @param stream_index stream 下标
     * @param dst 目标参数
     * @return
     */
    bool copyParame(int stream_index, AVCodecParameters* dst);

    /**
     * @brief rescaleTime 根据timebase计算pts
     * @return
     */
    bool rescaleTime(AVPacket* pkt, long long offset_pts,Rational time_base);

protected:
    AVFormatContext* m_c = nullptr; //封装或解封装上下文
    std::mutex       m_mutex;
    int              m_videoIndex    = 0; //video和audio在stream中的索引
    int              m_audioIndex    = 1;
    Rational         m_videoTimeBase = {1, 25};
    Rational         m_audioTimeBase = {1, 9000};
};

