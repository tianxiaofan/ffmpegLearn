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
#include <libavutil/opt.h>
}

#include <QThread>
#include <fstream>

//#define AV_SET_PRESET
//#define AV_SET_BIT
//#define AV_SET_CBR
#define AV_SET_CRF

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

    std::string filename = "1280x720_fast";
    std::string type     = "h264";
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
    ofs.open(filename, std::ios::binary);

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

#ifdef AV_SET_PRESET
    /**
    * preset 主要调节编码速度和返璞归真平衡
    * ultrafst,superfast,veryfast,faster,fast,medium,slow,slower,veryslow,placebo
    * --tune 视频类型,视觉优化
    *   film:电影类型
    *   animation:动画
    *   grain:需要保留大量grain时用,颗粒
    *   stillimage:表态图像编码时使用
    *   fastecode:可以快速解码的参数
    *   zerolatency:零延迟,用在需要非常低的延时的情况,比如电视电话会议的编码
    */
    ctx->max_b_frames = 0;//设置B帧数量,设置为0可降低延时
    av_opt_set(ctx->priv_data, "preset", "ultrafast", 0); //最快速度
    av_opt_set(ctx->priv_data, "tune", "zerolatency", 0); //零延时,h265不支持b帧
#endif

#ifdef AV_SET_BIT
    /**
    * 比特率控制
    *   Average Bitrate (ABR,also "target bitrate")
    *   避免使用此模式
    *   快速的脏编码,质量比较差
    *   c->bit_rate = 400000;
    * CQP 恒定质量
    *   恒定QP量化因子
    *   Constant QP(CQP)
    *   h.264的QP范围从0到51
    *   x264默认23 效果较好18
    *   x265默认28 效果较好25
    *   QP设置得足够低,则质量会越好,空间会增大,无法控制实际的比特率
    *   av_opt_set_int(c->priv_data,"qp",10,0);
    */

    //ABR 平均比特率
    int br = 400000; //400k
    //    ctx->bit_rate = br;

    //恒定质量
    av_opt_set_int(ctx->priv_data, "qp", 18, 0);
#endif

#ifdef AV_SET_CBR
    /**
     * 使用后 比特率会比较稳定
    * av_opt_set(c->priv_data,"nal-hrd","cbr",0);
    * int rate = 400000;
    * c->rc_min_rate = rate;
    * c->rc_max_rate = rate;
    * c->rc_buffer_size = rate*2;
    * c->bit_rate = rate;
    * 由于mp4不支持nal填充,因此输出文件必须为 mpeg-2 ts
    */
    int br              = 400000; //400k
    ctx->rc_max_rate    = br;
    ctx->rc_min_rate    = br;
    ctx->bit_rate       = br;
    ctx->rc_buffer_size = br;
    av_opt_set(ctx->priv_data, "nal-hrd", "cbr", 0);
#endif

#ifdef AV_SET_CRF
    /**
    *   恒定速率因子 CRF
    *   CRF
    *       提供恒定的质量
    *       CRF比CQP参数要复杂 CRF ranges from 0-51
    *       CRF每一帧的压缩不同大小,主观的质量感受差不多,会用不同的QP
    *       根据运动情况降低或者提高QP
    *       av_opt_set_int(c->priv_data,"crf","23",0);
    *   约束编码 VBV      Constrained Encoding(VBV)
    *       int rate = 400000;
    *       c->rc_max_rate = rate;
    *       c->rc_buffer_size = rate*2;
    */
    av_opt_set_int(ctx->priv_data, "crf", 23, 0);
    int rate            = 400000;
    ctx->rc_max_rate    = rate;
    ctx->rc_buffer_size = rate * 2;
#endif

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
    for (int i = 0; i < 500; i++)
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
