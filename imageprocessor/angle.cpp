#include "angle.h"
#include "ui_angle.h"
#include "mywidget.h"
#include "picform.h"
#include <QFileDialog>


angle::angle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::angle)
{
    ui->setupUi(this);
}

angle::~angle()
{
    delete ui;
}

void angle::on_OK_btn_clicked()
{
    double angle = ui->angle_text->toPlainText().toDouble();
    double x = ui->x_text->toPlainText().toDouble();
    double y = ui->y_text->toPlainText().toDouble();
    QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp");
    picForm *p = new picForm(filepath, 7, x, y, angle);
    //this->close();
    //p->show();
}

void angle::on_Cancel_btn_clicked()
{
    myWidget *w = new myWidget();
    this->close();
    w->show();
}
