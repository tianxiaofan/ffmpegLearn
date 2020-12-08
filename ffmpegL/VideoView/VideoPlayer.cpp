/**************************************************************************
*   文件名      ：VideoPlayer.cpp
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-12-7
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
#include "VideoPlayer.h"
#include "ui_VideoPlayer.h"

VideoPlayer::VideoPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoPlayer)
{
    ui->setupUi(this);
    startTimer(10);
}

VideoPlayer::~VideoPlayer()
{
    closePlayer();
    delete ui;
}

void VideoPlayer::timerEvent(QTimerEvent* e)
{
    auto f = m_decodec.getFrame();
    if (f && m_view)
    {
        m_view->drawAVFrame(f->frame);
    }
}

void VideoPlayer::closeEvent(QCloseEvent* event)
{
    closePlayer();
}

void VideoPlayer::closePlayer()
{
    m_demux.stop();
    m_decodec.stop();
    if (m_view)
    {
        m_view->close();
        delete m_view;
        m_view = nullptr;
    }
}


bool VideoPlayer::open(const QString& url)
{
    m_demux.stop();
    m_decodec.stop();

    if (!m_demux.open(url.toLatin1().data()))
        return false;
    auto vp = m_demux.copyVideoPara();
    if (!m_decodec.open(vp->para))
    {
        return false;
    }
    m_demux.setNext(&m_decodec);
    if (!m_view)
        m_view = VideoRerdererView::create();

    m_view->setWinId((void*) this->winId());
    if (!m_view->init(vp->para))
    {
        return false;
    }
    m_demux.setSynType(SYN_TYPE::SYN_VIDEO);
    m_demux.start();
    m_decodec.start();
    return true;
}
