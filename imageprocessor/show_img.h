#ifndef SHOW_IMG_H
#define SHOW_IMG_H

#include <QMainWindow>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

namespace Ui {
class show_img;
}

class show_img : public QMainWindow
{
    Q_OBJECT

public:
    explicit show_img(QWidget *parent = nullptr);
    ~show_img();

    QImage cvMat2QImage(const cv::Mat &mat);
    void show_image(const cv::Mat &image);
    void show_image2(const cv::Mat &image);

    void rotate(cv::Mat &src, int Angle);
    void rotate2(cv::Mat &src, int Angle, double cx, double cy);
    void scale(cv::Mat &src, double cx, double cy);

    void getbit();

    QString filepath;
    cv::Mat dst;

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::show_img *ui;
};

#endif // SHOW_IMG_H
