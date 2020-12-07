/**************************************************************************
*   文件名      ：Decodec.h
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-26
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
#include "Codec.h"

class VIDEODLL_EXPORT Decodec : public Codec
{
public:
    Decodec();

    /**
     * @brief send 发送到解码器
     * @param pkt
     * @return
     */
    bool send(const AVPacket* pkt);

    /**
     * @brief recv 获取解码后的数据
     * @param frame
     * @return
     */
    bool recv(AVFrame* frame,bool is_hw_copy = true);

    /**
     * @brief end 获取缓存数据
     * @return
     */
    std::vector<AVFrame*> end();

    /**
     * @brief initHW 初始化硬件加速,默认支持DXVA2
     * @param type
     * @return
     */
    bool initHW(int type = 4);
};

