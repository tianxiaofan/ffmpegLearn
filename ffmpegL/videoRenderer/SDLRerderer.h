/**************************************************************************
*   文件名      ：SDLRerderer.h
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
#pragma once

#include "VideoRerdererView.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
class SDLRerderer : public VideoRerdererView
{
public:
    bool init(int width, int height, PixFormat fmt = RGBA, void* winId = nullptr) override;
    bool draw(const unsigned char* data, int linesize = 0) override;
    void close() override;
    bool isExit() override;

private:
    SDL_Window*   m_sdlWindow  = nullptr;
    SDL_Renderer* m_sdlRender  = nullptr;
    SDL_Texture*  m_sdlTexture = nullptr;
};

