/**************************************************************************
*   文件名      ：SDLRerderer.cpp
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
#include "SDLRerderer.h"
#include "Logger.h"

#include <sdl/SDL.h>

static bool initVideo()
{
    static bool   is_first = true;
    static QMutex mux;
    QMutexLocker  lock(&mux);
    if (!is_first)
        return true;

    is_first = false;
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        LOG_WARN_THREAD << SDL_GetError();
        return false;
    }
    //设置双线性插值缩放算法,解决锯齿问题
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    return true;
}
bool SDLRerderer::init(int width, int height, VideoRerdererView::PixFormat fmt)
{
    //初始化sdl
    initVideo();
    if (width <= 0 || height <= 0)
        return false;

    QMutexLocker lock(&m_mutex);
    m_width  = width;
    m_height = height;
    m_fmt    = fmt;

    //创建窗口
    if (!m_sdlWindow)
    {
        if (!m_winId)
        {
            //创建新窗口
            m_sdlWindow = SDL_CreateWindow("",
                                           SDL_WINDOWPOS_UNDEFINED,
                                           SDL_WINDOWPOS_UNDEFINED,
                                           m_width,
                                           m_height,
                                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        }
        else
        {
            //依附于外部窗口
            m_sdlWindow = SDL_CreateWindowFrom(m_winId);
        }
    }
    if (!m_sdlWindow)
    {
        LOG_WARN_THREAD << SDL_GetError();
        return false;
    }

    if (m_sdlTexture)
    {
        SDL_DestroyTexture(m_sdlTexture);
    }
    if (m_sdlRender)
    {
        SDL_DestroyRenderer(m_sdlRender);
    }
    //创建渲染器
    m_sdlRender = SDL_CreateRenderer(m_sdlWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!m_sdlRender)
    {
        LOG_WARN_THREAD << SDL_GetError();
        return false;
    }

    //创建材质(显存)
    unsigned int sdl_fmt = SDL_PIXELFORMAT_ARGB8888;
    switch (m_fmt)
    {
    case VideoRerdererView::ARGB:
        sdl_fmt = SDL_PIXELFORMAT_ARGB32;
        break;
    case VideoRerdererView::RGBA:
        sdl_fmt = SDL_PIXELFORMAT_RGBA32;
        break;
    case VideoRerdererView::ABGR:
        sdl_fmt = SDL_PIXELFORMAT_ABGR32;
        break;
    case VideoRerdererView::BGRA:
        sdl_fmt = SDL_PIXELFORMAT_BGRA32;
        break;
    case VideoRerdererView::RGB24:
        sdl_fmt = SDL_PIXELFORMAT_RGB24;
        break;
    case VideoRerdererView::BGR24:
        sdl_fmt = SDL_PIXELFORMAT_BGR24;
        break;
    case VideoRerdererView::YUV420P:
        sdl_fmt = SDL_PIXELFORMAT_IYUV;
        break;
    case VideoRerdererView::NV12:
        sdl_fmt = SDL_PIXELFORMAT_NV12;
        break;
    default:
        break;
        ;
    }

    m_sdlTexture = SDL_CreateTexture(m_sdlRender,
                                     sdl_fmt,
                                     SDL_TEXTUREACCESS_STREAMING, //支持频繁修改渲染器(带锁)
                                     m_width,
                                     m_height);
    if (!m_sdlTexture)
    {
        LOG_WARN_THREAD << SDL_GetError();
        return false;
    }
    return true;
}

bool SDLRerderer::draw(const unsigned char* data, int linesize)
{
    if (!data)
        return false;

    QMutexLocker lock(&m_mutex);
    if (!m_sdlRender || !m_sdlTexture || !m_sdlWindow || m_width <= 0 || m_height <= 0)
        return false;

    if (linesize <= 0)
    {
        switch (m_fmt)
        {
        case VideoRerdererView::RGBA:
        case VideoRerdererView::ARGB:
        case VideoRerdererView::ABGR:
        case VideoRerdererView::BGRA:
            linesize = m_width * 4;
            break;
        case VideoRerdererView::RGB24:
        case VideoRerdererView::BGR24:
            linesize = m_width * 4;
            break;
        case VideoRerdererView::YUV420P:
            linesize = m_width; // YYYYYYYY UU VV
            break;
        default:
            break;
        }
    }
    if (linesize <= 0)
        return false;

    //复制内存数据到显存
    auto re = SDL_UpdateTexture(m_sdlTexture, nullptr, data, linesize);
    if (re != 0)
    {
        LOG_WARN_THREAD << SDL_GetError();
        return false;
    }
    //清空屏幕
    SDL_RenderClear(m_sdlRender);
    if (m_scalHeight <= 0)
        m_scalHeight = m_height;
    if (m_scaleWidth <= 0)
        m_scaleWidth = m_width;

    SDL_Rect  rect;
    SDL_Rect* prect = nullptr;
    if (m_scaleWidth > 0 && m_scalHeight > 0) //用户手动设置缩放
    {
        rect.x = 0;
        rect.y = 0;
        rect.w = m_scaleWidth;
        rect.h = m_scalHeight;
        prect  = &rect;
    }
    re = SDL_RenderCopy(m_sdlRender, m_sdlTexture, nullptr, prect);
    if (re != 0)
    {
        LOG_WARN_THREAD << SDL_GetError();
        return false;
    }

    //显示
    SDL_RenderPresent(m_sdlRender);
    return true;
}


bool SDLRerderer::draw(const unsigned char *y, int y_pitch, const unsigned char *u, int u_pitch, const unsigned char *v, int v_pitch)
{
    if(!y || !u || !v)
        return false;
    QMutexLocker lock(&m_mutex);
    if (!m_sdlRender || !m_sdlTexture || !m_sdlWindow || m_width <= 0 || m_height <= 0)
        return false;


    //复制内存数据到显存
    auto re = SDL_UpdateYUVTexture(m_sdlTexture, nullptr, y, y_pitch, u, u_pitch, v, v_pitch);
    if (re != 0)
    {
        LOG_WARN_THREAD << SDL_GetError();
        return false;
    }
    //清空屏幕
    SDL_RenderClear(m_sdlRender);
    if (m_scalHeight <= 0)
        m_scalHeight = m_height;
    if (m_scaleWidth <= 0)
        m_scaleWidth = m_width;

    SDL_Rect  rect;
    SDL_Rect* prect = nullptr;
    if (m_scaleWidth > 0 && m_scalHeight > 0) //用户手动设置缩放
    {
        rect.x = 0;
        rect.y = 0;
        rect.w = m_scaleWidth;
        rect.h = m_scalHeight;
        prect  = &rect;
    }
    re = SDL_RenderCopy(m_sdlRender, m_sdlTexture, nullptr, prect);
    if (re != 0)
    {
        LOG_WARN_THREAD << SDL_GetError();
        return false;
    }

    //显示
    SDL_RenderPresent(m_sdlRender);
    return true;
}

void SDLRerderer::close()
{
    QMutexLocker lock(&m_mutex);
    if (m_sdlTexture)
    {
        SDL_DestroyTexture(m_sdlTexture);
        m_sdlTexture = nullptr;
    }
    if (m_sdlRender)
    {
        SDL_DestroyRenderer(m_sdlRender);
        m_sdlRender = nullptr;
    }
    if (m_sdlWindow)
    {
        SDL_DestroyWindow(m_sdlWindow);
        m_sdlWindow = nullptr;
    }
}

bool SDLRerderer::isExit()
{
    SDL_Event sdlEvent;
    SDL_WaitEventTimeout(&sdlEvent,1);
    if(sdlEvent.type == SDL_QUIT)
        return true;
    return false;
}
