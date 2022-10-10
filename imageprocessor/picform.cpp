#include "picform.h"
#include "ui_picform.h"
#include "picType.h"
#include <QTextCodec>
#include <malloc.h>
#include <windows.h>
#include <QMessageBox>
#include "mywidget.h"
#include <iostream>
#include <QInputDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <fstream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>

using namespace std;
using namespace cv;

#define pi 3.1415926

//构造函数
picForm::picForm(QString path, int type, double x, double y, double a, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::picForm)
{
    //防止编码不一致导致的显示问题
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");    //获取系统编码
    QTextCodec::setCodecForLocale(codec);

    filepath = path;
    ui->setupUi(this);
    switch (type) {
    case 0:
    {
        m_type = NORMAL;
        m_show();
        setWindowTitle("原图像");
        //显示界面
        ui->setupUi(this);
        break;
    }
    case 1:
    {
        m_type = BMP;
        setWindowTitle("原图像");
        getBitsForBMP();

        ui->setupUi(this);
        break;
    }
    case 2:
    {
        m_type = _FT;
        this->frequencyFiltering();
        //直接进行变化，由imshow进行图像显示

        break;
    }
    case 3:
    {
        m_type = _EQUAL;
        this->_equalize();

       // ui->setupUi(this);
        break;
    }
    case 4:
    {
        m_type = _COLOREQUAL;
        this->_equalize2();
        break;
    }
    case 5:
    {
        m_type = _ROTATE;
        this->rotate();
        break;
    }
    case 6:
    {
        m_type = _SCALE;
        setWindowTitle("dst");
        cx = x;
        cy = y;
        this->scale();

        ui->setupUi(this);
        break;
    }
    case 7:
    {
        m_type = _ROTATE2;
        cx = x;
        cy = y;
        _angle = a;
        this->rotate2();
        break;
    }
    default:
        break;
    }

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(on_pushButton_clicked()));
}

void picForm::m_show(){
    if(filepath.trimmed().isEmpty() == false){
        QPixmap picture;
        picture.load(filepath);
        picture = picture.scaled(ui->picLabel->width(), ui->picLabel->height());
        ui->picLabel->setPixmap(picture);
    }
}

void picForm::getBitsForBMP(){
    char *ch;
    QByteArray ba = filepath.toLatin1();
    ch = ba.data();
    FILE *fp = fopen(ch, "rb");
    if(!fp){
        QMessageBox::critical(this, tr("错误"), tr("文件打开失败！"),
                              QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);
        return;
    }

    long width, height;
    BITMAPFILEHEADER fileHead;
    fread(&fileHead, sizeof(BITMAPFILEHEADER), 1, fp);
    BITMAPINFOHEADER infoHead;
    fread(&infoHead, sizeof(BITMAPINFOHEADER), 1, fp);
    width = infoHead.biWidth;
    height = infoHead.biHeight;

    char *bmpBuf = new char[width * height * 3];
    fseek(fp, long(sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER)), 0);
    fread(bmpBuf, sizeof (char), width * height * 3, fp);
    fclose(fp);

    FILE *outfile = fopen("show.bmp", "wb");//output
    fwrite(&fileHead, sizeof(BITMAPFILEHEADER), 1, outfile);
    fwrite(&infoHead, sizeof(BITMAPINFOHEADER), 1, outfile);
    char *tmp = bmpBuf;
    for(int j = 0; j < height; j++){
        for(int i = 0; i < width * 3; i++){
            fwrite(tmp++, 1, 1, outfile);
        }
    }
    fclose(outfile);

    HWND hwnd = GetForegroundWindow();
    HDC hdc = GetDC(hwnd);
    BYTE b, g, r;
    unsigned char *imagedata = NULL;
    int n = ((width * 3)%4 == 0) ? 0 : 4 - (width * 3)%4;
    int imageLen = (width * 3 + n) * height + 54;
    imagedata = (unsigned char*)malloc(imageLen);
    if((outfile = fopen("show.bmp", "r")) == NULL){
        QMessageBox::critical(this, tr("错误"), tr("文件打开失败！"),
                              QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);
        return;
    }
    fread(imagedata, 1, imageLen + 1, outfile);
   /* for(int j = 0; j < height; j++){
        for(int i = 0; i < width; i++){
            b = *tmp++;
            g = *tmp++;
            r = *tmp++;
            SetPixel(hdc, i, 150 + height - j, RGB(r, g, b));
        }
    }*/
    QImage image(imagedata + 54, width, height, QImage::Format_RGB888);
    QMatrix matrix;
    matrix.rotate(0);
    QImage m_image = image.transformed(matrix).rgbSwapped();
    m_image = m_image.mirrored();
    ui->picLabel->setPixmap(QPixmap::fromImage(m_image));
    ui->picLabel->resize(ui->picLabel->pixmap()->size());
}

