/**************************************************************************
*   文件名      ：%{Cpp:License:FileName}
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
#include "MainWindow.h"

#include <QApplication>
#include <fstream>
using namespace std;
extern "C"
{
#include <libavcodec/avcodec.h>
}
#include "Logger.h"
#include "VideoRerdererView.h"


//#define IS_HWACCEL

//ffmpeg.exe -i .\binghe.mp4 -ss 8:00 -t 1:00 h264out.h264
int main(int argc, char *argv[])
{
    //    QApplication a(argc, argv);
    //    MainWindow w;
    //    w.show();
    //    return a.exec();
    Logger::initLog();
    auto     view     = VideoRerdererView::create();
    bool     initWin  = false;
    string   filename = "f:/jqdz/h264out.h264";
    ifstream ifs;
    ifs.open(filename, ios::binary);
    if (!ifs)
        return -1;

    unsigned char buff[4096] = {0};
    AVCodecID     codecId    = AV_CODEC_ID_H264;

    auto codec        = avcodec_find_decoder(codecId);
    auto ctx          = avcodec_alloc_context3(codec);
    ctx->thread_count = 32;

#ifdef IS_HWACCEL
    auto hw_type = AV_HWDEVICE_TYPE_DXVA2;
    //打印所有支持的硬件加速方式
    for (int i = 0;; i++)
    {
        auto config = avcodec_get_hw_config(codec, i);
        if (!config)
            break;
        if (config->device_type)
            LOG_INFO << av_hwdevice_get_type_name(config->device_type);
    }

    /// 初始化硬件加速上下文
    AVBufferRef* hw_ctx = nullptr;
    av_hwdevice_ctx_create(&hw_ctx, hw_type, nullptr, nullptr, 0);
    ctx->hw_device_ctx = av_buffer_ref(hw_ctx); //引用到内部,这时候就可以释放hw_ctx
#endif



    auto re  = avcodec_open2(ctx, nullptr, nullptr);
    auto pkt = av_packet_alloc();

    auto frame    = av_frame_alloc();
    auto hw_frame = av_frame_alloc();

    //分割上下文
    auto parser = av_parser_init(codecId);

    auto begin = getNowMs(); //获取开始时间
    int  count = 0; //解码统计
    while (!ifs.eof())
    {
        ifs.read((char*) buff, sizeof(buff));
        //循环播放
        if (ifs.eof())
        {
            ifs.close();
            ifs.seekg(0, ios::beg);
        }
        int size = ifs.gcount();

        auto data = buff;
        while (size > 0)
        {
            //通过0001截断,输出到pkt
            re = av_parser_parse2(parser, ctx, &pkt->data, &pkt->size, buff, size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            data += re;
            size -= re;
            if (pkt->size)
            {
                re = avcodec_send_packet(ctx, pkt);
                if (re < 0) //发送到解码线程失败
                    break;
                //获取解码数据,有可能多帧
                while (re >= 0)
                {
                    //会自己处理上次用过的frame
                    re = avcodec_receive_frame(ctx, frame);
                    if (re < 0)
                        break;
                    auto pframe = frame;
#ifdef IS_HWACCEL
                    if (ctx->hw_device_ctx)
                    {
                        av_hwframe_transfer_data(hw_frame, frame, 0);
                        pframe = hw_frame;
                    }
#endif
                    if (!initWin)
                    {
                        //解码到第一帧后,初始化窗口
                        view->init(pframe->width, pframe->height, (VideoRerdererView::PixFormat) pframe->format);
                        initWin = true;
                    }
                    count++;
                    auto cur = getNowMs();
                    if (cur - begin >= 1000) //1秒计算一次
                    {
                        LOG_DEBUG << "fps: " << count;
                        count = 0;
                        begin = cur;
                    }
                    view->drawAVFrame(pframe);
//                    LOG_DEBUG << frame->format;
                }
            }
        }

    }

    //取出缓冲中的数据
    re = avcodec_send_packet(ctx,nullptr);
    while(re >= 0)
    {
        re = avcodec_receive_frame(ctx,frame);
        if(re < 0)
            break;
        LOG_DEBUG << "cache:" << frame->format;
    }

    av_frame_free(&frame);
    av_frame_free(&hw_frame);
    av_packet_free(&pkt);
    av_parser_close(parser);
    avcodec_free_context(&ctx);
    return -1;
}
