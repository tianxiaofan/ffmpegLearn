/**************************************************************************
*   文件名      ：%{Cpp:License:FileName}
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-27
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

#include <QApplication>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
using namespace std;
#include "Logger.h"
#include "Decodec.h"
#include "Encodec.h"
#include "VideoRerdererView.h"
#include <thread>
#include "Demux.h"
#include "Muxing.h"

#define CERR(err)                            \
    if (err != 0)                            \
    {                                        \
        LOG_WARN << printError(err).c_str(); \
        return -1;                           \
    }


int main(int argc, char* argv[])
{
    //    QApplication a(argc, argv);
    //    MainWindow w;
    //    w.show();
    //    return a.exec();
    //Logger::initLog();
    string url     = "f:/jqdz/mini.mp4";
    string out_url = "test.mp4";

    //解封装
    Demux demux;
    auto  ictx = demux.open(url.c_str());
    demux.setContext(ictx);

    int       begin_sec       = 10; //截取开始时间
    int       end_sec         = 30; //截取结束时间
    long long begin_video_pts = 0;
    long long begin_audio_pts = 0;
    long long video_end_pts   = 0;


    //移动到开始位置
    //移动开开始时间
    if (begin_sec > 0)
    {
        if (demux.videoIndex() >= 0 && demux.videoTimeBase().num > 0)
        {
            double t        = (double) demux.videoTimeBase().den / (double) demux.videoTimeBase().num;
            begin_video_pts = t * begin_sec;
            video_end_pts   = end_sec * t;
            demux.seek(begin_video_pts, demux.videoIndex());
        }
        if (demux.audioIndex() >= 0 && demux.audioTimeBase().num > 0)
        {
            double t        = (double) demux.audioTimeBase().den / (double) demux.audioTimeBase().num;
            begin_audio_pts = t * begin_sec;
        }
    }

    //视频解码
    Decodec dec;
    auto    dctx = dec.cteate(demux.videoCodecID(), false);

    //设置视频解码器参数
    demux.copyParame(demux.videoIndex(), dctx);

    dec.setContext(dctx);

    dec.open();

    auto frame = dec.createAVFrame(); //解码后的储存

    ///视频编码
    // 设置要编码的宽高
    int video_width  = 0;
    int video_height = 0;
    video_width      = demux.width();
    video_height     = demux.height();

    //设置编码器参数
    Encodec enc;
    auto    enctx  = enc.cteate(AV_CODEC_ID_H265, true);
    enctx->pix_fmt = AV_PIX_FMT_YUV420P;
    enctx->width   = video_width;
    enctx->height  = video_height;
    enc.setContext(enctx);
    enc.open();


    //进行封装
    Muxing remux;
    auto   rmx = remux.open(out_url.c_str());

    remux.setContext(rmx);
    auto vos = rmx->streams[remux.videoIndex()]; //视频流
    auto aos = rmx->streams[remux.audioIndex()]; //音频流

    //视频
    if (demux.videoIndex() >= 0)
    {
        //        demux.copyParame(demux.videoIndex(), vos->codecpar);

        vos->time_base.num = demux.videoTimeBase().num;
        vos->time_base.den = demux.videoTimeBase().den;

        avcodec_parameters_from_context(vos->codecpar, enctx); //复制编码器中的设置到封装器中
    }
    //音频
    if (demux.audioIndex() >= 0)
    {
        demux.copyParame(demux.audioIndex(), aos->codecpar);

        aos->time_base.num = demux.audioTimeBase().num;
        aos->time_base.den = demux.audioTimeBase().den;
    }

    //写入头部时会改变timebase
    remux.writeHead();






    int      video_count = 0;
    int      audio_count = 0;
    double   total_sec   = 0;
    AVPacket pkt;
    for (;;)
    {
        if (!demux.read(&pkt))
            break;

        //视频时间大于结束时间
        if (video_end_pts > 0 && pkt.pts > video_end_pts && pkt.stream_index == demux.videoIndex())
        {
            av_packet_unref(&pkt);
            break;
        }

        if (pkt.stream_index == demux.videoIndex())
        {
            remux.rescaleTime(&pkt, begin_video_pts, demux.videoTimeBase());

            //解码视频
            if (dec.send(&pkt))
            {
                while (dec.recv(frame))
                {
                    //开始编码
                    auto enpkt = enc.encodeAVFrame(frame);
                    if (enpkt)
                    {
                        enpkt->stream_index = remux.videoIndex();
                        //写入音视频帧,会清理pkt
                        remux.writeAVPacket(enpkt);
                        LOG_INFO << "write";
                    }
                }
            }

            video_count++;
            total_sec += pkt.duration * ((double)demux.videoTimeBase().num / demux.videoTimeBase().den);
        }
        if (pkt.stream_index == demux.audioIndex())
        {
            remux.rescaleTime(&pkt, begin_audio_pts, demux.audioTimeBase());
            audio_count++;
            remux.writeAVPacket(&pkt);
        }

        av_packet_unref(&pkt);
    }

    //写文件尾
    remux.writeEnd();
    LOG_INFO << "video_count:" << video_count;
    LOG_INFO << "audio_count:" << audio_count;
    LOG_INFO << "time:" << total_sec;

    remux.setContext(nullptr);
    demux.setContext(nullptr);
    enc.setContext(nullptr);
    dec.setContext(nullptr);

    return -1;
}
