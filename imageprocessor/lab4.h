#ifndef LAB4_H
#define LAB4_H

#include <QMainWindow>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "picType.h"

namespace Ui {
class lab4;
}

class lab4 : public QMainWindow
{
    Q_OBJECT

public:
    explicit lab4(QWidget *parent = nullptr);
    ~lab4();

    QImage Matrix2QImage(int *data, int width, int height);
    void show_image(QImage Img);

    QImage cvMat2QImage(const cv::Mat &mat);
    void show_image(const cv::Mat &image);

    void getbit();
    int convert_hex_inv(size_t count, const uint8_t *src);

    void my_window(cv::Mat &src, int ww, int wl);
    int convert(int ww, int wl, int tar);

    void my_window(QImage &src, int ww, int wl);
    void my_window(int ww, int wl);
    int convert2(int ww, int wl, int tar);

    void intensify(QImage &Img);
    void intensify(cv::Mat &Img);
    void sharp(QImage &Img);

    void save_img(QImage &Img, QString name);

    QString filepath;

    QImage src_img;//the source image
    QImage dst_img;

    cv::Mat dst;
    cv::Mat src;

    int iHeight;
    int iWidth;
    picType mytype;

    int *raw;

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_actionraw_triggered();

    void on_actionother_triggered();

    void on_pushButton_3_clicked();

private:
    Ui::lab4 *ui;
};

#endif // LAB4_H
