/**************************************************************************
*   文件名      ：VideoRerdererView.h
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

#include <QMutex>
/**
 * @brief The VideoRerdererView class 视频渲染接口
 */

class VideoRerdererView
{
public:
    enum PixFormat
    {
        RGBA = 0,
        ARGB,
        YUV420P
    };

    enum RendererType
    {
        SDL = 0
    };

    static VideoRerdererView* create(RendererType type = SDL);

    /**
     * @brief init 初始化渲染窗口,保证线程安全,可被多次调用
     * @param width 窗口宽度
     * @param height 窗口高度
     * @param fmt 绘制的像素格式
     * @param winId 窗口句柄,如果为空,创建新窗口
     * @return 是否创建成功
     */
    virtual bool init(int width, int height, PixFormat fmt = RGBA, void* winId = nullptr) = 0;

    /**
     * @brief draw 渲染图像,线程安全
     * @param data 要渲染的二进制数据
     * @param linesize 一行数据的字节数,对于yuv420p就是y一行字节数, <0 就根据宽度和像素格式自动算出大小
     * @return 渲染是否成功
     */
    virtual bool draw(const unsigned char* data, int linesize = 0) = 0;

    /**
     * @brief close 清理所有申请的资源,关闭窗口
     */
    virtual void close() = 0;

    /**
     * @brief isExit 退出窗口
     */
    virtual bool isExit() = 0;

    /**
     * @brief scale 进行显示的缩放
     * @param w
     * @param h
     */
    void scale(int w, int h)
    {
        m_scaleWidth = w;
        m_scalHeight = h;
    }

protected:
    int       m_width  = 0; //材质宽高
    int       m_height = 0;
    PixFormat m_fmt    = RGBA;
    QMutex    m_mutex;
    int       m_scaleWidth = 0;//实际显示宽高
    int       m_scalHeight = 0;
};

