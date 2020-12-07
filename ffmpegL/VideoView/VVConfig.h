/**************************************************************************
*   文件名      ：VideoViewConfig.h
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-12-3
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
#include <QList>
#include <QMutex>
#include <QString>
#include <QSettings>



struct VVData
{
    QString name;
    QString url;
    QString sub_url;
    QString save_path;

};

class VVConfig
{
public:
    static VVConfig* get()
    {
        static VVConfig config;
        return &config;
    }

    //添加数据,如果存入失败,返回false,存入返回true
    bool add(const VVData& data);

    //获取总数据长度
    int getSize();
    //获取数据
    VVData getData(int index);
    VVData getData(const QString& name);
    bool updata(int index, const VVData& data);

    //删除数据
    bool   deleteData(int index);
    bool   deleteData(const QString &name);

private:
    VVConfig();
    void writeToIni();
    void readFromeIni();

    QMutex         m_mutex;
    QList<VVData>  SETTING;
};

