/**************************************************************************
*   文件名      ：CustomCalender.h
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-12-7
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

#include <QCalendarWidget>
#include <QDate>
#include <QSet>

class CustomCalender : public QCalendarWidget
{
public:
    CustomCalender(QWidget* parent = nullptr);
    void paintCell(QPainter* painter, const QRect& rect, const QDate& date) const override;

    void addDate(const QDate& date) { m_dateSet.insert(date); }
    void clearDate() { m_dateSet.clear(); }
    void updateC() { updateCells(); }

private:
    QSet<QDate> m_dateSet;
};

