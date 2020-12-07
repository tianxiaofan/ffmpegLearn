/**************************************************************************
*   文件名      ：Demux.h
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
#include <vector>

struct VIDEODLL_EXPORT DictionaryOpt
{
    std::string key;
    std::string value;
};

class VIDEODLL_EXPORT Demux : public Format
{
public:
    Demux();

    /**
     * @brief open 打开解封装
     * @param url
     * @return
     */
    static AVFormatContext* open(const char* url, std::vector<DictionaryOpt> o = std::vector<DictionaryOpt>());

    /**
     * @brief read 读取一个AVPacket
     * @param pkt
     * @return
     */
    bool read(AVPacket* pkt);

    /**
     * @brief seek 移到pts位置
     * @param pts
     * @param stream_index
     * @return
     */
    bool seek(long long pts, int stream_index);
};

