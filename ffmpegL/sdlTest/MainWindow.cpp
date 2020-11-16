/**************************************************************************
*   文件名      ：%{Cpp:License:FileName}
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-16
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

#include <sdl/SDL.h>
#include <QDebug>

#undef main
static SDL_Window*    sdl_screen  = nullptr;
static SDL_Renderer*  sdl_render  = nullptr;
static SDL_Texture*   sdl_texture = nullptr;
static int            sdl_w = 0, sdl_h = 0;
static unsigned char* rgb      = nullptr;
static int            pix_size = 4;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SDL_Init(SDL_INIT_VIDEO);
    sdl_w = ui->video->width();
    sdl_h = ui->video->height();
    qDebug() << sdl_w << sdl_h;
    //创建窗口
    sdl_screen = SDL_CreateWindowFrom((void *)ui->video->winId());
    //创建渲染器
    sdl_render = SDL_CreateRenderer(sdl_screen, -1, SDL_RENDERER_ACCELERATED);
    //创建材质
    sdl_texture = SDL_CreateTexture(sdl_render,
                      SDL_PIXELFORMAT_ARGB8888,
                      SDL_TEXTUREACCESS_STREAMING,
                      sdl_w,
                      sdl_h);
    rgb = new unsigned char[sdl_h * sdl_w * pix_size];

    startTimer(10);
}

MainWindow::~MainWindow()
{
    delete ui;
}

static unsigned char tmp = 255;
void                 MainWindow::timerEvent(QTimerEvent* event)
{
    for (int j = 0; j < sdl_h; j++)
    {
        int b = j * sdl_w * pix_size;
        for (int i = 0; i < sdl_w * pix_size; i += pix_size)
        {
            rgb[b + i]     = 0;
            rgb[b + i + 1] = 255;
            rgb[b + i + 2] = tmp;
            rgb[b + i + 3] = 0;
        }
    }
    SDL_UpdateTexture(sdl_texture, nullptr, rgb, sdl_w * pix_size);
    SDL_RenderClear(sdl_render);
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = sdl_w;
    rect.h = sdl_w;
    SDL_RenderCopy(sdl_render, sdl_texture, nullptr, &rect);
    SDL_RenderPresent(sdl_render);
    tmp--;
}
