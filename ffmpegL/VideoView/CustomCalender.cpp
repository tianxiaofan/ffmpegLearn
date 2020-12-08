/**************************************************************************
*   文件名      ：CustomCalender.cpp
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
#include "CustomCalender.h"

#include <QPainter>
#include "Logger.h"

CustomCalender::CustomCalender(QWidget* parent) : QCalendarWidget(parent) {}

void CustomCalender::paintCell(QPainter* painter, const QRect& rect, const QDate& date) const
{
    //没有找到需特殊显示的时间
    if(!m_dateSet.contains(date))
    {
        QCalendarWidget::paintCell(painter,rect,date);
        return;
    }
    auto font = painter->font();
    font.setPixelSize(40);                 //设置字体大小

    //选中刷背景色
    if(date == selectedDate())
    {
        painter->setBrush(QColor(50, 178, 224)); //设置背景色
        painter->drawRect(rect);                 //绘制背景
    }
    painter->setFont(font);
    painter->setPen(QColor(255, 0, 0)); //设置字体颜色
    painter->drawText(rect,Qt::AlignCenter,QString::number(date.day()));
}
