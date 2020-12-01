/**************************************************************************
*   文件名      ：Encode.h
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-25
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
class Encodec: public Codec
{
public:
    Encodec();

    /**
     * @brief encodeAVFrame 编码数据,线程安全,每次会新创建AVPacket
     * @param frame 由设置维护
     * @return 失败返回nullptr,return AVPacket user mast use av_packet_free close;
     */
    AVPacket* encodeAVFrame(const AVFrame* frame);

    /**
     * @brief end 取编码器中的缓冲的值
     * @return
     */
    std::vector<AVPacket*> end();

};

