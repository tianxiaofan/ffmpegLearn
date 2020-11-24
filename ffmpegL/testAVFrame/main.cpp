/**************************************************************************
*   文件名      ：%{Cpp:License:FileName}
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-18
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
#include <QCoreApplication>

extern "C"
{
#include <libavcodec/avcodec.h>
}
#include "Logger.h"

int main(int argc, char *argv[])
{
    Logger::initLog();
    QCoreApplication a(argc, argv);
    LOG_DEBUG << avcodec_configuration();

    //创建framec对象
    auto frame1    = av_frame_alloc();
    frame1->width  = 1280;
    frame1->height = 720;
    frame1->format = AV_PIX_FMT_ARGB;

    //分配空间,默认对齐
    int re = av_frame_get_buffer(frame1, 0);
    if (re != 0)
    {
        char buff[1024] = {0};
        av_strerror(re, buff, sizeof(buff));
        LOG_DEBUG << buff;
    }
    LOG_DEBUG << frame1->linesize[0]; //5120 = 1280*4;ARGB是4个字节,且av_frame_get_buffer 默认32位对齐,
    if (frame1->buf[0])
    {
        LOG_DEBUG << "frame1 ref count = " << av_buffer_get_ref_count(frame1->buf[0]);
    }
    auto frame2 = av_frame_alloc();
    av_frame_ref(frame2,frame1);
    LOG_DEBUG << "frame1 ref count = " << av_buffer_get_ref_count(frame1->buf[0]);
    LOG_DEBUG << "frame2 ref count = " << av_buffer_get_ref_count(frame2->buf[0]);

    //引用计数-1,并将buf清零
    av_frame_unref(frame2);
    LOG_DEBUG << "frame1 ref count = " << av_buffer_get_ref_count(frame1->buf[0]);
    av_frame_free(&frame1);
    av_frame_free(&frame2);

    return a.exec();
}
