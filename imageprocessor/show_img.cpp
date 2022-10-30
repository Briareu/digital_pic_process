#include "show_img.h"
#include "ui_show_img.h"
#include "picType.h"
#include "lab3.h"

#include <QKeyEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <malloc.h>
#include <windows.h>
#include <iostream>
#include <QInputDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <fstream>
#include <sstream>
#include <QChar>
#include <qcolor.h>

show_img::show_img(picType my, QWidget *parent) :
    QMainWindow(parent), mytype(my),
    ui(new Ui::show_img)
{
    QString str = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg;*.raw");
    this->filepath = str;

    if(mytype == _LAB){
        src = cv::imread(filepath.toStdString());

        ui->setupUi(this);
        show_image(src);
    }
    else{
        ui->setupUi(this);
        this->getbit();
    }
}

void show_img::show_image(const cv::Mat &src){
    QImage Img = cvMat2QImage(src);

    QGraphicsScene *scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(Img));
    ui->pic2->setScene(scene);
    ui->pic2->show();

}

void show_img::show_image2(const cv::Mat &src){
    QImage Img = cvMat2QImage(src);

    QGraphicsScene *scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(Img));
    ui->pic1->setScene(scene);
    ui->pic1->show();

}

void show_img::show_image3(QImage Img){
    QGraphicsScene *scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(Img));
    ui->pic2->setScene(scene);
    ui->pic2->show();
}

void show_img::show_image4(QImage Img){
    QGraphicsScene *scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(Img));
    ui->pic1->setScene(scene);
    ui->pic1->show();
}

