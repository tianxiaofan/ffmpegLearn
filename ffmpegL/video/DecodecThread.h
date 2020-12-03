/**************************************************************************
*   文件名      ：DecodecThread.h
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
#include "Decodec.h"

class DecodecThread : public CThread
{
public:
    void todo(AVPacket* pkt) override;

    /**
     * @brief open 打开解码器
     * @param para
     * @return
     */
    bool open(AVCodecParameters* para);

    void threadRun() override;

    /**
     * @brief getFrame 返回当前需要渲染的AVFrame,线程安全
     * 如果没有返回null,通过m_need_view渲染
     * @return 返回值为智能指针,不需要手动释放
     */
    std::shared_ptr<PtrAVFrame> getFrame();

private:
    std::mutex m_decMutex;
    Decodec    m_decodec;
    AVPktList  m_pktList;
    AVFrame*   m_frame     = nullptr;
    bool       m_need_view = false; //是否需要渲染,每帧渲染一次
};

