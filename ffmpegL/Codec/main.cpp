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

#include <fstream>
#include <QApplication>
using namespace std;
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavcodec/dxva2.h>
}
#include "Decodec.h"
#include "Logger.h"
#include "VideoRerdererView.h"

#define ZERO_COPY

#ifdef ZERO_COPY
struct DXVA2DevicePriv
{
    HMODULE d3dlib;
    HMODULE dxva2lib;

    HANDLE device_handle;

    IDirect3D9*       d3d9;
    IDirect3DDevice9* d3d9device;
};
static HWND hwnd = nullptr;
void        drawFrame(AVFrame* frame, AVCodecContext* c)
{
    if (!frame->data[3] || !c)
        return;

    auto        surface = (IDirect3DSurface9*) frame->data[3];
    auto        ctx     = (AVHWDeviceContext*) c->hw_device_ctx->data;
    auto        priv    = (DXVA2DevicePriv*) ctx->user_opaque;
    auto        d3d9d   = priv->d3d9device;
    static RECT sourView;
    sourView.left   = 0;
    sourView.right  = frame->width;
    sourView.top    = 0;
    sourView.bottom = frame->height;
    d3d9d->Present(nullptr, nullptr, hwnd, 0);
    static IDirect3DSurface9* backBuffer = nullptr;
    if(!backBuffer)
        d3d9d->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer);
    d3d9d->StretchRect(surface,&sourView,backBuffer,nullptr,D3DTEXF_LINEAR);
}
#endif

//ffmpeg.exe -i .\binghe.mp4 -ss 8:00 -t 1:00 h264out.h264
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow   w;
    w.show();
    Logger::initLog();
#ifndef ZERO_COPY
    auto view = VideoRerdererView::create();
#endif

#ifdef ZERO_COPY
    hwnd = (HWND)w.winId();
#endif
    bool     initWin  = false;
    string   filename = "f:/jqdz/h264out.h264";
    ifstream ifs;
    ifs.open(filename, ios::binary);
    if (!ifs)
        return -1;

    unsigned char buff[4096] = {0};
    AVCodecID     codecId    = AV_CODEC_ID_H264;

    Decodec dec;
    auto    ctx = dec.cteate(codecId, false);
    dec.setContext(ctx);

    dec.initHW();

    dec.open();

    auto pkt = av_packet_alloc();

    auto frame = av_frame_alloc();

    //分割上下文
    auto parser = av_parser_init(codecId);

    auto begin = getNowMs(); //获取开始时间
    int  count = 0;          //解码统计
    while (!ifs.eof())
    {
        ifs.read((char*) buff, sizeof(buff));
        int size = ifs.gcount();
        if (size <= 0)
        {
            break;
        }

        if (ifs.eof())
        {
            ifs.clear();
            ifs.seekg(0, ios::beg);
        }

        auto data = buff;
        while (size > 0)
        {
            //通过0001截断,输出到pkt
            auto re =
                av_parser_parse2(parser, ctx, &pkt->data, &pkt->size, data, size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            data += re;
            size -= re;
            if (pkt->size)
            {
                if (!dec.send(pkt))
                    break;
                //获取解码数据,有可能多帧
#ifndef ZERO_COPY
                while (dec.recv(frame,false))
                {
                    if (!initWin)
                    {
                        //解码到第一帧后,初始化窗口
                        view->init(frame->width, frame->height, (VideoRerdererView::PixFormat) frame->format);
                        initWin = true;
                    }
                    view->drawAVFrame(frame);

#endif
#ifdef ZERO_COPY
                    while (dec.recv(frame,false))
                    {
                        drawFrame(frame, ctx);
#endif
                    count++;
                    auto cur = getNowMs();
                    if (cur - begin >= 1000) //1秒计算一次
                    {
                        LOG_DEBUG << "fps: " << count;
                        count = 0;
                        begin = cur;
                    }
                }
            }
        }
    }

    //取出缓冲中的数据d
    auto frames = dec.end();
    for (auto f : frames)
    {
#ifndef ZERO_COPY
        view->drawAVFrame(f);
#endif
        dec.free(f);
    }

    dec.free(frame);
    dec.free(pkt);
    av_parser_close(parser);
    dec.close();
    return a.exec();
}
