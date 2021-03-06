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

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN32

#if defined(VIDEODLL_LIBRARY)
#  define VIDEODLL_EXPORT Q_DECL_EXPORT
#else
#  define VIDEODLL_EXPORT Q_DECL_IMPORT
#endif

#elif Q_OS_LINUX
#define VIDEODLL_EXPORT
#endif
