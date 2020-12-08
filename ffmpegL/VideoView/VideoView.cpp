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
#include <QDir>
#include <QDateTime>
#include <QMap>
#include "VVConfig.h"
#include "Add.h"
#include "VVTools.h"
#include "Logger.h"
#include "VideoPlayer.h"


struct VideoProperty
{
    QString   filePath;
    QDateTime fileTime;
};

//存储视频的日期
static QMap<QDate, QVector<VideoProperty>> videoFiles;

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
        m_hLayout->addWidget(ui->left_widget);//流列表
        m_hLayout->addWidget(ui->cames_widget);//预览窗口
        m_hLayout->addWidget(ui->playback_widget); //回放窗口
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
    auto start_rec = m_leftMenu.addAction(C_STR("全部开始录制"));
    connect(start_rec, &QAction::triggered, this, &VideoView::startRecord);
    auto stop_rec = m_leftMenu.addAction(C_STR("全部停止录制"));
    connect(stop_rec, &QAction::triggered, this, &VideoView::stopRecord);

    //显示列表
    refresConfig();
    //默认显示9窗口
    onView(9);

    //启动定时器渲染视频
    startTimer(1);

    //默认显示预览
    on_btnView_clicked();
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

void VideoView::startRecord()
{
    //容错功能,开始前先清理
    stopRecord();
    ui->label_state->setText(C_STR("录制中..."));
    auto count = VVConfig::get()->getSize();
    for (int i = 0; i < count; ++i)
    {
        auto v    = VVConfig::get()->getData(i);
        auto path = QString(v.save_path + "/%1/").arg(i);
        QDir dir;
        dir.mkpath(path);
        VideoRecord* rec = new VideoRecord();
        rec->setRtspUrl(v.url);
        rec->setSavePath(path);
        rec->setFileTime(10);
        rec->start();
        m_rec.append(rec);
    }
}

void VideoView::stopRecord()
{
    ui->label_state->setText(C_STR("监控中..."));
    for (auto rec : m_rec)
    {
        if (rec->isRunning())
        {
            rec->requestInterruption();
            rec->quit();
            rec->wait();
        }
        delete rec;
    }
    m_rec.clear();
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

void VideoView::on_btnView_clicked()
{
    ui->cames_widget->show();
    ui->playback_widget->setHidden(true);
}

void VideoView::on_btnPlay_clicked()
{
    ui->cames_widget->setHidden(true);
    ui->playback_widget->show();
}

// 视频流选中的点击
void VideoView::on_videoList_clicked(const QModelIndex& index)
{
    auto v = VVConfig::get()->getData(index.row());
    if (v.name.isEmpty())
        return;

    //获取相机路径
    auto path = QString(v.save_path + "/%1/").arg(index.row());

    //遍历此目录
    QDir dir(path);
    if (!dir.exists())
        return;
    //获取文件列表
    QStringList filters = {"*.mp4", "*.avi"};
    dir.setNameFilters(filters);
    auto files = dir.entryInfoList();
    //清空日历
    ui->calendarWidget->clearDate();
    //清空map
    videoFiles.clear();

    for (auto file : files)
    {
        //file: .\video\0\video_2020_12_07_11_46_59.mp4
        auto filename = file.baseName(); //filename: video_2020_12_07_11_46_59
        filename      = filename.remove(0, filename.indexOf('_') + 1); //filename:2020_12_07_11_46_59
        auto dt       = QDateTime::fromString(filename, "yyyy_MM_dd_hh_mm_ss");
        ui->calendarWidget->addDate(dt.date());

        //记录存储到map,供日历使用
        VideoProperty vp;
        vp.filePath = file.absoluteFilePath();
        vp.fileTime = dt;
        videoFiles[dt.date()].push_back(vp);
    }
    ui->calendarWidget->updateC();
}

//选择日期
void VideoView::on_calendarWidget_clicked(const QDate& date)
{
    LOG_INFO << date;
    auto vp = videoFiles.value(date);
    ui->listWidget_time->clear();
    for (auto v : vp)
    {
        auto item = new QListWidgetItem(v.fileTime.time().toString());
        item->setData(Qt::UserRole, v.filePath); //存储用户数据到item中
        ui->listWidget_time->addItem(item);
    }
}


void VideoView::on_listWidget_time_activated(const QModelIndex& index)
{
    auto item = ui->listWidget_time->item(index.row());
    if (!item)
        return;

    auto               path = item->data(Qt::UserRole).toString();
    path = "f:/jqdz/binghe.mp4";
    static VideoPlayer player;
    player.open(path);
    player.show();
}
