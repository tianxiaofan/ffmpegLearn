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
#include "Logger.h"
using namespace std;


VideoRerdererView::~VideoRerdererView()
{
    if (m_cache)
    {
        delete[] m_cache;
        m_cache = nullptr;
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

bool VideoRerdererView::init(AVCodecParameters* para)
{
    if (!para)
        return false;
    auto fmt = (PixFormat) para->format;
    switch (para->format)
    {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
        fmt = YUV420P;
        break;
    default:
        break;
    }
    return init(para->width,para->height,fmt);
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
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUV420P:
        return draw(frame->data[0],
                    frame->linesize[0],
                    frame->data[1],
                    frame->linesize[1],
                    frame->data[2],
                    frame->linesize[2]);
        break;
    case AV_PIX_FMT_NV12:
        if (!m_cache)
        {
            m_cache = new unsigned char[4096 * 2160 * 1.5];
        }
        if (frame->linesize[0] == frame->width)
        {
            memcpy(m_cache, frame->data[0], frame->linesize[0] * frame->height); //Y
            memcpy(m_cache + frame->linesize[0] * frame->height,
                   frame->data[1],
                   frame->linesize[1] * frame->height / 2); //UV
        }
        else //一行一行复制
        {
            for (int i = 0; i < frame->height; i++) //Y
            {
                memcpy(m_cache + i * frame->width, frame->data[0] + i * frame->linesize[0], frame->width);
            }
            for (int i = 0; i < frame->height / 2; i++) //UV
            {
                auto p = m_cache + frame->height * frame->width;//移位Y
                memcpy(p+i*frame->width,frame->data[1] + i*frame->linesize[1],frame->width);
            }
        }
        return draw(m_cache,frame->linesize[0]);
        break;
    case AV_PIX_FMT_ARGB:
    case AV_PIX_FMT_RGBA:
    case AV_PIX_FMT_ABGR:
    case AV_PIX_FMT_BGRA:
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_BGR24:
        return draw(frame->data[0],frame->linesize[0]);
        break;
    default:
        break;
    }
    return true;
}

bool VideoRerdererView::openFile(string filePath)
{
    if (ifs.is_open())
        ifs.close();

    ifs.open(filePath, std::ios::binary);
    return true;
}

AVFrame* VideoRerdererView::read()
{
    if (!ifs || !ifs.is_open() || m_width <= 0 || m_height <= 0)
        return nullptr;
    //AVFrame如果已经申请,如果参数发生变化,需要释放空间
    if (m_frame)
    {
        if (m_frame->width != m_width || m_frame->height != m_height || m_frame->format != m_fmt)
        {
            av_frame_free(&m_frame);
        }
    }

    //为空时,重新生成AVFrame
    if (!m_frame)
    {
        m_frame         = av_frame_alloc();
        m_frame->width  = m_width;
        m_frame->height = m_height;
        m_frame->format = m_fmt;

        m_frame->linesize[0] = m_width*4;//默认设置为RGB
        if (m_frame->format == AV_PIX_FMT_YUV420P)
        {
            m_frame->linesize[0] = m_width;
            m_frame->linesize[1] = m_width/2;
            m_frame->linesize[2] = m_width/2;
        }
        else if (m_frame->format == AV_PIX_FMT_RGB24 || m_frame->format == AV_PIX_FMT_BGR24) {
            m_frame->linesize[0] = m_width * 3; //默认设置为RGB
        }
        auto re = av_frame_get_buffer(m_frame,0);
        if(re != 0)
        {
            char buf[1024] = {0};
            av_strerror(re,buf,sizeof (buf) - 1);
            LOG_WARN << buf;
            av_frame_free(&m_frame);
        }
    }
    if(!m_frame)
        return nullptr;

    //读取一帧数据
    if(m_frame->format == AV_PIX_FMT_YUV420P)
    {
        ifs.read((char*) m_frame->data[0], m_frame->linesize[0] * m_frame->height); //y
        ifs.read((char*) m_frame->data[1], m_frame->linesize[1] * m_frame->height / 2);//u
        ifs.read((char*) m_frame->data[2], m_frame->linesize[2] * m_frame->height / 2);//v
    }
    else if (m_frame->format == AV_PIX_FMT_ARGB || m_frame->format == AV_PIX_FMT_RGBA
             || m_frame->format == AV_PIX_FMT_ABGR || m_frame->format == AV_PIX_FMT_BGRA
             || m_frame->format == AV_PIX_FMT_RGB24 || m_frame->format == AV_PIX_FMT_BGR24)
    {
        ifs.read((char*) m_frame->data[0], m_frame->linesize[0] * m_frame->height);
    }
    if(ifs.gcount() == 0)
        return nullptr;
    return m_frame;
}




