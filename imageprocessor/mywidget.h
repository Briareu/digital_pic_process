#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "picType.h"
#include "picform.h"

QT_BEGIN_NAMESPACE
namespace Ui { class myWidget; }
QT_END_NAMESPACE

class myWidget : public QWidget
{
    Q_OBJECT

public:
    myWidget(QWidget *parent = nullptr);
    ~myWidget();

public slots:
    void on_m_upload_clicked();

private slots:
    void on_m_bmp_clicked();

    void on_FT_btn_clicked();

    void on_equal_btn_clicked();

    void on_equal_btn2_clicked();

    void on_rotate_btn_clicked();

    void on_scale_btn_clicked();

private:
    Ui::myWidget *ui;
};
#endif // MYWIDGET_H