cv::Mat picForm::FT(cv::Mat &srcImage){
    Mat paddingImage;

    int m = cv::getOptimalDFTSize(srcImage.rows);
    int n = cv::getOptimalDFTSize(srcImage.cols);

    cv::copyMakeBorder(srcImage, paddingImage, 0, m - srcImage.rows,
                       0, n - srcImage.cols, cv::BORDER_CONSTANT, cv::Scalar(0));

    cv::Mat planes[] = {cv::Mat_<float>(paddingImage), Mat::zeros(paddingImage.size(), CV_32FC1)};
    cv::Mat mergeImage;
    cv::merge(planes, 2, mergeImage);

    cv::dft(mergeImage, mergeImage, cv::DFT_COMPLEX_OUTPUT);

    return mergeImage;
}

cv::Mat picForm::getMagnitudeImage(const cv::Mat &fourierImage){
    cv::Mat planes[] = {cv::Mat::zeros(fourierImage.size(), CV_32FC1),
                            cv::Mat::zeros(fourierImage.size(), CV_32FC1)};

        cv::Mat magImage = planes[0].clone();
        cv::split(fourierImage, planes);
        cv::magnitude(planes[0], planes[1], magImage);


        magImage = magImage(cv::Rect(0, 0, magImage.cols-(magImage.cols%2), magImage.rows-(magImage.rows%2)));


        return magImage;
}

//中心化
cv::Mat picForm::changeCenter(const cv::Mat &magImage){
    int centerX = magImage.cols / 2;
        int centerY = magImage.rows / 2;

        cv::Mat magImageCopy = magImage.clone();
        cv::Mat planes[] = {cv::Mat::zeros(magImageCopy.size(), CV_32FC1),
                            cv::Mat::zeros(magImageCopy.size(), CV_32FC1)};

        cv::Mat mat1(magImageCopy, cv::Rect(0, 0, centerX, centerY));				//左上
        cv::Mat mat2(magImageCopy, cv::Rect(0, centerY, centerX, centerY));			//右上
        cv::Mat mat3(magImageCopy, cv::Rect(centerX, 0, centerX, centerY));				//左下
        cv::Mat mat4(magImageCopy, cv::Rect(centerX, centerY, centerX, centerY));	//右下

        //互换左上和右下
        cv::Mat tempImage;
        mat1.copyTo(tempImage);
        mat4.copyTo(mat1);
        tempImage.copyTo(mat4);

        //互换左下和右上
        mat2.copyTo(tempImage);
        mat3.copyTo(mat2);
        tempImage.copyTo(mat3);

        return magImageCopy;
}

cv::Mat picForm::getPhaseImage(const cv::Mat &fourierImage){
    cv::Mat planes[] = {cv::Mat::zeros(fourierImage.size(), CV_32FC1),
                            cv::Mat::zeros(fourierImage.size(), CV_32FC1)};

        cv::Mat phaseImage = planes[0].clone();
        cv::split(fourierImage, planes);
        cv::phase(planes[0], planes[1], phaseImage);
        //imshow("phase", phaseImage);
        return phaseImage;
}

