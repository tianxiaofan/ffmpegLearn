/**************************************************************************
*   文件名      ：Muxing.h
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-11-27
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

#include "Format.h"

/**
 * @brief The Muxing class 封装
 */
class Muxing : public Format
{
public:
    Muxing();

    /**
     * @brief open 打开封装
     * @param url
     * @return
     */
    static AVFormatContext* open(const char* url);

    /**
     * @brief writeHead 写入头部信息
     * @return
     */
    bool writeHead();

    /**
     * @brief writeAVPacket 写packet,packet会在内部释放
     * @param pkt
     * @return
     */
    bool writeAVPacket(AVPacket* pkt);

    /**
     * @brief writeEnd 写文件尾
     * @return
     */
    bool writeEnd();
};

