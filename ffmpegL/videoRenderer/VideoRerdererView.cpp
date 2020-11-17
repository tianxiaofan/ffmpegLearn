/**************************************************************************
*   文件名      ：VideoRerdererView.cpp
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-17
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
#include "VideoRerdererView.h"
#include "SDLRerderer.h"


VideoRerdererView *VideoRerdererView::create(VideoRerdererView::RendererType type)
{
    switch (type) {
    case RendererType::SDL:
        return new SDLRerderer();
        break;
    default:
        break;
    }
    return nullptr;
}
