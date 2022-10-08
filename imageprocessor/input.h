#ifndef INPUT_H
#define INPUT_H

#include <QWidget>

namespace Ui {
class input;
}

class input : public QWidget
{
    Q_OBJECT

public:
    explicit input(QWidget *parent = nullptr);
    ~input();

private slots:
    void on_cancel_btn_clicked();

    void on_ok_btn_clicked();

private:
    Ui::input *ui;
    double verti;
    double hori;
};

#endif // INPUT_H
