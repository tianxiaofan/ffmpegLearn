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
#include "thread"
extern "C"
{
#include <libavcodec/avcodec.h>
}

using namespace std;
void customSleep(unsigned int m)
{
    auto begin = clock();
    for (int i = 0; i < m; i++)
    {
        std::this_thread::sleep_for(1ms);
        if ((clock() - begin) / (CLOCKS_PER_SEC / 1000) >= m)
        {
            break;
        }
    }
}
VideoRerdererView* VideoRerdererView::create(VideoRerdererView::RendererType type)
{
    switch (type)
    {
    case RendererType::SDL:
        return new SDLRerderer();
        break;
    default:
        break;
    }
    return nullptr;
}

bool VideoRerdererView::drawAVFrame(AVFrame* frame)
{
    if (!frame || !frame->data[0])
        return false;

    //计算帧率
    m_count++;
    if (m_beginMs <= 0)
    {
        m_beginMs = clock();
    }
    else if ((clock() - m_beginMs) / (CLOCKS_PER_SEC / 1000) >= 1000) //1 秒计算一次fps
    {
        m_sfps    = m_count;
        m_count   = 0;
        m_beginMs = clock();
    }
    switch (frame->format) {
    case AV_PIX_FMT_YUV420P:
        return draw(frame->data[0],
                    frame->linesize[0],
                    frame->data[1],
                    frame->linesize[1],
                    frame->data[2],
                    frame->linesize[2]);
        break;
    case AV_PIX_FMT_BGRA:
        return draw(frame->data[0],frame->linesize[0]);
        break;
    default:
        break;
    }
    return true;
}
