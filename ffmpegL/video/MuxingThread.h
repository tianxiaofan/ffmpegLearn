/**************************************************************************
*   文件名      ：MuxingThread.h
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-12-2
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
#include "CTools.h"
#include "Muxing.h"

class MuxingThread : public CThread
{
public:
    MuxingThread();

    /**
     * @brief open 打开封装文件
     * @param url 文件地址
     * @param video_para 视频参数
     * @param video_time_base 视频时间基
     * @param audio_para 音频参数
     * @param audio_time_base 音频时间基
     * @return
     */
    bool open(const char*        url,
              AVCodecParameters* video_para      = nullptr,
              AVRational*        video_time_base = nullptr,
              AVCodecParameters* audio_para      = nullptr,
              AVRational*        audio_time_base = nullptr);

    void threadRun() override;

    void todo(AVPacket* pkt) override;

private:
    Muxing     remux;
    std::mutex m_reRemux;
    AVPktList  m_pktList;
};

