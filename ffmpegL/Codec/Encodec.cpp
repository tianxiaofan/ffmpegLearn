/**************************************************************************
*   文件名      ：Encode.cpp
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
#include "Encodec.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}
#include "Logger.h"

Encodec::Encodec()
{

}


AVPacket* Encodec::encodeAVFrame(const AVFrame* frame)
{
    if (!frame || frame->width <= 0 || frame->height <= 0)
        return nullptr;

    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_ctx)
        return nullptr;

    auto re = avcodec_send_frame(m_ctx, frame);
    if (re != 0)
        return nullptr;

    auto pkt = av_packet_alloc();
    re       = avcodec_receive_packet(m_ctx, pkt);

    if (re == 0)
        return pkt;

    if (re == AVERROR(EAGAIN) || re == AVERROR_EOF)
    {
        av_packet_free(&pkt);
    }
    if (re < 0)
    {
        LOG_WARN << "avcodec_reveive_packet: " << printError(re).c_str();
    }
    return nullptr;
}

std::vector<AVPacket*> Encodec::end()
{
    std::vector<AVPacket*>       vec;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_ctx)
        return vec;

    auto re = avcodec_send_frame(m_ctx, nullptr); //发送null获取缓冲
    if (re != 0)
        return vec;
    while (re >= 0)
    {
        auto pkt = av_packet_alloc();
        re       = avcodec_receive_packet(m_ctx, pkt);
        if(re != 0)
        {
            av_packet_free(&pkt);
            break;
        }
        vec.push_back(pkt);
    }
    return vec;
}

