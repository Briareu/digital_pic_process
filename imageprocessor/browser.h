#ifndef BROWSER_H
#define BROWSER_H

#include <QMainWindow>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "picType.h"
#include <vector>
#include "functiontype.h"


namespace Ui {
class Browser;
}

class Browser : public QMainWindow
{
    Q_OBJECT

public:
    explicit Browser(QWidget *parent = nullptr);
    ~Browser();


    QImage Matrix2QImage(int *data, int width, int height);
    void show_image(QImage Img);

    QImage cvMat2QImage(const cv::Mat &mat);
    void show_image(const cv::Mat &image);

    void getbit();

    void my_window(cv::Mat &src, int ww, int wl);
    int convert(int ww, int wl, int tar);

    void my_window(QImage &src, int ww, int wl);
    void my_window(int *raw_p, int ww, int wl);
    int convert2(int ww, int wl, int tar);

    void intensify(QImage &Img);
    void intensify();
    void intensify(cv::Mat &Img);

    void save_img(QImage &Img, QString name);
    void save_img2(QImage &Img, QString name);

    void scale(QImage image, double cx, double cy);
    void inverse(QImage image);
    void inverse();
    void reverse(QImage image);
    void reverse();
    void rotate(double Angle);

    void clear();

    QString filepath;

    QImage src_img;//the source image
    QImage dst_img;

    cv::Mat dst;
    cv::Mat src;

    int iHeight;
    int iWidth;
    picType mytype;

    int iHeight_temp;
    int iWidth_temp;

    int iHeight_window;
    int iWidth_window;

    int *raw;
    std::vector<std::pair<funcType, std::vector<float>>> back_find;
    std::vector<std::pair<funcType, std::vector<float>>> back_find_mat;

    std::vector<QImage> back_image;
    bool isArray = true;
    bool isWindowed = false;

private slots:
    void on_actionraw_triggered();

    void on_actionother_triggered();

    void on_btn_ok_clicked();

    void on_btn_recover_clicked();

    void on_btn_back_clicked();

    void on_btn_save_clicked();

private:
    Ui::Browser *ui;
};

#endif // BROWSER_H
