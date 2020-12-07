/**************************************************************************
*   文件名      ：Add.h
*   =======================================================================
*   创 建 者    ：田小帆
*   创建日期    ：2020-12-4
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

#include <QDialog>

namespace Ui {
class Add;
}

class Add : public QDialog
{
    Q_OBJECT

public:
    explicit Add(QWidget *parent = nullptr);
    ~Add();

    void updataData(int index);

signals:
    void saveSuccess();

private slots:
    void on_btnSave_clicked();


    bool save(const QString& name, const QString& url, const QString& sub_url, const QString& save);

    void on_btnExit_clicked();

private:
    Ui::Add *ui;
    int      m_index = -1;
};

