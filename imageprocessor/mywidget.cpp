#include "mywidget.h"
#include "ui_mywidget.h"
#include <QFileDialog>
#include "picform.h"
#include "input.h"
#include "angle.h"

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

void myWidget::on_equal_btn2_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg");
    picForm *p = new picForm(filepath, 4);
}

void myWidget::on_rotate_btn_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp");
    picForm *p = new picForm(filepath, 5);
}

void myWidget::on_scale_btn_clicked()
{
    input *p = new input();
    this->close();
    p->show();
}

void myWidget::on_rotate2_btn_clicked()
{
    //QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp");
    //picForm *p = new picForm(filepath, 7);
    angle *p = new angle();
    this->close();
    p->show();
}
