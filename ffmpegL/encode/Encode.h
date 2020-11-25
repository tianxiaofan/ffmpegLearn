/**************************************************************************
*   文件名      ：Encode.h
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-25
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

#include <mutex>
#include <vector>
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
class Encode
{
public:
    Encode();
    /**
     * @brief cteate 创建编码器上下文
     * @param codeId 编码器id,与ffmpeg相对应
     * @return  返回编码器上下文,失败返回nullptr
     */
    static AVCodecContext* cteate(int codeId);

    /**
     * @brief setContext 设置对象的编码器上下文,线程安全
     * @param c 上下文传递到对象中,空间由Encode来维护,如果对象内不是null,需要先清理
     */
    void setContext(AVCodecContext* c);

    /**
     * @brief setOpt 设置编码参数,线程安全
     * @param key
     * @param val
     * @return
     */
    bool setOpt(const char* key, const char* val);
    bool setOpt(const char* key, int val);

    /**
     * @brief openEncodec 打开编码器,线程安全
     * @return
     */
    bool open();

    /**
     * @brief close  关闭并释放编码器
     */
    void close();

    /**
     * @brief encodeAVFrame 编码数据,线程安全,每次会新创建AVPacket
     * @param frame 由设置维护
     * @return 失败返回nullptr,return AVPacket user mast use av_packet_free close;
     */
    AVPacket* encodeAVFrame(const AVFrame* frame);

    /**
     * @brief end 取编码器中的缓冲的值
     * @return
     */
    std::vector<AVPacket*> end();

    /**
     * @brief createAVFrame 创建一个AVFrame,由用户释放
     * @return
     */
    AVFrame* createAVFrame();

    /**
     * @brief free 释放资源
     * @param pkt
     */
    static void free(AVPacket* pkt);
    static void free(AVFrame* frame);


private:
    AVCodecContext* m_ctx = nullptr; //编码器上下文
    std::mutex      m_mutex;         //编码器上下文锁
};

