/**************************************************************************
*   文件名      ：%{Cpp:License:FileName}
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
#include "VideoRerdererView.h"
#include <thread>

#define CERR(err)                            \
    if (err != 0)                            \
    {                                        \
        LOG_WARN << printError(err).c_str(); \
        return -1;                           \
    }

//测试解封装后播放
#define TEST_PLAY 0
//解封装后重新封装为mp4
#define TEST_REMUX 1

int main(int argc, char* argv[])
{
    //    QApplication a(argc, argv);
    //    MainWindow w;
    //    w.show();
    //    return a.exec();
    Logger::initLog();
    string           url = "f:/jqdz/mini.mp4";
    AVFormatContext* ic  = nullptr;
    auto             re  = avformat_open_input(&ic, url.c_str(), nullptr, nullptr);
    CERR(re)

    //获取媒体信息
    re = avformat_find_stream_info(ic, nullptr);
    CERR(re)

    av_dump_format(ic, 0, url.c_str(), 0 //0表示是打印输入的信息,1是表示输出的
    );

    AVStream* as = nullptr;
    AVStream* vs = nullptr;
    for (int i = 0; i < ic->nb_streams; i++)
    {
        if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            as = ic->streams[i];
            LOG_INFO << "audio: " << as->codecpar->sample_rate;
        }
        else if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vs = ic->streams[i];
            LOG_INFO << "video: " << vs->codecpar->codec_id << " " << vs->codecpar->width;
        }
    }

    Decodec dec;
    auto    ctx = dec.cteate(vs->codecpar->codec_id, false);

    //解封装的视频编码参数,传递给解码对象
    avcodec_parameters_to_context(ctx, vs->codecpar);

    //设置完后,不能其外部使用
    dec.setContext(ctx);
    if (!dec.open())
    {
        LOG_WARN << "open error!";
        return -1;
    }
    auto frame = av_frame_alloc();

#if TEST_PLAY
    /// 渲染初始化
    auto view = VideoRerdererView::create();
    view->init(vs->codecpar->width, vs->codecpar->height, (VideoRerdererView::PixFormat) vs->codecpar->format);
#endif

#if TEST_REMUX
    const char*      out_url = "test.mp4";
    AVFormatContext* oc      = nullptr;
    re                       = avformat_alloc_output_context2(&oc, nullptr, nullptr, out_url);
    CERR(re);

    //添加视频流,音频流
    auto ovs = avformat_new_stream(oc, nullptr);
    auto oas = avformat_new_stream(oc, nullptr);

    //打开开输出IO
    re = avio_open2(&oc->pb, out_url, AVIO_FLAG_WRITE, nullptr, nullptr);
    CERR(re);

    //设置编码音视频流参数
    if (vs)
    {
        ovs->time_base = vs->time_base; //时间基保持一至
        avcodec_parameters_copy(ovs->codecpar, vs->codecpar);
    }
    if (as)
    {
        oas->time_base = as->time_base;
        avcodec_parameters_copy(oas->codecpar, as->codecpar);
    }

    //写入头文件
    re = avformat_write_header(oc, nullptr);
    CERR(re);

    /// 取10~20秒之间的音视频,取多不取少
    double    begin_sec       = 10.0; //截取开始时间
    double    end_sec         = 20.0; //截取结束时间
    long long begin_pts       = 0;
    long long begin_audio_pts = 0;
    long long end_pts         = 0;

    // 换算成pts,换算马输入ic的pts, 以视频流为准
    if (vs && vs->time_base.num > 0)
    {
        //pts = sec /(num/den)
        double t  = (double) vs->time_base.den / (double) vs->time_base.num;
        begin_pts = begin_sec * t;
        end_pts   = end_sec * t;
    }

    if (as && as->time_base.den > 0)
        begin_audio_pts = begin_sec * ((double) as->time_base.den / (double) as->time_base.num);

    if (vs)
    {
        //seek输入媒体,移动到第十秒的关键帧位置
        re = av_seek_frame(ic, vs->index, begin_pts, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD); //向后移动
    }
#endif

    AVPacket pkt;
    for (;;)
    {
        re = av_read_frame(ic, &pkt);
        CERR(re)

        //重新计算pts dts duration
        AVStream* in_stream  = ic->streams[pkt.stream_index];
        AVStream* out_stream = nullptr;
        long long offset_pts = 0; //用于偏移pts,截断开头的pts计算

        if (pkt.stream_index == vs->index)
        {
#if TEST_REMUX
            //超过20秒退出
            if (pkt.pts >= end_pts)
            {
                av_packet_unref(&pkt);
                break;
            }
            out_stream = oc->streams[0];
            offset_pts = begin_pts;
#endif
            //            LOG_INFO << "video: " << pkt.size;
#if TEST_PLAY
            if (dec.send(&pkt))
            {
                while (dec.recv(frame))
                {
                    //                    LOG_INFO << frame->width << frame->height;
                    view->drawAVFrame(frame);
                }
            }
#endif
        }
        else if (pkt.stream_index == as->index)
        {
            //            LOG_INFO << "audio: " << pkt.size;
#if TEST_REMUX
            out_stream = oc->streams[1];
            offset_pts = begin_audio_pts;
#endif
        }

#if TEST_REMUX


        pkt.pts      = av_rescale_q_rnd(pkt.pts - offset_pts,
                                   in_stream->time_base,
                                   out_stream->time_base,
                                   (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts      = av_rescale_q_rnd(pkt.dts - offset_pts,
                                   in_stream->time_base,
                                   out_stream->time_base,
                                   (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos      = -1;

        //写入音视频帧,会清理pkt
        re = av_interleaved_write_frame(oc, &pkt);

#endif
        av_packet_unref(&pkt);
        //        this_thread::sleep_for(30ms);
    }

#if TEST_REMUX
    //写文件尾
    re = av_write_trailer(oc);
    if (re != 0)
        LOG_WARN << printError(re).c_str();
    avio_closep(&oc->pb);
    avformat_free_context(oc);
    oc = nullptr;

#endif
    av_frame_free(&frame);
    avformat_close_input(&ic);
    return -1;
}
