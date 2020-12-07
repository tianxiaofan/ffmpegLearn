/**************************************************************************
*   文件名      ：VideoViewConfig.cpp
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
#include "VVConfig.h"

#include <QTextCodec>
#include <QDebug>
#include "VVTools.h"

#define INI_NAME "config.ini"

VVConfig::VVConfig()
{
    readFromeIni();
}

void VVConfig::writeToIni()
{
    QMutexLocker lock(&m_mutex);
    QSettings    set(INI_NAME, QSettings::IniFormat);
    set.setIniCodec(QTextCodec::codecForName("UTF-8")); //在此添加设置，即可读写ini文件中的中文
    for (int i = 0; i < SETTING.size(); ++i)
    {
        set.beginGroup(SETTING.at(i).name);
        set.setValue(ToStr(name), SETTING.at(i).name);
        set.setValue(ToStr(url), SETTING.at(i).url);
        set.setValue(ToStr(sub_url), SETTING.at(i).sub_url);
        set.setValue(ToStr(save_path), SETTING.at(i).save_path);
        set.endGroup();
    }
}

void VVConfig::readFromeIni()
{
    QMutexLocker lock(&m_mutex);
    QSettings    set(INI_NAME, QSettings::IniFormat);
    set.setIniCodec(QTextCodec::codecForName("UTF-8")); //在此添加设置，即可读写ini文件中的中文
    auto groups = set.childGroups();
    for (auto s : groups)
    {
        set.beginGroup(s);
        VVData data;
        data.name      = set.value(ToStr(name)).toString();
        data.url       = set.value(ToStr(url)).toString();
        data.sub_url   = set.value(ToStr(sub_url)).toString();
        data.save_path = set.value(ToStr(save_path)).toString();
        set.endGroup();
        SETTING.append(data);
    }
}

bool VVConfig::add(const VVData& data)
{
    QMutexLocker lock(&m_mutex);

    //查找是否有重复数据
    if (!SETTING.isEmpty())
    {
        for (auto s : SETTING)
        {
            if (s.name == data.name) //有重复数据则直接退出,不写入链表和配置文件
                return false;
        }
    }

    SETTING.append(data);
    lock.unlock();
    writeToIni();
    return true;
}

int VVConfig::getSize()
{
    QMutexLocker lock(&m_mutex);
    return SETTING.size();
}

VVData VVConfig::getData(int index)
{
    QMutexLocker lock(&m_mutex);
    if (index > SETTING.size() || index < 0)
        return VVData{};
    return SETTING.at(index);
}

VVData VVConfig::getData(const QString& name)
{
    QMutexLocker lock(&m_mutex);
    for (int i = 0; i < SETTING.size(); ++i)
    {
        if (SETTING.at(i).name == name)
        {
            lock.unlock();
            return getData(i);
        }
    }
    return VVData{};
}

bool VVConfig::updata(int index, const VVData& data)
{
    QMutexLocker lock(&m_mutex);
    if (index < 0 || index > SETTING.size())
        return false;
    SETTING[index] = data;

    lock.unlock();
    writeToIni();
    return true;
}

bool VVConfig::deleteData(int index)
{
    QMutexLocker lock(&m_mutex);
    if (SETTING.isEmpty() || index > SETTING.size() || index < 0)
        return false;
    auto name = SETTING.at(index).name;
    lock.unlock();
    return deleteData(name);
}

bool VVConfig::deleteData(const QString& name)
{
    if (name.isEmpty())
        return false;
    QMutexLocker lock(&m_mutex);
    for (int i = 0; i < SETTING.size(); i++)
    {
        if (SETTING.at(i).name == name)
        {
            QSettings    set(INI_NAME, QSettings::IniFormat);
            set.setIniCodec(QTextCodec::codecForName("UTF-8")); //在此添加设置，即可读写ini文件中的中文
            set.remove(name);
            SETTING.removeAt(i);
            return true;
        }
    }
    return false;
}
