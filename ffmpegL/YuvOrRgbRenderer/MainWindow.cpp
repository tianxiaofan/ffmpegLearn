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
#include <QFileDialog>
#include "VideoRerdererView.h"
#include <vector>
static std::vector<VideoRerdererView*> renders;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    Logger::initLog();
    ui->setupUi(this);

    connect(this, &MainWindow::sigView, this, &MainWindow::onView);
    //创建两个窗体
    renders.push_back(VideoRerdererView::create());
    renders.push_back(VideoRerdererView::create());
    // 设置窗口
    renders[0]->setWinId(reinterpret_cast<void*>(ui->video1->winId()));
    renders[1]->setWinId(reinterpret_cast<void*>(ui->video2->winId()));

    //启动线程
    m_th = std::thread(&MainWindow::threadMain, this);
}

MainWindow::~MainWindow()
{
    delete ui;
    m_isExit = true;
    m_th.join();
}

void MainWindow::threadMain()
{
    m_isExit = false;
    while (!m_isExit)
    {
        emit sigView();
        customSleep(1);
    }
}
void MainWindow::timerEvent(QTimerEvent* event)
{

}

void MainWindow::resizeEvent(QResizeEvent *ev)
{
//    for (int i = 0; i < renders.size() ; i++ ) {
//        renders[i]->scale(ui->video1->width(),ui->video1->height());
//    }
}

void MainWindow::onOpen1()
{
    open(0);
}

void MainWindow::onOpen2()
{
    open(1);
}

void MainWindow::open(int i)
{
    QFileDialog fopen;
    auto        filename = fopen.getOpenFileName();
    if (filename.isEmpty())
        return;
    LOG_INFO << filename;
    if (!renders[i]->openFile(filename.toStdString()))
    {
        return;
    }

    int     w, h;
    QString pix;
    if (i == 0)
    {
        w   = ui->spinBoxWidth_1->value();
        h   = ui->spinBoxHeight_1->value();
        pix = ui->comboBox_1->currentText();
        renders[i]->scale(ui->video1->width(),ui->video1->height());
    }
    else if (i == 1)
    {
        w   = ui->spinBoxWidth_2->value();
        h   = ui->spinBoxHeight_2->value();
        pix = ui->comboBox_2->currentText();
        renders[i]->scale(ui->video2->width(),ui->video2->height());
    }
    VideoRerdererView::PixFormat format = VideoRerdererView::YUV420P;
    if (pix == "YUV420P")
    {
    }
    else if (pix == "RGBA")
    {
        format = VideoRerdererView::RGBA;
    }
    else if (pix == "ARGB")
    {
        format = VideoRerdererView::ARGB;
    }
    else if (pix == "BGRA")
    {
        format = VideoRerdererView::BGRA;
    }
    else if(pix == "ABGR")
    {
        format = VideoRerdererView::ABGR;
    }
    else if (pix == "RGB24") {
        format = VideoRerdererView::RGB24;
    }
    else if (pix == "BGR24") {
        format = VideoRerdererView::BGR24;
    }
    //初始化窗口和材质
    renders[i]->init(w, h, format);


}

void MainWindow::onView()
{
    //存放上次渲染的时间戳
    static int last_pts[32] = {0};
    static int fps_arr[32]  = {0};
    fps_arr[0]              = ui->spinBoxFps_1->value();
    fps_arr[1]              = ui->spinBoxFps_2->value();
    for (int i = 0; i < renders.size(); i++)
    {
        if (fps_arr[i] <= 0)
            continue;

        //判断是否到了可渲染时间,需要间隔时间
        int ms = 1000 / fps_arr[i];
        if (getNowMs() - last_pts[i] <= ms)
            continue;
        last_pts[i] = getNowMs();

       auto frame =  renders[i]->read();
       if(!frame)
           continue;
       renders[i]->drawAVFrame(frame);
       if(i == 0)
       {
           ui->labelFps_1->setText(QString("fps:%1").arg(renders[i]->getFps()));
       }
       else {
           ui->labelFps_2->setText(QString("fps:%1").arg(renders[i]->getFps()));
       }
    }
}
