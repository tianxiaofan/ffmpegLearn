/**************************************************************************
*   文件名      ：Codec.h
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-26
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
#include "CTools.h"



/**
 * @brief The Codec class 编码和解码的基类
 */
class VIDEODLL_EXPORT Codec
{
public:
    Codec();

    /**
     * @brief cteate 创建编解码器上下文
     * @param codeId 编码器id,与ffmpeg相对应
     * @return  返回编码器上下文,失败返回nullptr
     */
    static AVCodecContext* create(int codeId,bool isEncodec);

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


protected:
    AVCodecContext* m_ctx = nullptr; //编码器上下文
    std::mutex      m_mutex;         //编码器上下文锁
};

