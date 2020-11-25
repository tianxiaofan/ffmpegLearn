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
#include "Encode.h"

//#define AV_SET_PRESET
//#define AV_SET_BIT
//#define AV_SET_CBR
//#define AV_SET_CRF
//#define TEST_NALU

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

    Encode en;
    auto ctx = en.cteate(codecId);
    ctx->width  = 1280; //视频宽高
    ctx->height = 720;  //
    en.setContext(ctx);
    en.setOpt("qp",18);

    int re = 0;
    if (en.open())
        LOG_INFO << "avcodec_open2: success";
    auto frame = en.createAVFrame();
    int  count = 0; //
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
        auto pkt   = en.encodeAVFrame(frame);
        if (pkt)
        {
            ofs.write((char*) pkt->data, pkt->size);
            en.free(pkt);
            count++;
        }
    }

    auto vec = en.end();
    LOG_INFO << vec.size();
    for (auto p : vec)
    {
        ofs.write((char*) p->data, p->size);
        en.free(p);
        count++;
    }

    ofs.close();
    LOG_INFO << count;
    en.free(frame);
    en.close();
//    av_packet_free(&pkt);
    //释放编码器
//    avcodec_free_context(&ctx);
}
