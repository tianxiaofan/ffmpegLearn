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

#include <QApplication>
#include <QDir>
#include "VVConfig.h"
#include "Logger.h"
#include "VideoRecord.h"

const QString save_path = "./video";
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    Logger::initLog();
    VideoView w;
    w.show();

    QDir               dir;
    dir.mkpath(save_path);

    static VideoRecord record;
    record.setRtspUrl("rtsp://192.168.1.247/0");
    record.setSavePath(save_path);
    record.start();

    return a.exec();
}
