/**************************************************************************
*   鏂囦欢鍚     锛FILENAME%
*   =======================================================================
*   鍒寤鑰   锛氱敯灏忓竼
*   鍒涘缓鏃ユ湡    锛YEAR%-11-23
*   閭  绠   锛99131808@qq.com
*   QQ    		锛99131808
*   鍔熻兘鎻忚堪    锛
*   浣跨敤璇存槑    锛
*   ======================================================================
*   淇敼鑰     锛
*   淇敼鏃ユ湡    锛
*   淇敼鍐呭    锛
*   ======================================================================
*
***************************************************************************/

//获取YUV,,注意,在此处可能会出问题,我原mp4为1280*720的加上-s转800*600后,以下程序中文件打
//开后会read不到数据,暂时不知道是什么原因,但我后面用的-s 400*300生成的yuv文件确没有问题
// PS F:\jqdz> .\ffmpeg.exe -i .\binghe.mp4 -ss 5:00 -t 1:00 -s 800x600 800_600_25_yuvout.yuv
#include "MainWindow.h"

#include <QApplication>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <fstream>
#include <iostream>
#include <QThread>

#include "Logger.h"
using namespace std;

#define YUV_FILE "f:/jqdz/output.yuv"
#define RGB_FILE "f:/jqdz/800_600_25_rgbout.rgb"

int main(int argc, char* argv[])
{
    //    QApplication a(argc, argv);
    //    MainWindow w;
    //    w.show();
    //    return a.exec();
    Logger::initLog();
    int width      = 400;
    int height     = 300;
    int rgb_width  = 800;
    int rgb_height = 600;
    // YUV420P 平面存储 yyyy uu vv
    unsigned char* yuv[3]          = {0};
    int            yuv_linesize[3] = {width, width / 2, width / 2};
    yuv[0]                         = new unsigned char[width * height]; //Y
    yuv[1]                         = new unsigned char[width * height / 4]; //U
    yuv[2]                         = new unsigned char[width * height / 4]; //V

    //RGBA交叉存储 rgba rgba
    unsigned char* rgba          = new unsigned char[rgb_width * rgb_height * 4];
    int            rgba_linesize = rgb_width * 4;

    ifstream ifs;
    ifs.open(YUV_FILE, ios::binary);
    if (!ifs)
    {
        cerr << "open " << YUV_FILE << " failed!" << endl;
        return -1;
    }
    ofstream ofs;
    ofs.open(RGB_FILE, ios::binary);
    if (!ofs)
    {
        cerr << "open " << YUV_FILE << " failed!" << endl;
        return -1;
    }

    cout << "yuv open " << endl;
    //////////////////////////////////////////
    //// YUV720转RGBA
    SwsContext* yuv2rgb = nullptr;
    for (int i = 0 ; i < 100; i++)
    {
        // 读取YUV帧
        ifs.read((char*) yuv[0], width * height);
        ifs.read((char*) yuv[1], width * height / 4);
        ifs.read((char*) yuv[2], width * height / 4);
        if (ifs.gcount() == 0)
            break;

        //YUV转RGBA
        //上下文件创建和获取
        yuv2rgb = sws_getCachedContext(
            yuv2rgb, width, height, AV_PIX_FMT_YUV420P, rgb_width, rgb_height, AV_PIX_FMT_RGBA, SWS_BILINEAR, 0, 0, 0);

        if (!yuv2rgb)
        {
            cerr << "sws_getCachedContext failed!" << endl;
            return -1;
        }

        unsigned char* data[1];
        data[0]      = rgba;
        int lines[1] = {rgba_linesize};
        int re       = sws_scale(yuv2rgb,
                           yuv,          //输入数据
                           yuv_linesize, //输入数据行字节数
                           0,
                           height, //输入高度
                           data,   //输出数据
                           lines);
        cout << re << " " << flush;
        ofs.write((char*) rgba, rgb_width * rgb_height * 4);
    }

    ofs.close();
    ifs.close();

    cout << endl;

    // rgba 转yuv420
    SwsContext* rgba2yuv = nullptr;
    ifs.open(RGB_FILE, ios::binary);
    while (1)
    {
        //读取rgb数据
        ifs.read((char*) rgba, rgb_height * rgb_width * 4);
        if (ifs.gcount() == 0)
            break;

        rgba2yuv = sws_getCachedContext(
            rgba2yuv, rgb_width, rgb_height, AV_PIX_FMT_RGBA, width, height, AV_PIX_FMT_YUV420P, SWS_BICUBLIN, 0, 0, 0);
        if (!rgba2yuv)
        {
            cerr << "sws error" << endl;
        }
        unsigned char* rgb_data[1];
        rgb_data[0]      = rgba;
        int rgb_lines[1] = {rgba_linesize};
        int re           = sws_scale(rgba2yuv, rgb_data, rgb_lines, 0, rgb_height, yuv, yuv_linesize);
        cout << re << " " << flush;
    }

    delete yuv[0];
    delete yuv[1];
    delete yuv[2];
    delete[] rgba;
}