QImage show_img::cvMat2QImage(const cv::Mat &mat)
{

    // 8-bits unsigned, NO. OF CHANNELS = 1
    if (mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_RGBX64);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for (int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for (int row = 0; row < mat.rows; row++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;					//         Index1
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if (mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();		//         Index2
    }
    else if (mat.type() == CV_8UC4)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();			//         Index3
    }
    else
    {
        return QImage();
    }
    /*
    cvtColor(mat, mat, cv::COLOR_BGR2RGB);
    QImage qim(mat.data, mat.cols, mat.rows, mat.step,
               QImage::Format_RGB888);
    return qim;*/
}

cv::Mat show_img::QImage2Mat(QImage const& src)
{
     cv::Mat tmp(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());
     cv::Mat result; // deep copy just in case (my lack of knowledge with open cv)
     cvtColor(tmp, result,CV_16UC1);
     return result;
}

void show_img::rotate(cv::Mat &src, int Angle){
    bool bRet = false;

    double angle = Angle*CV_PI / 180.0;
    //构造输出图像
    int dst_rows = round(fabs(src.rows * cos(angle)) + fabs(src.cols * sin(angle)));//图像高度
    int dst_cols = round(fabs(src.cols * cos(angle)) + fabs(src.rows * sin(angle)));//图像宽度

    if (src.channels() == 1) {
        dst = cv::Mat::zeros(dst_rows, dst_cols, CV_8UC1); //灰度图初始
    }
    else {
        dst = cv::Mat::zeros(dst_rows, dst_cols, CV_8UC3); //RGB图初始
    }

    cv::Mat tran1 = (cv::Mat_<double>(3,3) << 1.0,0.0,0.0 , 0.0,-1.0,0.0, -0.5*src.cols , 0.5*src.rows , 1.0); // 将原图像坐标映射到数学笛卡尔坐标
    cv::Mat tran2 = (cv::Mat_<double>(3,3) << cos(angle),-sin(angle),0.0 , sin(angle), cos(angle),0.0, 0.0,0.0,1.0); //数学笛卡尔坐标下顺时针旋转的变换矩阵
    double t3[3][3] = { { 1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }, { 0.5*dst.cols, 0.5*dst.rows ,1.0} }; // 将数学笛卡尔坐标映射到旋转后的图像坐标
    cv::Mat tran3 = cv::Mat(3.0,3.0,CV_64FC1,t3);
    cv::Mat T = tran1*tran2*tran3;
    cv::Mat T_inv = T.inv(); // 求逆矩阵

    for (double i = 0.0; i < dst.rows; i++){
        for (double j = 0.0; j < dst.cols; j++){
            cv::Mat dst_coordinate = (cv::Mat_<double>(1, 3) << j, i, 1.0);
            cv::Mat src_coordinate = dst_coordinate * T_inv;
            double v = src_coordinate.at<double>(0, 0);
            double w = src_coordinate.at<double>(0, 1);

            //双线性插值
            if (int(Angle) % 90 == 0) {
                if (v < 0)
                    v = 0;
                if (v > src.cols - 1)
                    v = src.cols - 1;
                if (w < 0)
                    w = 0;
                if (w > src.rows - 1)
                    w = src.rows - 1;
            }

            if (v >= 0 && w >= 0 && v <= src.cols - 1 && w <= src.rows - 1){
                int top = floor(w), bottom = ceil(w), left = floor(v), right = ceil(v);
                double pw = w - top ;
                double pv = v - left;
                if (src.channels() == 1){
                    dst.at<uchar>(i, j) = (1 - pw)*(1 - pv)*src.at<uchar>(top, left) +
                            (1 - pw)*pv*src.at<uchar>(top, right) +
                            pw*(1 - pv)*src.at<uchar>(bottom, left) +
                            pw*pv*src.at<uchar>(bottom, right);
                }
                else{
                    dst.at<cv::Vec3b>(i, j)[0] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[0] +
                            (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[0] +
                            pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[0] +
                            pw*pv*src.at<cv::Vec3b>(bottom, right)[0];
                    dst.at<cv::Vec3b>(i, j)[1] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[1] +
                            (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[1] +
                            pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[1] +
                            pw*pv*src.at<cv::Vec3b>(bottom, right)[1];
                    dst.at<cv::Vec3b>(i, j)[2] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[2] +
                            (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[2] +
                            pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[2] +
                            pw*pv*src.at<cv::Vec3b>(bottom, right)[2];
                }
            }
        }
    }

    //show_image2(dst);
    //ui->label->setText("123");
}

void show_img::rotate2(cv::Mat &src, int Angle, double cx, double cy){
    bool bRet = false;

    double angle = Angle*CV_PI / 180.0;
    //构造输出图像
    //int dst_rows = round(fabs(src.rows * cos(angle)) + fabs(src.cols * sin(angle)));//图像高度
    //int dst_cols = round(fabs(src.cols * cos(angle)) + fabs(src.rows * sin(angle)));//图像宽度

    int dst_rows = src.rows;
    int dst_cols = src.cols;

    if (src.channels() == 1) {
        dst = cv::Mat::zeros(dst_rows, dst_cols, CV_8UC1); //灰度图初始
    }
    else {
        dst = cv::Mat::zeros(dst_rows, dst_cols, CV_8UC3); //RGB图初始
    }

    cv::Mat tran1 = (cv::Mat_<double>(3,3) << 1.0,0.0,0.0 , 0.0,-1.0,0.0, -cx , cy , 1.0); // 将原图像坐标映射到数学笛卡尔坐标
    cv::Mat tran2 = (cv::Mat_<double>(3,3) << cos(angle),-sin(angle),0.0 , sin(angle), cos(angle),0.0, 0.0,0.0,1.0); //数学笛卡尔坐标下顺时针旋转的变换矩阵
    double t3[3][3] = { { 1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }, { cx, cy ,1.0} }; // 将数学笛卡尔坐标映射到旋转后的图像坐标
    cv::Mat tran3 = cv::Mat(3.0,3.0,CV_64FC1,t3);
    cv::Mat T = tran1*tran2*tran3;
    cv::Mat T_inv = T.inv(); // 求逆矩阵

    for (double i = 0.0; i < dst.rows; i++){
        for (double j = 0.0; j < dst.cols; j++){
            cv::Mat dst_coordinate = (cv::Mat_<double>(1, 3) << j, i, 1.0);
            cv::Mat src_coordinate = dst_coordinate * T_inv;
            double v = src_coordinate.at<double>(0, 0);
            double w = src_coordinate.at<double>(0, 1);

            //双线性插值
            if (int(Angle) % 90 == 0) {
                if (v < 0)
                    v = 0;
                if (v > src.cols - 1)
                    v = src.cols - 1;
                if (w < 0)
                    w = 0;
                if (w > src.rows - 1)
                    w = src.rows - 1;
            }

            if (v >= 0 && w >= 0 && v <= src.cols - 1 && w <= src.rows - 1){
                int top = floor(w), bottom = ceil(w), left = floor(v), right = ceil(v);
                double pw = w - top ;
                double pv = v - left;
                if (src.channels() == 1){
                    dst.at<uchar>(i, j) = (1 - pw)*(1 - pv)*src.at<uchar>(top, left) +
                            (1 - pw)*pv*src.at<uchar>(top, right) +
                            pw*(1 - pv)*src.at<uchar>(bottom, left) +
                            pw*pv*src.at<uchar>(bottom, right);
                }
                else{
                    dst.at<cv::Vec3b>(i, j)[0] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[0] +
                            (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[0] +
                            pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[0] +
                            pw*pv*src.at<cv::Vec3b>(bottom, right)[0];
                    dst.at<cv::Vec3b>(i, j)[1] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[1] +
                            (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[1] +
                            pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[1] +
                            pw*pv*src.at<cv::Vec3b>(bottom, right)[1];
                    dst.at<cv::Vec3b>(i, j)[2] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[2] +
                            (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[2] +
                            pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[2] +
                            pw*pv*src.at<cv::Vec3b>(bottom, right)[2];
                }
            }
        }
    }

    //show_image2(dst);
    //ui->label->setText("123");
}

void show_img::scale(cv::Mat &src, double cx, double cy){
    int dst_rows = round(cy*src.rows);//图像高度
    int dst_cols = round(cx*src.cols);//图像宽度

    if (src.channels() == 1) {
        dst = cv::Mat::zeros(dst_rows, dst_cols, CV_8UC1); //灰度图初始
    }
    else {
        dst = cv::Mat::zeros(dst_rows, dst_cols, CV_8UC3); //RGB图初始
    }

    cv::Mat T = (cv::Mat_<double>(3, 3) <<cx,0,0, 0,cy,0 ,0,0,1 ); //尺度变换矩阵
    cv::Mat T_inv = T.inv(); // 求逆矩阵

    for (int i = 0; i < dst.rows; i++){
        for (int j = 0; j < dst.cols; j++){
            cv::Mat dst_coordinate = (cv::Mat_<double>(1, 3) << j, i, 1);
            cv::Mat src_coordinate = dst_coordinate * T_inv;
            double v = src_coordinate.at<double>(0, 0); // 原图像的横坐标，列，宽
            double w = src_coordinate.at<double>(0, 1); // 原图像的纵坐标，行，高

            /*双线性插值*/
            if (v < 0) v = 0; if (v > src.cols - 1) v = src.cols - 1;
            if (w < 0) w = 0; if (w > src.rows - 1) w = src.rows - 1;

            if (v >= 0 && w >= 0 && v <= src.cols - 1 && w <= src.rows - 1){
                int top = floor(w), bottom = ceil(w), left = floor(v), right = ceil(v); //与映射到原图坐标相邻的四个像素点的坐标
                double pw = w - top; //pw为坐标 行 的小数部分(坐标偏差)
                double pv = v - left; //pv为坐标 列 的小数部分(坐标偏差)
                if (src.channels() == 1){
                    //灰度图像
                    dst.at<uchar>(i, j) = (1 - pw)*(1 - pv)*src.at<uchar>(top, left) +
                            (1 - pw)*pv*src.at<uchar>(top, right) +
                            pw*(1 - pv)*src.at<uchar>(bottom, left) +
                            pw*pv*src.at<uchar>(bottom, right);
                }
                else{
                    dst.at<cv::Vec3b>(i, j)[0] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[0] +
                            (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[0] +
                            pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[0] +
                            pw*pv*src.at<cv::Vec3b>(bottom, right)[0];
                    dst.at<cv::Vec3b>(i, j)[1] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[1] +
                            (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[1] +
                            pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[1] +
                            pw*pv*src.at<cv::Vec3b>(bottom, right)[1];
                    dst.at<cv::Vec3b>(i, j)[2] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[2] +
                            (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[2] +
                            pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[2] +
                            pw*pv*src.at<cv::Vec3b>(bottom, right)[2];
                }
            }
        }
    }
    //cv::Mat Rgb;
    //QImage Img = cvMat2QImage(dst);
    //ui->picLabel->setPixmap(QPixmap::fromImage(Img));

    //ui->picLabel->setFixedSize(QSize(dst.cols , dst.rows));
    //ui->picLabel->show();

    //ui->scrollArea->setWidget(ui->picLabel);
    //ui->picLabel->resize(ui->picLabel->pixmap()->size());
    //ui->picLabel->adjustSize();
    //ui->scrollArea->setWidget(ui->picLabel);
}

void show_img::scale_img(QImage &src, double cx, double cy){
    int dst_rows = round(cy*src.height());//图像高度
    int dst_cols = round(cx*src.width());//图像宽度

    if (src.depth() == 1) {
        dst_img = QImage(dst_cols, dst_rows, QImage::Format_RGBX64); //灰度图初始
        dst_img.fill(0);
    }
    else {
        dst_img = QImage(dst_cols, dst_rows, QImage::Format_ARGB32); //RGB图初始
        dst_img.fill(0);
    }

    cv::Mat T = (cv::Mat_<double>(3, 3) <<cx,0,0, 0,cy,0 ,0,0,1 ); //尺度变换矩阵
    cv::Mat T_inv = T.inv(); // 求逆矩阵

    for (int i = 0; i < dst_rows; i++){
        for (int j = 0; j < dst_cols; j++){
            cv::Mat dst_coordinate = (cv::Mat_<double>(1, 3) << j, i, 1);
            cv::Mat src_coordinate = dst_coordinate * T_inv;
            double v = src_coordinate.at<double>(0, 0); // 原图像的横坐标，列，宽
            double w = src_coordinate.at<double>(0, 1); // 原图像的纵坐标，行，高

            /*双线性插值*/
            if (v < 0) v = 0; if (v > src.width() - 1) v = src.width() - 1;
            if (w < 0) w = 0; if (w > src.height() - 1) w = src.height() - 1;

            if (v >= 0 && w >= 0 && v <= src.width() - 1 && w <= src.width() - 1){
                int top = floor(w), bottom = ceil(w), left = floor(v), right = ceil(v); //与映射到原图坐标相邻的四个像素点的坐标
                double pw = w - top; //pw为坐标 行 的小数部分(坐标偏差)
                double pv = v - left; //pv为坐标 列 的小数部分(坐标偏差)
                if (src.depth() == 1){
                    //灰度图像
                    int temp = (1 - pw)*(1 - pv)*raw[top * src_img.width()+left] + (1 - pw)*pv*raw[top * src_img.width()+right] + pw*(1 - pv)*raw[bottom * src_img.width()+left] + pw*pv*raw[bottom * src_img.width()+right];
                    dst_img.setPixel(j, i, qRgb(temp, temp, temp));
                }
                else{
                    int temp = (1 - pw)*(1 - pv)*src.pixel(top, left) + (1 - pw)*pv*src.pixel(top, right) + pw*(1 - pv)*src.pixel(bottom, left) + pw*pv*src.pixel(bottom, right);
                    dst_img.setPixel(j, i, qRgb(temp, temp, temp));
                }
            }
        }
    }
}

void show_img::rotate_img(QImage &src_img, int Angle){
    bool bRet = false;

    double angle = Angle*CV_PI / 180.0;
    //构造输出图像
    int dst_rows = round(fabs(src_img.height() * cos(angle)) + fabs(src_img.width() * sin(angle)));//图像高度
    int dst_cols = round(fabs(src_img.width() * cos(angle)) + fabs(src_img.height() * sin(angle)));//图像宽度

    if (src.depth() == 1) {
        dst_img = QImage(dst_cols, dst_rows, QImage::Format_RGBX64); //灰度图初始
        dst_img.fill(0);
    }
    else {
        dst_img = QImage(dst_cols, dst_rows, QImage::Format_ARGB32); //RGB图初始
        dst_img.fill(0);
    }

    cv::Mat tran1 = (cv::Mat_<double>(3,3) << 1.0,0.0,0.0 , 0.0,-1.0,0.0, -0.5*src_img.width() , 0.5*src_img.height() , 1.0); // 将原图像坐标映射到数学笛卡尔坐标
    cv::Mat tran2 = (cv::Mat_<double>(3,3) << cos(angle),-sin(angle),0.0 , sin(angle), cos(angle),0.0, 0.0,0.0,1.0); //数学笛卡尔坐标下顺时针旋转的变换矩阵
    double t3[3][3] = { { 1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }, { 0.5*dst_img.width(), 0.5*dst_img.height() ,1.0} }; // 将数学笛卡尔坐标映射到旋转后的图像坐标
    cv::Mat tran3 = cv::Mat(3.0,3.0,CV_64FC1,t3);
    cv::Mat T = tran1*tran2*tran3;
    cv::Mat T_inv = T.inv(); // 求逆矩阵

    for (double i = 0.0; i < dst_rows; i++){
        for (double j = 0.0; j < dst_cols; j++){
            cv::Mat dst_coordinate = (cv::Mat_<double>(1, 3) << j, i, 1.0);
            cv::Mat src_coordinate = dst_coordinate * T_inv;
            double v = src_coordinate.at<double>(0, 0);
            double w = src_coordinate.at<double>(0, 1);

            //双线性插值
            if (int(Angle) % 90 == 0) {
                if (v < 0)
                    v = 0;
                if (v > src_img.width() - 1)
                    v = src_img.width() - 1;
                if (w < 0)
                    w = 0;
                if (w > src_img.width() - 1)
                    w = src_img.width() - 1;
            }

            if (v >= 0 && w >= 0 && v <= src_img.width() - 1 && w <= src_img.height() - 1){
                int top = floor(w), bottom = ceil(w), left = floor(v), right = ceil(v);
                double pw = w - top ;
                double pv = v - left;
                //if (src.depth() == 1){
                int temp = (1 - pw)*(1 - pv)*raw[left * src_img.width()+top] + (1 - pw)*pv*raw[right * src_img.width()+top] + pw*(1 - pv)*raw[left * src_img.width()+bottom] + pw*pv*raw[right * src_img.width()+bottom];
                    //int temp = (1 - pw)*(1 - pv)*src.pixel(top, left) + (1 - pw)*pv*src.pixel(top, right) + pw*(1 - pv)*src.pixel(bottom, left) + pw*pv*src.pixel(bottom, right);
                temp = temp * 255 / 4095;
                dst_img.setPixel(j, i, qRgb(temp, temp, temp));
                /*}
                else{
                    int temp = (1 - pw)*(1 - pv)*src.pixel(top, left) + (1 - pw)*pv*src.pixel(top, right) + pw*(1 - pv)*src.pixel(bottom, left) + pw*pv*src.pixel(bottom, right);
                    dst_img.setPixel(j, i, qRgb(temp, temp, temp));
                }*/
            }
        }
    }

    //show_image2(dst);
    //ui->label->setText("123");
}

void show_img::rotate2_img(QImage &src, int Angle, double cx, double cy){
    bool bRet = false;

    double angle = Angle*CV_PI / 180.0;
    //构造输出图像
    int dst_rows = round(fabs(src.height() * cos(angle)) + fabs(src.width() * sin(angle)));//图像高度
    int dst_cols = round(fabs(src.width() * cos(angle)) + fabs(src.height() * sin(angle)));//图像宽度

    if (src.depth() == 1) {
        dst_img = QImage(dst_cols, dst_rows, QImage::Format_RGBX64); //灰度图初始
        dst_img.fill(0);
    }
    else {
        dst_img = QImage(dst_cols, dst_rows, QImage::Format_ARGB32); //RGB图初始
        dst_img.fill(0);
    }

    cv::Mat tran1 = (cv::Mat_<double>(3,3) << 1.0,0.0,0.0 , 0.0,-1.0,0.0, -cx , cy , 1.0); // 将原图像坐标映射到数学笛卡尔坐标
    cv::Mat tran2 = (cv::Mat_<double>(3,3) << cos(angle),-sin(angle),0.0 , sin(angle), cos(angle),0.0, 0.0,0.0,1.0); //数学笛卡尔坐标下顺时针旋转的变换矩阵
    double t3[3][3] = { { 1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }, { cx, cy ,1.0} }; // 将数学笛卡尔坐标映射到旋转后的图像坐标
    cv::Mat tran3 = cv::Mat(3.0,3.0,CV_64FC1,t3);
    cv::Mat T = tran1*tran2*tran3;
    cv::Mat T_inv = T.inv(); // 求逆矩阵

    for (double i = 0.0; i < dst_rows; i++){
        for (double j = 0.0; j < dst_cols; j++){
            cv::Mat dst_coordinate = (cv::Mat_<double>(1, 3) << j, i, 1.0);
            cv::Mat src_coordinate = dst_coordinate * T_inv;
            double v = src_coordinate.at<double>(0, 0);
            double w = src_coordinate.at<double>(0, 1);

            //双线性插值
            if (int(Angle) % 90 == 0) {
                if (v < 0)
                    v = 0;
                if (v > src.width() - 1)
                    v = src.width() - 1;
                if (w < 0)
                    w = 0;
                if (w > src.width() - 1)
                    w = src.width() - 1;
            }

            if (v >= 0 && w >= 0 && v <= src.width() - 1 && w <= src.height() - 1){
                int top = floor(w), bottom = ceil(w), left = floor(v), right = ceil(v);
                double pw = w - top ;
                double pv = v - left;
                if (src.depth() == 1){
                    int temp = (1 - pw)*(1 - pv)*raw[left * src_img.width()+top] + (1 - pw)*pv*raw[right * src_img.width()+top] + pw*(1 - pv)*raw[left * src_img.width()+bottom] + pw*pv*raw[right * src_img.width()+bottom];
                    dst_img.setPixel(j, i, qRgb(temp, temp, temp));
                }
                else{
                    int temp = (1 - pw)*(1 - pv)*src.pixel(top, left) + (1 - pw)*pv*src.pixel(top, right) + pw*(1 - pv)*src.pixel(bottom, left) + pw*pv*src.pixel(bottom, right);
                    dst_img.setPixel(j, i, qRgb(temp, temp, temp));
                }
            }
        }
    }

    //show_image2(dst);
    //ui->label->setText("123");
}

void show_img::getbit(){
    FILE *pfRaw = fopen(filepath.toLatin1(), "rb");
    if(!pfRaw){
        QMessageBox::critical(this, tr("错误"), tr("文件打开失败！"),
                              QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);
        return;
    }

    //std::cout<<filepath.toStdString();

    //unsigned char *data = (unsigned char *)malloc(sizeof(unsigned long));
    uint8_t temp[4];
    fread(temp, 1, 4, pfRaw);
    fread(temp, 1, 4, pfRaw);

    FILE *pf = fopen(filepath.toLatin1(), "rb");
    unsigned char data[4];
    fread(data, 1, 4, pf);
    int t1 = data[0] - '0' + 48;
    int t2 = data[1] - '0' + 48;
    t2 *= 256;
    iWidth = t1 + t2;

    fread(data, 1, 4, pf);
    t1 = data[0] - '0' + 48;
    t2 = data[1] - '0' + 48;
    t2 *= 256;
    iHeight = t1 + t2;

    pushRawdata2 = (unsigned short *)malloc(sizeof(unsigned short) * iWidth * iHeight);
    fread(pushRawdata2, iWidth * iHeight * sizeof(unsigned short), 1, pf);
    fclose(pf);
    for(int i = 0; i < iHeight * iWidth; i++){
            pushRawdata2[i] = pushRawdata2[i]>>4;
        }


    unsigned char *pushRawdata = (unsigned char *)malloc(sizeof(unsigned char) * iWidth * iHeight * 2);

    fread(pushRawdata, iWidth * iHeight * sizeof(unsigned char) * 2, 1, pfRaw);
    fclose(pfRaw);

    raw = (int *)malloc(sizeof(int) * iWidth * iHeight);
    //unsigned char *raw = (unsigned char *)malloc(sizeof(unsigned char) * iWidth * iHeight);
    //unsigned char *raw1 = (unsigned char *)malloc(sizeof(unsigned char) * iWidth * iHeight);
    for(int i = 0; i < iHeight * iWidth; i++){
        t1 = pushRawdata[2 * i] - '0' + 48;
        t2 = pushRawdata[2 * i + 1] - '0' + 48;
        //t1 %= 16;
        t2 *= 256;
        raw[i] = t1 + t2;
        //std::cout<<t1 + t2<<std::endl;
    }

    switch(mytype){
    case _LAB:
    {
        cv::Mat image(cv::Size(iWidth, iHeight), CV_8UC1, raw);
        //cv::Mat image(cv::Size(iWidth, iHeight), CV_8UC1, raw);
        src = image;
        this->show_image(image);
        break;
    }
    case _RAW:
    {
        this->Mattix2QImage(raw, iWidth, iHeight);
        this->show_image3(src_img);
        break;
    }
    default:
        break;
    }


    return;
}

QImage show_img::Mattix2QImage(int *data, int width, int height){
    QImage tempImg = QImage(width, height, QImage::Format_RGB16);

    int num = 0;
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            int temp = data[num] * 4095 / 4095;
            //tempImg.setPixel(j, i, temp);
            tempImg.setPixelColor(j, i, qRgb(temp, temp, temp));
            num++;
            //tempImg.setPixel(i, j, qRgba64(4000, 4000, 4000, 4000));
        }
    }

    src_img = tempImg;
    return tempImg;
}

int show_img::convert_hex_inv(size_t count, const uint8_t *src) {
    QString temp = " ";
    std::ostringstream strStream;
    strStream << (uint16_t)src[1];
    temp += QString::fromStdString(strStream.str());
    strStream.clear();
    strStream << (uint16_t)src[0];
    temp += QString::fromStdString(strStream.str());
    strStream.clear();
    strStream << (uint16_t)src[2];
    temp += QString::fromStdString(strStream.str());
    strStream.clear();
    strStream << (uint16_t)src[3];
    temp += QString::fromStdString(strStream.str());


    int j=0;
    QChar cha( '0' );
    while(temp.at(j)==cha)
        j++;
    temp = temp.right(temp.size()-j);
    return temp.toInt( );
}

int show_img::convert(int ww, int wl, int tar){
    int res = 0;
    /*if(tar > wl + ww/2)
        res = 255;
    else if(tar < wl - ww/2)
        res = 0;
    else{
        float a = 256/ww;
        float b  = (ww/2 - wl) * 256 / ww;
        res = a * tar + b;
    }
    return res;*/

    double min = (2 * wl - ww)/2.0 + 0.5;
    double max = (2 * wl + ww)/2.0 + 0.5;

    double factor = 255.0 / (double)(max - min);

    if(tar < min) return 0;
    if(tar > max) return 255;
    res = (int)((tar - min) * factor);

    if(res < 0) return 0;
    if(res > 255) return 255;
    return res;
}

void show_img::my_window(cv::Mat &src, int ww, int wl){
    dst = src.clone();
    for(int i = 0; i < src.rows; i++){
        for(int j = 0; j < src.cols; j++){
            if(src.channels() == 1){
                int temp = src.at<uchar>(i, j);
                dst.at<uchar>(i, j) = this->convert(ww, wl, temp);
            }else{
                int temp = dst.at<cv::Vec3b>(i, j)[0];
                dst.at<cv::Vec3b>(i, j)[0] = convert(ww, wl, temp);

                temp = dst.at<cv::Vec3b>(i, j)[1];
                dst.at<cv::Vec3b>(i, j)[1] = convert(ww, wl, temp);

                temp = dst.at<cv::Vec3b>(i, j)[2];
                dst.at<cv::Vec3b>(i, j)[2] = convert(ww, wl, temp);
            }
        }
    }
}

int show_img::convert2(int ww, int wl, int tar){
    int res = 0;
    double min = (2 * wl - ww)/2.0 + 0.5;
    double max = (2 * wl + ww)/2.0 + 0.5;

    double factor = 4096.0 / (double)(max - min);

    if(tar < min) return 0;
    if(tar > max) return 4095.0;
    res = (int)((tar - min) * factor);

    if(res < 0) return 0;
    if(res > 4095.0) return 4095.0;
    return res;
}

void show_img::my_window(QImage &src, int ww, int wl){
    QImage tempImg = QImage(src.width(), src.height(), QImage::Format_RGB16);

    int num = 0;
    for (int i = 0; i < src.height(); ++i)
    {
        for (int j = 0; j < src.width(); ++j)
        {
            int temp = raw[num];
            //tempImg.setPixel(j, i, temp);
            temp = convert(ww, wl, temp);
            tempImg.setPixelColor(j, i, qRgb(temp, temp, temp));
            num++;
            //tempImg.setPixel(i, j, qRgba64(4000, 4000, 4000, 4000));
        }
    }

    dst_img = tempImg;
}

show_img::~show_img()
{
    delete ui;
}

void show_img::on_pushButton_clicked()
{
    switch (mytype) {
    case _LAB:
    {
        cv::Mat dst_t = src;
        int pross = 0;//whether the dst has data

        if(ui->angle->document()->isEmpty())
            pross = 0;
        else{
            this->rotate(dst_t, ui->angle->toPlainText().toInt());
            pross = 1;
            dst_t = dst;
        }

        if(ui->angle_2->document()->isEmpty()){
            if(pross == 0)
                pross = 0;
        }
        else{
            this->rotate2(dst_t, ui->angle_2->toPlainText().toInt(), ui->rotate_x->toPlainText().toDouble(), ui->rotate_y->toPlainText().toDouble());
            pross = 1;
            dst_t = dst;
        }

        if(ui->scale_x->document()->isEmpty() || ui->scale_y->document()->isEmpty()){
            //empty
        }else{
            dst_t = dst;
            this->scale(dst_t, ui->scale_x->toPlainText().toDouble(), ui->scale_y->toPlainText().toDouble());
            dst_t = dst;
        }

        if(ui->w_loc->document()->isEmpty() || ui->w_wid->document()->isEmpty()){
            //empty
        }else{
            this->my_window(dst_t, ui->w_wid->toPlainText().toInt(), ui->w_loc->toPlainText().toInt());
            dst_t = dst;
        }
        show_image2(dst);
        //ui->label->setText("123");
        break;
    }
    case _RAW:
    {
        QImage dst_t = src_img;

        if(ui->angle->document()->isEmpty())
            ;//empty
        else{
            this->rotate_img(dst_t, ui->angle->toPlainText().toInt());
            QMatrix matrix; matrix.rotate(ui->angle->toPlainText().toInt()); dst_img =  dst_t.transformed(matrix,Qt::FastTransformation);
            show_image4(dst_img);
            dst_t = dst_img;
        }

        if(ui->angle_2->document()->isEmpty()){
        }
        else{
            this->rotate2_img(dst_t, ui->angle_2->toPlainText().toInt(), ui->rotate_x->toPlainText().toDouble(), ui->rotate_y->toPlainText().toDouble());
           QMatrix matrix; matrix.rotate(ui->angle->toPlainText().toInt()); dst_img =  dst_t.transformed(matrix,Qt::FastTransformation);
           show_image4(dst_img);
            dst_t = dst_img;
        }

        if(ui->scale_x->document()->isEmpty() || ui->scale_y->document()->isEmpty()){
            //empty
        }else{
            this->scale_img(dst_t, ui->scale_x->toPlainText().toDouble(), ui->scale_y->toPlainText().toDouble());
            dst_img = dst_t.scaled(dst_t.width()*ui->scale_x->toPlainText().toDouble(), dst_t.height()*ui->scale_y->toPlainText().toDouble(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            show_image4(dst_img);
            dst_t = dst_img;
        }

        if(ui->w_loc->document()->isEmpty() || ui->w_wid->document()->isEmpty()){
            //empty
        }else{
            this->my_window(src_img, ui->w_wid->toPlainText().toInt(), ui->w_loc->toPlainText().toInt());
            show_image4(dst_img);
            dst_t = dst_img;
        }
        //show_image4(dst_img);
        //ui->label->setText("123");
        break;
    }
    default:
        break;
    }
}

void show_img::on_pushButton_2_clicked()
{
    ui->angle->clear();
    ui->angle_2->clear();
    ui->rotate_x->clear();
    ui->rotate_y->clear();
    ui->scale_x->clear();
    ui->scale_y->clear();
    ui->w_loc->clear();
    ui->w_wid->clear();
    dst_img = src_img;
    dst = src;
}

void show_img::on_actionselect_triggered()
{
    Lab3 *w = new Lab3();
    this->close();
    w->show();
}
