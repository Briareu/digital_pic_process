#ifndef SHOW_IMG_H
#define SHOW_IMG_H

#include <QMainWindow>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "picType.h"

namespace Ui {
class show_img;
}

class show_img : public QMainWindow
{
    Q_OBJECT

public:
    explicit show_img(picType my, QWidget *parent = nullptr);
    ~show_img();

    QImage cvMat2QImage(const cv::Mat &mat);
    cv::Mat QImage2Mat(QImage const& src);
    QImage Mattix2QImage(int *data, int width, int height);
    void show_image(const cv::Mat &image);
    void show_image2(const cv::Mat &image);
    void show_image3(QImage Img);
    void show_image4(QImage Img);

    void rotate(cv::Mat &src, int Angle);
    void rotate2(cv::Mat &src, int Angle, double cx, double cy);
    void scale(cv::Mat &src, double cx, double cy);

    void rotate_img(QImage &src, int Angle);
    void rotate2_img(QImage &src, int Angle, double cx, double cy);
    QImage rotateBmpFromImage(QImage pImage1, int nRotate);
    void scale_img(QImage &src, double cx, double cy);

    void getbit();
    int convert_hex_inv(size_t count, const uint8_t *src);
    void my_window(cv::Mat &src, int ww, int wl);
    void my_window(QImage &src, int ww, int wl);
    int convert(int ww, int wl, int tar);
    int convert2(int ww, int wl, int tar);


    QString filepath;
    cv::Mat dst;
    cv::Mat src;
    QImage src_img;
    QImage dst_img;
    picType mytype;
    unsigned short *pushRawdata2;
    int *raw;

    int iHeight;
    int iWidth;

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_actionselect_triggered();

private:
    Ui::show_img *ui;
};

#endif // SHOW_IMG_H
