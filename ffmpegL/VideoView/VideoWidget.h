/**************************************************************************
*   文件名      ：VideoWidget.h
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
#pragma once

#include <QWidget>

class DecodecThread;
class DemuxThread;
class VideoRerdererView;
class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget* parent = nullptr);
    ~VideoWidget();

    //拖拽进入
    void dragEnterEvent(QDragEnterEvent* event) override;
    //拖拽松开
    void dropEvent(QDropEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

    //打开视频
    bool open(const QString& url);


    void draw();


signals:

private:
    DecodecThread*     decodec = nullptr;
    DemuxThread*       demux   = nullptr;
    VideoRerdererView* view    = nullptr;

};

