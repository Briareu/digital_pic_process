#ifndef PICFORM_H
#define PICFORM_H

#include <QWidget>
#include <QPushButton>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


#include "picType.h"

namespace Ui {
class picForm;
}

class picForm : public QWidget
{
    Q_OBJECT

public:
    explicit picForm(QString path = nullptr, int typr = 0, double x = 0.0, double  y = 0.0, double angle = 0.0, QWidget *parent = nullptr);

    void m_show();
    void getBitsForBMP();

    cv::Mat FT(cv::Mat &srcImage);
    cv::Mat getMagnitudeImage(const cv::Mat &fourierImage);
    cv::Mat changeCenter(const cv::Mat &magImage);
    cv::Mat getPhaseImage(const cv::Mat &fourierImage);
    void frequencyFiltering();
    cv::Mat inverseFT(const cv::Mat &fourier);

    void _equalize();//black-and-white picture
    void _equalize2();//colorful

    void rotate();
    void scale();
    void rotate2();

    QImage cvMat2QImage(const cv::Mat &mat);

    ~picForm();
    QString filepath;
    picType m_type;
    double cx, cy, _angle;


public slots:
    void on_pushButton_clicked();

private:
    Ui::picForm *ui;
};

#endif // PICFORM_H
