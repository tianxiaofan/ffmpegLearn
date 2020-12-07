/**************************************************************************
*   文件名      ：VideoWidget.cpp
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-12-4
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
#include "VideoWidget.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QListWidget>

#include "DemuxThread.h"
#include "DecodecThread.h"
#include "VideoRerdererView.h"

#include "VVConfig.h"


VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
    //设置支持拖拽
    this->setAcceptDrops(true);
}

VideoWidget::~VideoWidget()
{
    if (demux)
    {
        demux->stop();
        delete demux;
        demux = nullptr;
    }
    if (decodec)
    {
        decodec->stop();
        delete decodec;
        decodec = nullptr;
    }

    if (view)
    {
        view->close();
        delete view;
        view = nullptr;
    }
}


void VideoWidget::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void VideoWidget::dropEvent(QDropEvent* event)
{
    //拿到url等数据
    auto w = qobject_cast<QListWidget*>(event->source());
    if (w)
    {
        auto index = w->currentIndex().row();
        auto v     = VVConfig::get()->getData(index);
        open(v.url);
    }
}

void VideoWidget::paintEvent(QPaintEvent* event)
{
    //样式表 添加
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

bool VideoWidget::open(const QString& url)
{
    //解封装
    if (!demux)
    {
        demux = new DemuxThread();
    }
    std::vector<DictionaryOpt> dic;
    dic.push_back({"fflags","nobuffer"});
    dic.push_back({"flags","low_delay"});
    dic.push_back({"rtsp_transport","udp"});
    dic.push_back({"probesize","4096"});
    dic.push_back({"max_delay","5000000"});
    dic.push_back({"buffer_size","8388608"});
//    dic.push_back({"timeout","5000000"});
    if (!demux->open(url.toLatin1().data(),dic))
    {
        return false;
    }

    //解码
    if(!decodec)
        decodec = new DecodecThread();

    auto para = demux->copyVideoPara();
    if (!decodec->open(para->para))
    {
        return false;
    }
    demux->setNext(decodec);

    //渲染
    if(!view)
        view = VideoRerdererView::create();
    view->setWinId((void*) this->winId());
    view->init(para->para);

    //启动解封装和解码线程
    demux->start();
    decodec->start();

    return true;
}

void VideoWidget::draw()
{
    if (!demux || !decodec || !view)
        return;
    auto f = decodec->getFrame();
    if(!f)
        return;
    view->drawAVFrame(f->frame);
    view->scale(this->width(),this->height());
}
