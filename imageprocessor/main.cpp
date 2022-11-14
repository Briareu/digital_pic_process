#include "mywidget.h"
#include "show_img.h"
#include "browser.h"
#include <QApplication>
#include <QFileDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QString filepath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg");
    Browser *p = new Browser();
    p->show();
    return a.exec();
}
