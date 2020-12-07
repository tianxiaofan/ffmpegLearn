/**************************************************************************
*   文件名      ：%{Cpp:License:FileName}
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-12-3
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
#include "VideoView.h"
#include "ui_VideoView.h"
#include <QDebug>

#include <QMessageBox>
#include <QMouseEvent>
#include "VVConfig.h"
#include "Add.h"
#include "VVTools.h"



VideoView::VideoView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoView)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    if (!m_vLayout)
    {
        m_vLayout = new QVBoxLayout();
        //边框间隔
        m_vLayout->setContentsMargins(0, 0, 0, 0);
        //元素间隔
        m_vLayout->setSpacing(0);
        m_vLayout->addWidget(ui->head);
        m_vLayout->addWidget(ui->body_widget);
        this->setLayout(m_vLayout);
    }

    if (!m_hLayout)
    {
        m_hLayout = new QHBoxLayout();
        m_hLayout->setContentsMargins(0, 0, 0, 0);
        m_hLayout->setSpacing(0);
        m_hLayout->addWidget(ui->left_widget);
        m_hLayout->addWidget(ui->cames_widget);
        ui->body_widget->setLayout(m_hLayout);
    }

    //先隐藏normal
    ui->btnNormal->setHidden(true);

    //添加右键菜单
    auto m = m_leftMenu.addMenu(C_STR("视图"));
    auto action = m->addAction(C_STR("1窗口"));
    connect(action, &QAction::triggered, this, &VideoView::onView1);
    action = m->addAction(C_STR("4窗口"));
    connect(action, &QAction::triggered, this, &VideoView::onView4);
    action = m->addAction(C_STR("9窗口"));
    connect(action, &QAction::triggered, this, &VideoView::onView9);
    action = m->addAction(C_STR("16窗口"));
    connect(action, &QAction::triggered, this, &VideoView::onView16);

    //显示列表
    refresConfig();
    //默认显示9窗口
    onView(9);

    //启动定时器渲染视频
    startTimer(1);
}

VideoView::~VideoView()
{
    delete ui;
}

void VideoView::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_mouseIsPress)
    {
        QWidget::mouseMoveEvent(event);
        return;
    }
    this->move(event->globalPos() - m_mousePos);
}

void VideoView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPress = true;
        m_mousePos     = event->pos();
    }
}

void VideoView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPress = false;
    }
}

void VideoView::resizeEvent(QResizeEvent* event)
{
    QPoint pos(width() - ui->minMax->width(),ui->minMax->y());
    ui->minMax->move(pos) ;
}

void VideoView::contextMenuEvent(QContextMenuEvent* event)
{
    m_leftMenu.exec(QCursor::pos());
    event->accept();
}

void VideoView::timerEvent(QTimerEvent* event)
{
    for (auto v : m_views)
    {
        if (v)
        {
            //渲染多窗口视频
            v->draw();
        }
    }

}

void VideoView::refresConfig()
{
    auto config = VVConfig::get();
    ui->videoList->clear();
    auto count = config->getSize();
    for (int i = 0; i < count; i++)
    {
        auto pro  = config->getData(i);
        auto item = new QListWidgetItem(QIcon(":/new/prefix1/image/camera.png"), pro.name);
        ui->videoList->addItem(item);
    }
}


void VideoView::on_btnMax_clicked()
{
    showMaximized();
    ui->btnNormal->setHidden(false);
    ui->btnMax->setHidden(true);
}

void VideoView::on_btnNormal_clicked()
{
    showNormal();
    ui->btnNormal->setHidden(true);
    ui->btnMax->setHidden(false);
}

void VideoView::onView1()
{
    onView(1);
}

void VideoView::onView4()
{
    onView(4);
}

void VideoView::onView9()
{
    onView(9);
}

void VideoView::onView16()
{
    onView(16);
}

void VideoView::onView(int count)
{
    //1 2*2 3*3 4*4
    int col = sqrt(count);

    //添加layout
    auto layout = (QGridLayout*) ui->cames_widget->layout();
    if (!layout)
    {
        layout = new QGridLayout();
        ui->cames_widget->setLayout(layout);
    }
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);


    //添加窗口
    for (int i = 0; i < count; i++)
    {
        if (m_views.size() <= i)
        {
            auto w = new VideoWidget(ui->cames_widget);
            w->setStyleSheet("background-color: #515151;");
            m_views.append(w);
        }
        layout->addWidget(m_views.at(i), i / col, i % col);
    }

    //移除多余窗口
    while (m_views.size() > count)
    {
        auto w = m_views.takeLast();
        w->deleteLater();
    }
}

void VideoView::on_btnAdd_clicked()
{
    Add add(ui->left_widget);
    connect(&add, &Add::saveSuccess, this, [=]() { refresConfig(); });
    add.exec();
}




void VideoView::on_btnEdit_clicked()
{
    auto index = ui->videoList->currentIndex().row();
    if (index < 0)
    {
        QMessageBox::information(nullptr, C_STR("错误"), C_STR("请选择要修改项"));
        return;
    }
    Add add(ui->left_widget);
    add.updataData(index);
    connect(&add, &Add::saveSuccess, this, [=]() { refresConfig(); });
    add.exec();
}

void VideoView::on_btnDelete_clicked()
{
    auto index = ui->videoList->currentIndex().row();
    if (index < 0)
    {
        QMessageBox::information(nullptr, C_STR("错误"), C_STR("请选择要修改项"));
        return;
    }
    if (VVConfig::get()->deleteData(index))
    {
        refresConfig();
    }
}
