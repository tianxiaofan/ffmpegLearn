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
#include "VideoRerdererView.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

#include <QLabel>
#include <QSpinBox>

//mp4转yuv -ss 起始位置,-t 要转换的时长
//ffmpeg.exe -i .\binghe.mp4 -ss 5:00 -t 1:00 yuvout.yuv

static int            sdl_w = 0, sdl_h = 0;
static unsigned char* yuv      = nullptr;
static int            pix_size = 2;
static std::ifstream  yuvFile;

static VideoRerdererView* videoView = nullptr;

static AVFrame*  yuvframe    = nullptr;
static long long yuvFileSize = 0;
static QLabel*   labelFps    = nullptr;
static QSpinBox* spinFps     = nullptr;
int              defaultFps  = 25;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    Logger::initLog();

    ui->setupUi(this);

    connect(this, &MainWindow::sigView, this, &MainWindow::onView);

    //自己找视频转换
    yuvFile.open("f:/jqdz/yuvout.yuv", std::ios::binary);//1280*720
    if (!yuvFile)
    {
        LOG_WARN << "open yuv failed!";
        return;
    }
    yuvFile.seekg(0, std::ios::end); //移动到文件结尾位置
    yuvFileSize = yuvFile.tellg();//文件指针位置
        yuvFile.seekg(0,std::ios::beg);//移动到开头

    LOG_INFO << "open yuv success!";
    sdl_w = 1280;
    sdl_h = 720;
    ui->video->resize(sdl_w, sdl_h);

    yuv         = new unsigned char[sdl_h * sdl_w * pix_size];
    videoView = VideoRerdererView::create();
    videoView->init(sdl_w, sdl_h, VideoRerdererView::YUV420P ,reinterpret_cast <void*>(ui->video->winId()));

    yuvframe         = av_frame_alloc();
    yuvframe->width  = sdl_w;
    yuvframe->height = sdl_h;
    yuvframe->format = AV_PIX_FMT_YUV420P;
    auto re          = av_frame_get_buffer(yuvframe, 0); //生成frame存储空间.默认32位对齐
    if (re != 0)
    {
        char b[1024] = {0};
        av_strerror(re, b, sizeof(b));
        LOG_DEBUG << b;
    }
    labelFps = new QLabel(this);
    labelFps->move(50,200);
    spinFps = new QSpinBox(this);
    spinFps->move(50,400);
    spinFps->setValue(25);
    spinFps->setRange(1,300);

//    startTimer(40);
    //启动线程
    m_th = std::thread(&MainWindow::threadMain,this);
}

MainWindow::~MainWindow()
{
    delete ui;
    yuvFile.close();

    //等待渲染线程退出
    m_isExit = true;
    m_th.join();
}

void MainWindow::threadMain()
{
    m_isExit = false;
    while (!m_isExit)
    {
       emit sigView();
       if(defaultFps > 0)
       {
           customSleep(1000 / defaultFps);
       }
       else {
           customSleep(10);
       }
    }
}

void MainWindow::timerEvent(QTimerEvent* event)
{
    //    yuvFile.read((char*) yuv, sdl_w * sdl_h * 1.5);
    yuvFile.read((char*) yuvframe->data[0], sdl_w * sdl_h);//Y
    yuvFile.read((char*) yuvframe->data[1], sdl_w * sdl_h / 4); //U
    yuvFile.read((char*) yuvframe->data[2], sdl_w * sdl_h / 4); //V
    if(yuvFile.tellg() == yuvFileSize) //读到文件尾
    {
        yuvFile.seekg(0,std::ios::beg);//移动到开头
    }
    if (videoView->isExit())
    {
        videoView->close();
        exit(0);
    }
//    videoView->draw(yuv);
    videoView->drawAVFrame(yuvframe);
}

void MainWindow::resizeEvent(QResizeEvent *ev)
{
    ui->video->resize(ui->centralwidget->size());
    videoView->scale(ui->centralwidget->width(),ui->centralwidget->height());
}

void MainWindow::onView()
{
    yuvFile.read((char*) yuvframe->data[0], sdl_w * sdl_h); //Y
    yuvFile.read((char*) yuvframe->data[1], sdl_w * sdl_h / 4); //U
    yuvFile.read((char*) yuvframe->data[2], sdl_w * sdl_h / 4); //V
    if(yuvFile.tellg() == yuvFileSize) //读到文件尾
    {
        yuvFile.seekg(0,std::ios::beg);//移动到开头
    }
    if (videoView->isExit())
    {
        videoView->close();
        exit(0);
    }
    //    videoView->draw(yuv);
    videoView->drawAVFrame(yuvframe);
    labelFps->setText(QString("fps: %1").arg(videoView->getFps()));
    defaultFps = spinFps->value();
}
