/**************************************************************************
*   文件名      ：Add.cpp
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
#include "Add.h"
#include "ui_Add.h"
#include "VVConfig.h"
#include <QMessageBox>
#include "VVTools.h"
#include "Logger.h"

Add::Add(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Add)
{
    ui->setupUi(this);
    m_index = -1;
}

Add::~Add()
{
    delete ui;
}

void Add::updataData(int index)
{
    auto data = VVConfig::get()->getData(index);
    if (data.name.isEmpty())
    {
        QMessageBox::information(nullptr, C_STR("错误"), C_STR("无法修改"));
        return;
    }

    ui->lineEditName->setText(data.name);
    ui->lineEditUrl->setText(data.url);
    ui->lineEditSubUrl->setText(data.sub_url);
    ui->lineEditSaveFile->setText(data.save_path);
    m_index = index;
}

void Add::on_btnSave_clicked()
{
        LOG_DEBUG << m_index;
    if (save(ui->lineEditName->text(), ui->lineEditUrl->text(), ui->lineEditSubUrl->text(), ui->lineEditSaveFile->text()))
    {
        emit saveSuccess();
        this->close();
        return;
    }

    QMessageBox::information(nullptr, C_STR("保存错误"), C_STR("已有存在名称"));
    return;
}


bool Add::save(const QString& name, const QString& url, const QString& sub_url, const QString& save)
{
    if (name.isEmpty())
    {
        QMessageBox::information(nullptr, C_STR("错误"), C_STR("请输入名称"));
        return false;
    }
    if (url.isEmpty())
    {
        QMessageBox::information(nullptr, C_STR("错误"), C_STR("请输入主码流"));
        return false;
    }
    if (sub_url.isEmpty())
    {
        QMessageBox::information(nullptr, C_STR("错误"), C_STR("请输入子码流"));
        return false;
    }
    if (save.isEmpty())
    {
        QMessageBox::information(nullptr, C_STR("错误"), C_STR("请输入保存路径"));
        return false;
    }

    VVData data{name, url, sub_url, save};
    if (m_index >= 0)
    {
        LOG_DEBUG << m_index;
        return VVConfig::get()->updata(m_index,data);
    }
    else {
        return VVConfig::get()->add(data);
    }
    return false;
}

void Add::on_btnExit_clicked()
{
    this->close();
}
