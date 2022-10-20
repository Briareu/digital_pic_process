#include "show_img.h"
#include "ui_show_img.h"

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

show_img::show_img(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::show_img)
{
    QString str = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg");
    this->filepath = str;

    cv::Mat src = cv::imread(filepath.toStdString());

    ui->setupUi(this);
    show_image(src);
}

void show_img::show_image(const cv::Mat &src){
    QImage Img = cvMat2QImage(src);

    QGraphicsScene *scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(Img));
    ui->pic1->setScene(scene);
    ui->pic1->show();

}

void show_img::show_image2(const cv::Mat &src){
    QImage Img = cvMat2QImage(src);

    QGraphicsScene *scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(Img));
    ui->pic2->setScene(scene);
    ui->pic2->show();

}

QImage show_img::cvMat2QImage(const cv::Mat &mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if (mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
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

void show_img::getbit(){
    char *ch;
    QByteArray ba = filepath.toLatin1();
    ch = ba.data();
    FILE *fp = fopen(ch, "rb");
    if(!fp){
        QMessageBox::critical(this, tr("错误"), tr("文件打开失败！"),
                              QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);
        return;
    }
    unsigned char *imagedata = NULL;

    long width, height;
    BITMAPFILEHEADER fileHead;
    fread(&fileHead, sizeof(BITMAPFILEHEADER), 1, fp);
}

show_img::~show_img()
{
    delete ui;
}

void show_img::on_pushButton_clicked()
{
    cv::Mat src, dst_t;
    src = cv::imread(filepath.toStdString());
    int pross = 0;//whether the dst has data

    if(ui->angle->document()->isEmpty())
        pross = 0;
    else{
        this->rotate(src, ui->angle->toPlainText().toInt());
        pross = 1;
        src = dst;
    }

    if(ui->angle_2->document()->isEmpty()){
        if(pross == 0)
            pross = 0;
    }
    else{
        this->rotate2(src, ui->angle_2->toPlainText().toInt(), ui->rotate_x->toPlainText().toDouble(), ui->rotate_y->toPlainText().toDouble());
        pross = 1;
        src = dst;
    }

    if(ui->scale_x->document()->isEmpty() || ui->scale_y->document()->isEmpty()){
        //empty
    }else{
        if(pross == 1){
            dst_t = dst;
            this->scale(dst_t, ui->scale_x->toPlainText().toDouble(), ui->scale_y->toPlainText().toDouble());
        }else{
            this->scale(src, ui->scale_x->toPlainText().toDouble(), ui->scale_y->toPlainText().toDouble());
            pross = 1;
        }
    }
    show_image2(dst);
    //ui->label->setText("123");
}

void show_img::on_pushButton_2_clicked()
{
    ui->angle->clear();
    ui->angle_2->clear();
    ui->rotate_x->clear();
    ui->rotate_y->clear();
    ui->scale_x->clear();
    ui->scale_y->clear();
}
