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
#pragma once

#include <QMenu>
#include <QVBoxLayout>
#include <QWidget>
#include <QVector>
#include "VideoWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class VideoView; }
QT_END_NAMESPACE

class VideoView : public QWidget
{
    Q_OBJECT

public:
    VideoView(QWidget *parent = nullptr);
    ~VideoView();

    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

    //读取配置项目
    void refresConfig();

private slots:
    void on_btnMax_clicked();

    void on_btnNormal_clicked();

    void onView1();
    void onView4();
    void onView9();
    void onView16();
    void onView(int count);

    void on_btnAdd_clicked();

    void on_btnEdit_clicked();

    void on_btnDelete_clicked();

private:
    Ui::VideoView *ui;
    bool            m_mouseIsPress = false;
    QPoint          m_mousePos;
    QMenu           m_leftMenu;
    QVBoxLayout*    m_vLayout = nullptr;
    QHBoxLayout*    m_hLayout = nullptr;
    QList<VideoWidget*> m_views;
    int             m_list_max;
};
