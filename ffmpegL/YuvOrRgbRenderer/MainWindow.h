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
#pragma once

#include <QMainWindow>
#include <thread>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //线程函数用于刷新视频
    void threadMain();
protected:
    void timerEvent(QTimerEvent* event) override;
    void resizeEvent(QResizeEvent* ev) override;

signals:
    void sigView();

private slots:
    void onOpen1();
    void onOpen2();
    void open(int i);
    void onView(); //显示
private:
    Ui::MainWindow* ui;
    std::thread     m_th;
    bool            m_isExit = false;
};
