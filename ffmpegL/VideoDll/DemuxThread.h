/**************************************************************************
*   文件名      ：DemuxThread.h
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
#include "CTools.h"
#include "Demux.h"

class VIDEODLL_EXPORT DemuxThread : public CThread
{
public:
    DemuxThread();
    virtual void threadRun() override;

    /**
     * @brief open 打开解封装
     * @param url rtsp 地焉
     * @param timeout 默认超时,1000ms
     * @return
     */
    bool open(std::string url, std::vector<DictionaryOpt> d = std::vector<DictionaryOpt>(), int timeout = 1000);

    std::shared_ptr<VAPara> copyVideoPara() { return m_demux.copyVideoPara(); }
    std::shared_ptr<VAPara> copyAudioPara() { return m_demux.copyAudioPara(); }

private:
    Demux                      m_demux;
    std::string                m_url;
    int                        m_timeout = 0;
    std::vector<DictionaryOpt> m_dic;
};

