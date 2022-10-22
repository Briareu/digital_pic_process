#include "lab3.h"
#include "ui_lab3.h"
#include "show_img.h"

Lab3::Lab3(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Lab3)
{
    ui->setupUi(this);
}

Lab3::~Lab3()
{
    delete ui;
}

void Lab3::on_raw_btn_clicked()
{
    show_img *w = new show_img(_RAW);
    w->show();
    this->close();
}

void Lab3::on_other_btn_clicked()
{
    show_img *w = new show_img(_LAB);
    w->show();
    this->close();
}
