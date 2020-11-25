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
#include "Encode.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}
#include "Logger.h"

static std::string printError(int err)
{
    char buff[1024] = {0};
    av_strerror(err,buff,sizeof (buff) -1);
    return buff;
}

Encode::Encode()
{

}

AVCodecContext* Encode::cteate(int codeId)
{
    auto codec = avcodec_find_encoder((AVCodecID) codeId);
    if (!codec)
    {
        LOG_WARN << "avcodec_find_encoder: failed!" << codeId;
        return nullptr;
    }

    auto ctx = avcodec_alloc_context3(codec);
    if (!ctx)
    {
        LOG_WARN << "avcodec_alloc_context3 : failed!";
        return nullptr;
    }

    //设置默认参数
    ctx->time_base    = {1, 25};
    ctx->pix_fmt      = AV_PIX_FMT_YUV420P;
    ctx->thread_count = std::thread::hardware_concurrency();

    return ctx;
}

void Encode::setContext(AVCodecContext* c)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_ctx)
    {
        avcodec_free_context(&m_ctx);
    }
    m_ctx = c;
}

bool Encode::setOpt(const char* key, const char* val)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_ctx)
        return false;
    auto re = av_opt_set(m_ctx->priv_data, key, val, 0);
    if (re != 0)
    {
        LOG_WARN << "av_opt_set: " << printError(re).c_str();
        return false;
    }
    return true;
}

bool Encode::setOpt(const char* key, int val)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_ctx)
        return false;
    auto re = av_opt_set_int(m_ctx->priv_data, key, val, 0);
    if (re != 0)
    {
        LOG_WARN << "av_opt_set: " << printError(re).c_str();
        return false;
    }
    return true;
}

bool Encode::open()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_ctx)
        return false;
    auto re = avcodec_open2(m_ctx, nullptr, nullptr);
    if (re != 0)
    {
        LOG_WARN << "avcodec_open2: " << printError(re).c_str();
        return false;
    }
    return true;
}

void Encode::close()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_ctx)
        return;
    avcodec_free_context(&m_ctx);
}

AVPacket* Encode::encodeAVFrame(const AVFrame* frame)
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

std::vector<AVPacket*> Encode::end()
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

AVFrame* Encode::createAVFrame()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_ctx)
        return nullptr;
    auto frame    = av_frame_alloc();
    frame->width  = m_ctx->width;
    frame->height = m_ctx->height;
    frame->format = m_ctx->pix_fmt;
    auto re       = av_frame_get_buffer(frame, 0);
    if (re != 0)
    {
        av_frame_free(&frame);
        LOG_WARN << "av_frame_get_buffer: " << printError(re).c_str();
        return nullptr;
    }
    return frame;
}

void Encode::free(AVPacket *pkt)
{
    if(pkt)
    {
        av_packet_free(&pkt);
    }
}

void Encode::free(AVFrame *frame)
{
    if(frame)
    {
        av_frame_free(&frame);
    }
}
