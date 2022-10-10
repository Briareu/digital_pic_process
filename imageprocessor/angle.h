#ifndef ANGLE_H
#define ANGLE_H

#include <QWidget>

namespace Ui {
class angle;
}

class angle : public QWidget
{
    Q_OBJECT

public:
    explicit angle(QWidget *parent = nullptr);
    ~angle();

private slots:
    void on_OK_btn_clicked();

    void on_Cancel_btn_clicked();

private:
    Ui::angle *ui;
};

#endif // ANGLE_H
