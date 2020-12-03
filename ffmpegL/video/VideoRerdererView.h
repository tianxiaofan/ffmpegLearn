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
#include <string>
#include <fstream>
#include "CTools.h"

/**
 * @brief The VideoRerdererView class 视频渲染接口
 */

class VideoRerdererView
{
public:
    enum PixFormat
    {
        // 这里的默认值同ffmpeg里AVPixFormat所对应的值一致,ffmpeg就可直接使用
        YUV420P = 0,
        RGB24   = 2,
        BGR24   = 3,
        NV12    = 23,
        ARGB    = 25,
        RGBA    = 26,
        ABGR    = 27,
        BGRA    = 28
    };

    enum RendererType
    {
        SDL = 0
    };

    virtual ~VideoRerdererView();
    static VideoRerdererView* create(RendererType type = SDL);

    /**
     * @brief init 初始化渲染窗口,保证线程安全,可被多次调用
     * @param width 窗口宽度
     * @param height 窗口高度
     * @param fmt 绘制的像素格式
     * @return 是否创建成功
     */
    virtual bool init(int width, int height, PixFormat fmt = RGBA) = 0;
    virtual bool init(AVCodecParameters* para);

    /**
     * @brief draw 渲染图像,线程安全
     * @param data 要渲染的二进制数据
     * @param linesize 一行数据的字节数,对于yuv420p就是y一行字节数, <0 就根据宽度和像素格式自动算出大小
     * @return 渲染是否成功
     */
    virtual bool draw(const unsigned char* data, int linesize = 0) = 0;

    virtual bool draw(const unsigned char* y,
                      int                  y_pitch,
                      const unsigned char* u,
                      int                  u_pitch,
                      const unsigned char* v,
                      int                  v_pitch) = 0;

    /**
     * @brief drawAVFrame 渲染AVFrame
     * @param frame AVFrame
     * @return
     */
    virtual bool drawAVFrame(AVFrame* frame);

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

    int getFps() { return m_sfps; }

    /**
     * @brief openFile 打开本地文件
     * @param filePath
     * @return
     */
    bool openFile(std::string filePath);

    /**
     * @brief setWinId 设置窗口句柄
     * @param winid
     */
    void setWinId(void* winid) { m_winId = winid; }

    /**
     * @brief read 从文件中读取一帧数据,并维护AVFrame空间,每次调用会覆盖上一次的空间
     * @return
     */
    AVFrame* read();

protected:
    int       m_sfps   = 0; //显示帧率
    int       m_width  = 0; //材质宽高
    int       m_height = 0;
    PixFormat m_fmt    = RGBA;
    QMutex    m_mutex;
    int       m_scaleWidth = 0; //实际显示宽高
    int       m_scalHeight = 0;
    long long m_beginMs    = 0;//计时开始时间
    int       m_count      = 0; //统计显示次数
    void*     m_winId;

private:
    std::ifstream  ifs;
    AVFrame*       m_frame = nullptr;
    unsigned char* m_cache = nullptr; //nv12的临时缓冲空间
};

