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
#include "MainWindow.h"

#include <QApplication>
#include <iostream>
#include <ctime>
#include <thread>
#include <QTimer>
#include <Windows.h>
#include <QThread>
#include <QEventLoop>
#include <QWaitCondition>
#include <QReadWriteLock>
#include <QMutex>
//测试sleep
//#define TEST_C11_SLEEP
//#define TEST_Qt_SLEEP
//#define TEST_WIAT
#define NO_TEST
//#define CUSTOM_SLEEP

using namespace std;
void customQtSleep(unsigned int m)
{
    //    static QEventLoop loop;
    static QWaitCondition wait;
    static QMutex         mutex;
    auto                  begin = clock();
    for (int i = 0; i < m; i++)
    {
        //        QTimer::singleShot(1,Qt::PreciseTimer,&loop,&QEventLoop::quit);
        //        loop.exec();
        mutex.lock();
        wait.wait(&mutex, m);
        mutex.unlock();
        if ((clock() - begin) / (CLOCKS_PER_SEC / 1000) >= m)
        {
            break;
        }
    }
}

int main(int argc, char *argv[])
{
#ifdef TEST_Qt_SLEEP
    //测试wait,事件超时帧率
    int  fps    = 0;
    auto begin  = clock();
    for (;;)
    {
        fps++;
        auto tmpc = clock();
        QThread::usleep(10*1000);
        cout << clock() - tmpc << " " << flush;
        if ((clock() - begin) / (CLOCKS_PER_SEC / 1000) > 1000)
        {
            cout << "wait fps:" << fps << endl;
            break;
        }
    }
#endif


#ifdef TEST_WIAT
    //测试wait,事件超时帧率
    auto handle = CreateEvent(NULL, false, false, nullptr);
    int  fps    = 0;
    auto begin  = clock();
    for (;;)
    {
        fps++;
        auto tmpc = clock();
        WaitForSingleObject(handle, 10); //等待10ms超时
        cout << clock() - tmpc << " " << flush;
        if ((clock() - begin) / (CLOCKS_PER_SEC / 1000) > 1000)
        {
            cout << "wait fps:" << fps << endl;
            break;
        }
    }
#endif

#ifdef TEST_C11_SLEEP

    //测试c++11的sleep
    auto begin = clock(); //开始时间,cpu跳数
    int  fps   = 0;
    for (;;)
    {
        fps++;
        auto tmpc = clock();
        this_thread::sleep_for(10ms);
        cout << clock() - tmpc << " " << flush;
        //1秒后开始统计,CLOCKS_PER_SEC:cpu每秒跳数
        if ((clock() - begin) / (CLOCKS_PER_SEC / 1000) > 1000)
        {
            cout << "c++11 sleep for fps:" << fps << endl;
            break;
            ;
        }
    }
#endif

#ifdef CUSTOM_SLEEP
    auto begin = clock(); //开始时间,cpu跳数
    int  fps   = 0;
    for (;;)
    {
        fps++;
        auto tmpc = clock();
        customSleep(10);
        cout << clock() - tmpc << " " << flush;
        //1秒后开始统计,CLOCKS_PER_SEC:cpu每秒跳数
        if ((clock() - begin) / (CLOCKS_PER_SEC / 1000) > 1000)
        {
            cout << "sleep for fps:" << fps << endl;
            break;
            ;
        }
    }
#endif

#ifdef NO_TEST
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
#endif
}
