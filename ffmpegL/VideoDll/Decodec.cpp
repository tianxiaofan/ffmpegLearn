/**************************************************************************
*   文件名      ：Decodec.cpp
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
#include "Decodec.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}
#include "Logger.h"

Decodec::Decodec()
{

}

bool Decodec::send(const AVPacket *pkt)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_ctx)
        return false;
    auto re = avcodec_send_packet(m_ctx, pkt);
    if(re != 0)
        return false;
    return true;
}

bool Decodec::recv(AVFrame *frame, bool is_hw_copy)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_ctx)
        return false;

    auto tmp_frame = frame;
    if (is_hw_copy &&  m_ctx->hw_device_ctx)
    {
        tmp_frame = av_frame_alloc();
    }

    auto re = avcodec_receive_frame(m_ctx, tmp_frame);
    if (re == 0)
    {
        if (is_hw_copy && m_ctx->hw_device_ctx) //硬解
        {
            //显存转内存
            re = av_hwframe_transfer_data(frame, tmp_frame, 0);
            av_frame_free(&tmp_frame);
            if (re != 0)
            {
                LOG_WARN << printError(re).c_str();
                return false;
            }
        }
        return true;
    }
    if (is_hw_copy && m_ctx->hw_device_ctx)
        av_frame_free(&tmp_frame);
    return false;
}

std::vector<AVFrame*> Decodec::end()
{
    std::vector<AVFrame*>        vec;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_ctx)
        return vec;

    auto re = avcodec_send_packet(m_ctx, nullptr); //发送null获取缓冲
    while (re >= 0)
    {
        auto frame = av_frame_alloc();
        re         = avcodec_receive_frame(m_ctx, frame);
        if (re != 0)
        {
            av_frame_free(&frame);
            break;
        }
        vec.push_back(frame);
    }
    return vec;
}

bool Decodec::initHW(int type)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_ctx)
        return false;

    AVBufferRef* hw_device_ctx = nullptr;
    auto         re            = av_hwdevice_ctx_create(&hw_device_ctx, (AVHWDeviceType) type, nullptr, nullptr, 0);
    if (re != 0)
    {
        LOG_WARN << printError(re).c_str();
        return false;
    }
    m_ctx->hw_device_ctx = hw_device_ctx;
    LOG_INFO << "hwaccel: " << type;
    return true;
}
