/**************************************************************************
*   文件名      ：%{Cpp:License:FileName}
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-24
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
#include "MainWindow.h"

#include "Logger.h"
#include <QApplication>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/log.h>
}

#include <QThread>
#include <fstream>

int main(int argc, char* argv[])
{
    Logger::initLog();
    //    QApplication a(argc, argv);
    //    MainWindow   w;
    //    w.show();
    //    //C++ 和Qt 各自获取cpu核心数
    //    LOG_DEBUG << std::thread::hardware_concurrency();
    //    LOG_DEBUG << QThread::idealThreadCount();
    //    return a.exec();

    std::string filename = "1280x720";
    std::string type = "hevc";
    AVCodecID   codecId  = AV_CODEC_ID_H264;
    if (type == "h265" || type == "hevc")
    {
        codecId = AV_CODEC_ID_HEVC;
    }

    if (codecId == AV_CODEC_ID_H264)
    {
        filename += ".h264";
    }
    if (codecId == AV_CODEC_ID_HEVC)
    {
        filename += ".h265";
    }

    //打开文件
    std::ofstream ofs;
    ofs.open(filename,std::ios::binary);

    //[1]查找编码器
    auto codec = avcodec_find_encoder(codecId);
    if (!codec)
    {
        LOG_WARN << "codec not find";
        return -1;
    }

    //[2]创建编码器上下文
    auto ctx = avcodec_alloc_context3(codec);

    if (!ctx)
    {
        LOG_WARN << "avcodec_alloc_context3:error";
        return -1;
    }
    //[3]设置参数
    ctx->width  = 1280; //视频宽高
    ctx->height = 720; //
    ctx->time_base = {1, 25}; //帧数据的时间单位,1秒内有个单位, 时间基数,在编码时必须由用户设置
    ctx->pix_fmt = AV_PIX_FMT_YUV420P; //要编码成的目标格式
    //编码线程数,可以通过调用系统接口来调用cpu核心数量来设置,本机是6
    ctx->thread_count = std::thread::hardware_concurrency();

    //[4]打开编码上下文
    int re = avcodec_open2(ctx, codec, nullptr);
    if (re != 0)
    {
        char buff[1024] = {0};
        av_strerror(re, buff, sizeof(buff) - 1);
        LOG_WARN << buff;
        avcodec_free_context(&ctx);
        return -1;
    }
    LOG_INFO << "avcodec_open2: success";

    //[5]创建frame,用以存放未压缩数据
    auto frame    = av_frame_alloc();
    frame->width  = ctx->width;
    frame->height = ctx->height;
    frame->format = ctx->pix_fmt;
    re            = av_frame_get_buffer(frame, 0);
    if (re != 0)
    {
        char buff[1024] = {0};
        av_strerror(re, buff, sizeof(buff) - 1);
        LOG_WARN << buff;
        av_frame_free(&frame);
        avcodec_free_context(&ctx);
        return -1;
    }
    //[6]初始化avpacket
    auto pkt = av_packet_alloc();

    //[7] 创建10s的视频,25fps
    for (int i = 0; i < 250; i++)
    {
        //[7.1] 创建avframe数据,yuv420p.来自ffmpeg官方案例
        for (int y = 0; y < ctx->height; y++) //Y
        {
            for (int x = 0; x < ctx->width; x++)
            {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
            }
        }
        for (int u = 0; u < ctx->height / 2; u++) //UV
        {
            for (int x = 0; x < ctx->width / 2; x++)
            {
                frame->data[1][u * frame->linesize[1] + x] = 128 + u + i * 2;
                frame->data[2][u * frame->linesize[2] + x] = 64 + x + i * 5;
            }
        }

        //[7.2]设置显示时间
        frame->pts = i;

        //[7.3]发送进编码器
        re = avcodec_send_frame(ctx, frame);
        if (re != 0)
        {
            continue;
        }
        do
        {
            //[7.4] 接收压缩帧,一般前几次调用返回为空,内部有缓冲,编码未完成时调用也会会立刻返回,因编码是在独立线程
            re = avcodec_receive_packet(ctx, pkt);
            if (re == AVERROR(EAGAIN) || re == AVERROR_EOF)
                break;
            if (re < 0)
            {
                char buff[1024] = {0};
                av_strerror(re, buff, sizeof(buff) - 1);
                LOG_WARN << buff;
                break;
            }
            LOG_INFO << pkt->size;
            ofs.write((char*) pkt->data, pkt->size);
            av_packet_unref(pkt);
        } while (re >= 0);
    }

    ofs.close();
    av_frame_free(&frame);
    av_packet_free(&pkt);
    //释放编码器
    avcodec_free_context(&ctx);
}
