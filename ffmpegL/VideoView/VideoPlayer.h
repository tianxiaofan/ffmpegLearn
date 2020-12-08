/**************************************************************************
*   文件名      ：VideoPlayer.h
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
#pragma once

#include <QWidget>

#include "DemuxThread.h"
#include "DecodecThread.h"
#include "VideoRerdererView.h"


namespace Ui {
class VideoPlayer;
}

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

    void timerEvent(QTimerEvent* e) override;
    void closeEvent(QCloseEvent* event) override;
    void closePlayer();

    bool open(const QString& url);

private:
    Ui::VideoPlayer *ui;
    DemuxThread        m_demux;
    DecodecThread      m_decodec;
    VideoRerdererView* m_view = nullptr;
};

