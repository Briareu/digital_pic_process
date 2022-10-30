#include "mywidget.h"
#include "show_img.h"
#include "lab4.h"
#include <QApplication>
#include <QFileDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg");
    lab4 *p = new lab4();
    p->show();
    return a.exec();
}
