/**************************************************************************
*   文件名      ：%{Cpp:License:FileName}
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-17
*   邮   箱    ：499131808@qq.com
*   公   司    ：西安英杰伟成航空科技有限公司
*   功能描述    ：
*   使用说明    ：任何人或机构要使用、修改、发布本源代码,都必须要获得西安英杰伟成航空科技有限公司的授权
*   ======================================================================
*   修改者      ：
*   修改日期    ：
*   修改内容    ：
*   ======================================================================
*
***************************************************************************/
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Logger.h"
#include <QDebug>
#include <sdl/SDL.h>
#include <fstream>

//mp4转yuv -ss 起始位置,-t 要转换的时长
//ffmpeg.exe -i .\binghe.mp4 -ss 5:00 -t 1:00 yuvout.yuv

static SDL_Window*    sdl_screen  = nullptr;
static SDL_Renderer*  sdl_render  = nullptr;
static SDL_Texture*   sdl_texture = nullptr;
static int            sdl_w = 0, sdl_h = 0;
static unsigned char* yuv      = nullptr;
static int            pix_size = 2;
static std::ifstream  yuvFile;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    Logger::initLog();

    ui->setupUi(this);
    //自己找视频转换
    yuvFile.open("f:/jqdz/yuvout.yuv", std::ios::binary);//1280*720
    if (!yuvFile)
    {
        LOG_WARN << "open yuv failed!";
        return;
    }
    LOG_INFO << "open yuv success!";
    sdl_w = 1280;
    sdl_h = 720;
    ui->video->resize(sdl_w, sdl_h);
    SDL_Init(SDL_INIT_VIDEO);
    LOG_INFO << sdl_w << sdl_h;
    //创建窗口
    sdl_screen = SDL_CreateWindowFrom((void*) ui->video->winId());
    //创建渲染器
    sdl_render = SDL_CreateRenderer(sdl_screen, -1, SDL_RENDERER_ACCELERATED);

    //创建材质
    sdl_texture = SDL_CreateTexture(sdl_render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, sdl_w, sdl_h);
    yuv         = new unsigned char[sdl_h * sdl_w * pix_size];


    startTimer(40);
}

MainWindow::~MainWindow()
{
    delete ui;
    yuvFile.close();
}

void MainWindow::timerEvent(QTimerEvent* event)
{
    yuvFile.read((char*) yuv, sdl_w * sdl_h * 1.5);
    //yuv面平存储
    //yyyyyyyyuuvv
    SDL_UpdateTexture(sdl_texture, nullptr, yuv, sdl_w);//一行Y的字节数
    SDL_RenderClear(sdl_render);
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = sdl_w;
    rect.h = sdl_h;
    SDL_RenderCopy(sdl_render, sdl_texture, nullptr, &rect);
    SDL_RenderPresent(sdl_render);
}
