#include "input.h"
#include "ui_input.h"
#include "mywidget.h"
#include "picform.h"
#include <QFileDialog>

input::input(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::input)
{
    ui->setupUi(this);
}

input::~input()
{
    delete ui;
}

void input::on_cancel_btn_clicked()
{
    myWidget *w = new myWidget();
    this->close();
    w->show();
}

void input::on_ok_btn_clicked()
{
    verti = ui->vert_text->toPlainText().toDouble();
    hori = ui->hori_text->toPlainText().toDouble();
    QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp");
    picForm *p = new picForm(filepath, 6, hori, verti);
}