cv::Mat picForm::inverseFT(const cv::Mat &fourierImage){
        cv::Mat dealtImage = fourierImage.clone();
        cv::Mat iDFT[] = {cv::Mat::zeros(dealtImage.size(), CV_32FC1),
                          cv::Mat::zeros(dealtImage.size(), CV_32FC1)};

        cv::idft(dealtImage, dealtImage);
        cv::split(dealtImage, iDFT);
        normalize(iDFT[0], iDFT[0], 0, 1, NORM_MINMAX);
        return iDFT[0];
}

void picForm::frequencyFiltering(){
    Mat srcImage = cv::imread(filepath.toStdString(), 0);
    cv::Mat fourierTransformImage = FT(srcImage);

    cv::Mat magImage = getMagnitudeImage(fourierTransformImage);

    magImage = changeCenter(magImage);

    cv::log(magImage + 1, magImage);

    cv::normalize(magImage, magImage, 0, 1, NORM_MINMAX);

    cv::imshow("originImage", srcImage);
    cv::imshow("test2", magImage);

    auto rImage = inverseFT(fourierTransformImage);

    cv::imshow("inverseFourierTransformResult", rImage);
}

void picForm::_equalize(){
    Mat src, dst;
    src = cv::imread(filepath.toStdString());
    if(!src.data){
        QMessageBox::critical(this, tr("错误"), tr("文件打开失败！"),
                              QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);
        return;
    }

    cvtColor(src, src, CV_BGR2GRAY);
    equalizeHist(src, dst);
    char INPUT_T[] = "input image";
    char OUTPUT_T[] = "result image";
    namedWindow(INPUT_T, CV_WINDOW_AUTOSIZE);
    namedWindow(OUTPUT_T, CV_WINDOW_AUTOSIZE);

    imshow(INPUT_T, src);
    imshow(OUTPUT_T, dst);
}

void picForm::_equalize2(){
    Mat src, dst, dst1;
    src = cv::imread(filepath.toStdString());
    if(!src.data){
        QMessageBox::critical(this, tr("错误"), tr("文件打开失败！"),
                              QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);
        return;
    }
    char INPUT_T[] = "input image";
    char OUTPUT_T[] = "result image";

    namedWindow(INPUT_T, CV_WINDOW_AUTOSIZE);
    namedWindow(OUTPUT_T, CV_WINDOW_AUTOSIZE);

    imshow(INPUT_T, src);

    vector<Mat> Channels;
    split(src, Channels);

    Mat b,g,r;
    b = Channels.at(0);
    g = Channels.at(1);
    r = Channels.at(2);

    equalizeHist(b, b);
    equalizeHist(g, g);
    equalizeHist(r, r);

    merge(Channels, dst);
    imshow(OUTPUT_T, dst);
}

void picForm::rotate(){
    cv::Mat dst;
    double Angle;
    bool bRet = false;
    Angle = QInputDialog::getDouble(this, "输入", "角度", 0.0, -360.0, 360.0, 1, &bRet);
    if(!bRet)
        return;
    cv::Mat src = cv::imread(filepath.toStdString());
    if(!src.data){
        QMessageBox::critical(this, tr("错误"), tr("文件打开失败！"),
                              QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);
        return;
    }

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

        cv::namedWindow("src");
        cv::imshow("src", src);
        //cv::namedWindow("dst",1);
        cv::imshow("dst", dst);
}

