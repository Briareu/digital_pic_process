#include "mywidget.h"
#include "ui_mywidget.h"
#include <QFileDialog>
#include "picform.h"

myWidget::myWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::myWidget)
{
    ui->setupUi(this);
}

myWidget::~myWidget()
{
    delete ui;
}

void myWidget::on_m_upload_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg");
    picForm *p = new picForm(filepath, 0);
    this->close();
    p->show();

}


void myWidget::on_m_bmp_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.dib");
    picForm *p = new picForm(filepath, 1);
    this->close();
    p->show();
}

void myWidget::on_FT_btn_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg");
    picForm *p = new picForm(filepath, 2);
    //this->close();
    //p->show();

}

void myWidget::on_equal_btn_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg");
    picForm *p = new picForm(filepath, 3);
}
