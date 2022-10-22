#include "mywidget.h"
#include "show_img.h"
#include "lab3.h"
#include <QApplication>
#include <QFileDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg");
    Lab3 *p = new Lab3();
    p->show();
    return a.exec();
}