void picForm::rotate2(){
    cv::Mat dst;

    cv::Mat src = cv::imread(filepath.toStdString());
    if(!src.data){
        QMessageBox::critical(this, tr("错误"), tr("文件打开失败！"),
                              QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);
        return;
    }

    //int tran_x = cx

    double angle = _angle*CV_PI / 180.0;
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

    cv::Mat tran1 = (cv::Mat_<double>(3,3) << 1.0,0.0,0.0 , 0.0,-1.0,0.0, -cy , cx , 1.0); // 将原图像坐标映射到数学笛卡尔坐标
    cv::Mat tran2 = (cv::Mat_<double>(3,3) << cos(angle),-sin(angle),0.0 , sin(angle), cos(angle),0.0, 0.0,0.0,1.0); //数学笛卡尔坐标下顺时针旋转的变换矩阵
    double t3[3][3] = { { 1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }, { cy, cx ,1.0} }; // 将数学笛卡尔坐标映射到旋转后的图像坐标
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
            if (int(_angle) % 90 == 0) {
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




    cv::namedWindow("src");
    cv::imshow("src", src);
    //cv::namedWindow("dst",1);
    cv::imshow("dst", dst);
}

void picForm::scale(){
    cv::Mat dst;
    cv::Mat src = cv::imread(filepath.toStdString());
    if(!src.data){
        QMessageBox::critical(this, tr("错误"), tr("文件打开失败！"),
                              QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);
        return;
    }

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
            // 判断是否越界
            if (v < 0) v = 0; if (v > src.cols - 1) v = src.cols - 1;
            if (w < 0) w = 0; if (w > src.rows - 1) w = src.rows - 1;

            if (v >= 0 && w >= 0 && v <= src.cols - 1 && w <= src.rows - 1){
                int top = floor(w), bottom = ceil(w), left = floor(v), right = ceil(v); //与映射到原图坐标相邻的四个像素点的坐标
                double pw = w - top; //pw为坐标 行 的小数部分(坐标偏差)
                double pv = v - left; //pv为坐标 列 的小数部分(坐标偏差)
                if (src.channels() == 1){
                    //灰度图像
                    dst.at<uchar>(i, j) = (1 - pw)*(1 - pv)*src.at<uchar>(top, left) + (1 - pw)*pv*src.at<uchar>(top, right) + pw*(1 - pv)*src.at<uchar>(bottom, left) + pw*pv*src.at<uchar>(bottom, right);
                }
                else{
                    //彩色图像
                    dst.at<cv::Vec3b>(i, j)[0] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[0] + (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[0] + pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[0] + pw*pv*src.at<cv::Vec3b>(bottom, right)[0];
                    dst.at<cv::Vec3b>(i, j)[1] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[1] + (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[1] + pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[1] + pw*pv*src.at<cv::Vec3b>(bottom, right)[1];
                    dst.at<cv::Vec3b>(i, j)[2] = (1 - pw)*(1 - pv)*src.at<cv::Vec3b>(top, left)[2] + (1 - pw)*pv*src.at<cv::Vec3b>(top, right)[2] + pw*(1 - pv)*src.at<cv::Vec3b>(bottom, left)[2] + pw*pv*src.at<cv::Vec3b>(bottom, right)[2];
                }
            }
        }
    }
    cv::namedWindow("src");
    cv::imshow("src", src);
    //cv::namedWindow("dst",0);
    //cv::resizeWindow("dst", 480, 480);
    //cv::imshow("dst", dst);

    cv::Mat Rgb;
    QImage Img = cvMat2QImage(dst);
    ui->picLabel->setPixmap(QPixmap::fromImage(Img));

    ui->picLabel->setFixedSize(QSize(dst.cols , dst.rows));
    ui->picLabel->show();
    //ui->picLabel->resize(ui->picLabel->pixmap()->size());
    //ui->picLabel->adjustSize();
    //ui->scrollArea->setWidget(ui->picLabel);
}

QImage picForm::cvMat2QImage(const cv::Mat &mat)
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

picForm::~picForm()
{
    delete ui;
}

void picForm::on_pushButton_clicked()
{
    switch (m_type) {
    case NORMAL:
    {
        myWidget *w = new myWidget();
        this->close();
        w->show();
        break;
    }
    case BMP:
    {
        myWidget *w = new myWidget();
        this->close();
        w->show();
        break;
    }
    case _FT:
    {
        break;
    }
    case _EQUAL:
    {
        break;
    }
    case _COLOREQUAL:
    {
        break;
    }
    case _ROTATE:
    {
        break;
    }
    case _SCALE:
    {
        myWidget *w = new myWidget();
        this->close();
        w->show();
        break;
    }
    case _ROTATE2:
    {
        break;
    }
    }
}

