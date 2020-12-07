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
#include "MainWindow.h"

#include <QApplication>

#include "DemuxThread.h"
#include "Logger.h"

int main(int argc, char *argv[])
{
    //    QApplication a(argc, argv);
    //    MainWindow w;
    //    w.show();
    //    return a.exec();
    Logger::initLog();
    DemuxThread dumux;
    dumux.open("test.mp4");
    dumux.start();
    getchar();
    return -1;
}
