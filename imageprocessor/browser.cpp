#include "browser.h"
#include "ui_browser.h"

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
#include <QVector>
#include <cstdlib>
#include <algorithm>
#include <QtGlobal>
#include <QMessageBox>

int *raw_latter;
int *raw_temp_ptr;
int *raw_window;

Browser::Browser(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Browser)
{
    ui->setupUi(this);
}

Browser::~Browser()
{
    delete ui;
}

QImage Browser::Matrix2QImage(int *data, int width, int height){
    QImage tempImg = QImage(width, height, QImage::Format_RGB16);

    int num = 0;
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            int temp = data[num];
            tempImg.setPixelColor(j, i, qRgb(temp, temp, temp));
            num++;
        }
    }

    return tempImg;
}

void Browser::show_image(QImage Img){
    QGraphicsScene *scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(Img));
    ui->pic->setScene(scene);
    ui->pic->show();
}

void Browser::show_image(const cv::Mat &image){
    QImage Img = cvMat2QImage(image);

    QGraphicsScene *scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(Img));
    ui->pic->setScene(scene);
    ui->pic->show();
}

QImage Browser::cvMat2QImage(const cv::Mat &mat){
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
}

void Browser::getbit(){
    FILE *pfRaw = fopen(filepath.toLatin1(), "rb");
    if(!pfRaw){
        QMessageBox::critical(this, tr("错误"), tr("文件打开失败！"),
                              QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);
        return;
    }

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
    iWidth_temp = t1 + t2;
    iWidth_window = t1 + t2;

    fread(data, 1, 4, pf);
    t1 = data[0] - '0' + 48;
    t2 = data[1] - '0' + 48;
    t2 *= 256;
    iHeight = t1 + t2;
    iHeight_temp = t1 + t2;
    iHeight_window = t1 + t2;
    raw_latter = (int *)malloc(sizeof(int) * iWidth * iHeight);
    raw_window = (int *)malloc(sizeof(int) * iWidth * iHeight);

    unsigned short *pushRawdata2;
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
    for(int i = 0; i < iHeight * iWidth; i++){
        t1 = pushRawdata[2 * i] - '0' + 48;
        t2 = pushRawdata[2 * i + 1] - '0' + 48;
        //t1 %= 16;
        t2 *= 256;
        raw[i] = t1 + t2;
        raw_latter[i] = t1 + t2;
        raw_window[i] = t1 + t2;
        //std::cout<<t1 + t2<<std::endl;
    }

    src_img = this->Matrix2QImage(raw, iWidth, iHeight);
    dst_img = src_img;
    this->show_image(src_img);

    return;
}

void Browser::my_window(cv::Mat &src, int ww, int wl){
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

int Browser::convert(int ww, int wl, int tar){
    int res = 0;

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

void Browser::my_window(QImage &src, int ww, int wl){
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

void Browser::my_window(int *raw_p, int ww, int wl){
    QImage tempImg = QImage(iWidth_temp, iHeight_temp, QImage::Format_RGB16);

    int num = 0;
    for (int i = 0; i < iHeight_temp; ++i)
    {
        for (int j = 0; j < iWidth_temp; ++j)
        {
            int temp = raw_window[num];
            //tempImg.setPixel(j, i, temp);
            temp = convert(ww, wl, temp);
            tempImg.setPixelColor(j, i, qRgb(temp, temp, temp));
            raw_p[num] = temp;
            num++;
            //tempImg.setPixel(i, j, qRgba64(4000, 4000, 4000, 4000));
        }
    }

    dst_img = tempImg;
}

int Browser::convert2(int ww, int wl, int tar){
    int res = 0;
    double min = (2 * wl - ww)/2.0 + 0.5;
    double max = (2 * wl + ww)/2.0 + 0.5;

    double factor = 4095.0 / (double)(max - min);

    if(tar < min) return 0;
    if(tar > max) return 4095.0;
    res = (int)((tar - min) * factor);

    if(res < 0) return 0;
    if(res > 4095.0) return 4095.0;
    return res;
}

void Browser::intensify(QImage &Img){
    QImage tempImg = QImage(iWidth_temp, iHeight_temp, QImage::Format_RGB16);

    for (int i = 1; i < iHeight_temp - 1; i++)
    {
        for (int j = 1; j < iWidth_temp - 1; j++)
        {
            QColor imageKernel[9];
            imageKernel[0] = QColor(Img.pixel(j - 1, i - 1));
            imageKernel[1] = QColor(Img.pixel(j, i - 1));
            imageKernel[2] = QColor(Img.pixel(j + 1, i - 1));
            imageKernel[3] = QColor(Img.pixel(j - 1, i));
            imageKernel[4] = QColor(Img.pixel(j, i));
            imageKernel[5] = QColor(Img.pixel(j + 1, i));
            imageKernel[6] = QColor(Img.pixel(j - 1, i + 1));
            imageKernel[7] = QColor(Img.pixel(j, i + 1));
            imageKernel[8] = QColor(Img.pixel(j + 1, i + 1));

            //化简后结果   这里使用了 1,1.414,1 的模板（各向同性Sobel算子），与 1,2,1的模板区别不是很大
            float GX_r = imageKernel[2].red() - imageKernel[0].red() + (imageKernel[5].red() - imageKernel[3].red()) * 1.414 + imageKernel[8].red() - imageKernel[6].red();
            float GY_r = imageKernel[0].red() + imageKernel[2].red() + (imageKernel[1].red() - imageKernel[7].red()) * 1.414 - imageKernel[6].red() - imageKernel[8].red();
            float r = sqrt(GX_r*GX_r + GY_r*GY_r) + QColor(Img.pixel(j, i)).red();

            float GX_g = imageKernel[2].green() - imageKernel[0].green() + (imageKernel[5].green() - imageKernel[3].green()) * 1.414 + imageKernel[8].green() - imageKernel[6].green();
            float GY_g = imageKernel[0].green() + imageKernel[2].green() + (imageKernel[1].green() - imageKernel[7].green()) * 1.414 - imageKernel[6].green() - imageKernel[8].green();
            float g = sqrt(GX_g*GX_g + GY_g*GY_g) + QColor(Img.pixel(j, i)).green();

            float GX_b = imageKernel[2].blue() - imageKernel[0].blue() + (imageKernel[5].blue() - imageKernel[3].blue()) * 1.414 + imageKernel[8].blue() - imageKernel[6].blue();
            float GY_b = imageKernel[0].blue() + imageKernel[2].blue() + (imageKernel[1].blue() - imageKernel[7].blue()) * 1.414 - imageKernel[6].blue() - imageKernel[8].blue();
            float b = sqrt(GX_b*GX_b + GY_b*GY_b) + QColor(Img.pixel(j, i)).blue();

            tempImg.setPixel(j, i, qRgb(r, g, b));
        }
    }
    dst_img = tempImg;
}

void Browser::intensify(){
    QImage tempImg = QImage(iWidth_temp, iHeight_temp, QImage::Format_RGB16);

    raw_temp_ptr = nullptr;
    raw_temp_ptr = (int *)malloc(sizeof(int) * iWidth_temp * iHeight_temp);
    //temp = (int *)malloc(sizeof(int) * iWidth * iHeight);

    for (int i = 1; i < iHeight_temp - 1; i++)
    {
        for (int j = 1; j < iWidth_temp - 1; j++)
        {
            unsigned char imageKernel[9] = { 0 };
            imageKernel[0] = raw_latter[(i - 1)*iWidth_temp + j - 1];
            imageKernel[1] = raw_latter[(i - 1)*iWidth_temp + j];
            imageKernel[2] = raw_latter[(i - 1)*iWidth_temp + j + 1];
            imageKernel[3] = raw_latter[(i)*iWidth_temp + j - 1];
            imageKernel[4] = raw_latter[(i)*iWidth_temp + j];
            imageKernel[5] = raw_latter[(i)*iWidth_temp + j + 1];
            imageKernel[6] = raw_latter[(i + 1)*iWidth_temp + j - 1];
            imageKernel[7] = raw_latter[(i + 1)*iWidth_temp + j];
            imageKernel[8] = raw_latter[(i + 1)*iWidth_temp + j + 1];

            //化简后结果   这里使用了 1,1.414,1 的模板（各向同性Sobel算子），与 1,2,1的模板区别不是很大
            float GX = imageKernel[2] - imageKernel[0] + (imageKernel[5] - imageKernel[3]) * 1.414 + imageKernel[8] - imageKernel[6];
            float GY = imageKernel[0] + imageKernel[2] + (imageKernel[1] - imageKernel[7]) * 1.414 - imageKernel[6] - imageKernel[8];

            //int val = LimitValue(sqrt(GX*GX + GY*GY) + 0.5);

            raw_temp_ptr[i*iWidth_temp + j] = sqrt(GX*GX + GY*GY) + raw_latter[i*iWidth_temp + j];
            //temp[i*iWidth + j] = sqrt(GX*GX + GY*GY) + imageKernel[4];
        }
    }

    tempImg = this->Matrix2QImage(raw_temp_ptr, iWidth_temp, iHeight_temp);
    dst_img = tempImg;
    for(int i = 0; i < iWidth_temp * iHeight_temp; i++)
        raw_latter[i] = raw_temp_ptr[i];
    raw_temp_ptr = nullptr;
}

void Browser::intensify(cv::Mat &Img){
    cv::Mat tempImg = Img;

    int rows = Img.rows;
    int cols = Img.cols;
    cv::Mat Dx = Img.clone();//用于存放水平方向微分算子的运算结果
    cv::Mat Dy = Img.clone();//用于存放垂直方向微分算子的运算结果

    if(Img.channels() == 1){
        //卷积
        //注意这里的i一定要从1开始，到ows-1结束。
        for (int i = 1; i < rows-1; i++)
        {
            for (int j = 1; j < cols-1; j++)
            {
                Dx.at<uchar>(i, j) = Img.at<uchar>(i + 1, j - 1) - Img.at<uchar>(i - 1, j - 1)
                        + 2 * (Img.at<uchar>(i + 1, j) - Img.at<uchar>(i - 1, j))
                        + Img.at<uchar>(i + 1, j + 1) - Img.at<uchar>(i - 1, j + 1);
                Dy.at<uchar>(i, j) = Img.at<uchar>(i - 1, j + 1) - Img.at<uchar>(i - 1, j - 1)
                        + 2 * (Img.at<uchar>(i, j + 1) - Img.at<uchar>(i, j - 1))
                        + Img.at<uchar>(i + 1, j + 1) - Img.at<uchar>(i + 1, j - 1);
            }
        }

        for (int i = 0; i < rows-1; i++)
        {
            for (int j = 0; j < cols-1; j++)
            {
                tempImg.at<uchar>(i, j) = abs(Dx.at<uchar>(i, j)) + abs(Dy.at <uchar>(i, j));
            }
        }

        dst = tempImg;
    }
    else if(Img.channels() == 3){
        //卷积
        //注意这里的i一定要从1开始，到ows-1结束。
        for (int i = 1; i < rows-1; i++)
        {
            for (int j = 1; j < cols-1; j++)
            {
                Dx.at<cv::Vec3b>(i, j)[0] = Img.at<cv::Vec3b>(i + 1, j - 1)[0] - Img.at<cv::Vec3b>(i - 1, j - 1)[0]
                        + 2 * (Img.at<cv::Vec3b>(i + 1, j)[0] - Img.at<cv::Vec3b>(i - 1, j)[0])
                        + Img.at<cv::Vec3b>(i + 1, j + 1)[0] - Img.at<cv::Vec3b>(i - 1, j + 1)[0];
                Dx.at<cv::Vec3b>(i, j)[1] = Img.at<cv::Vec3b>(i + 1, j - 1)[1] - Img.at<cv::Vec3b>(i - 1, j - 1)[1]
                        + 2 * (Img.at<cv::Vec3b>(i + 1, j)[1] - Img.at<cv::Vec3b>(i - 1, j)[1])
                        + Img.at<cv::Vec3b>(i + 1, j + 1)[1] - Img.at<cv::Vec3b>(i - 1, j + 1)[1];
                Dx.at<cv::Vec3b>(i, j)[2] = Img.at<cv::Vec3b>(i + 1, j - 1)[2] - Img.at<cv::Vec3b>(i - 1, j - 1)[2]
                        + 2 * (Img.at<cv::Vec3b>(i + 1, j)[2] - Img.at<cv::Vec3b>(i - 1, j)[2])
                        + Img.at<cv::Vec3b>(i + 1, j + 1)[2] - Img.at<cv::Vec3b>(i - 1, j + 1)[2];

                Dy.at<cv::Vec3b>(i, j)[0] = Img.at<cv::Vec3b>(i - 1, j + 1)[0] - Img.at<cv::Vec3b>(i - 1, j - 1)[0]
                        + 2 * (Img.at<cv::Vec3b>(i, j + 1)[0] - Img.at<cv::Vec3b>(i, j - 1)[0])
                        + Img.at<cv::Vec3b>(i + 1, j + 1)[0] - Img.at<cv::Vec3b>(i + 1, j - 1)[0];
                Dy.at<cv::Vec3b>(i, j)[1] = Img.at<cv::Vec3b>(i - 1, j + 1)[1] - Img.at<cv::Vec3b>(i - 1, j - 1)[1]
                        + 2 * (Img.at<cv::Vec3b>(i, j + 1)[1] - Img.at<cv::Vec3b>(i, j - 1)[1])
                        + Img.at<cv::Vec3b>(i + 1, j + 1)[1] - Img.at<cv::Vec3b>(i + 1, j - 1)[1];
                Dy.at<cv::Vec3b>(i, j)[2] = Img.at<cv::Vec3b>(i - 1, j + 1)[2] - Img.at<cv::Vec3b>(i - 1, j - 1)[2]
                        + 2 * (Img.at<cv::Vec3b>(i, j + 1)[2] - Img.at<cv::Vec3b>(i, j - 1)[2])
                        + Img.at<cv::Vec3b>(i + 1, j + 1)[2] - Img.at<cv::Vec3b>(i + 1, j - 1)[2];
            }
        }

        for (int i = 0; i < rows-1; i++)
        {
            for (int j = 0; j < cols-1; j++)
            {
                tempImg.at<cv::Vec3b>(i, j)[0] = abs(Dx.at<cv::Vec3b>(i, j)[0]) + abs(Dy.at <cv::Vec3b>(i, j)[0]);
                tempImg.at<cv::Vec3b>(i, j)[1] = abs(Dx.at<cv::Vec3b>(i, j)[1]) + abs(Dy.at <cv::Vec3b>(i, j)[1]);
                tempImg.at<cv::Vec3b>(i, j)[2] = abs(Dx.at<cv::Vec3b>(i, j)[2]) + abs(Dy.at <cv::Vec3b>(i, j)[2]);
            }
        }

        dst = tempImg;
    }
    dst = tempImg;
}

void Browser::save_img(QImage &Img, QString name){
    QString finale_name = name;
    finale_name.append(".bmp");
    Img.save(finale_name);
}

void Browser::save_img2(QImage &Img, QString name){
    QString finale_name = name;
    finale_name.append(".jpg");
    Img.save(finale_name);
}
/*
void Browser::scale(QImage image, double cx, double cy){
    unsigned int OutWidth = (unsigned int)(iWidth_temp * cx +0.5);
    unsigned int OutHeight = (unsigned int)(iHeight_temp * cy +0.5 );
    QImage* newImage = new QImage(OutWidth, OutHeight , QImage::Format_RGB16);
    raw_temp_ptr = (int *)malloc(sizeof(int) * iWidth_temp * iHeight_temp);

    double  x = 0;
    double  y = 0;
    int r,g,b;
    for (unsigned int  j = 0; j < OutHeight- cy; j++) //    最后一行会溢出，所以去掉
    {
        y = j / cy  ;

        for(unsigned int i =0; i < OutWidth; i++)
        {
            x = i / cx ;

            int x1, x2, y1, y2;
            x1= ( int)x;
            x2 = x1 + 1;
            y1 = ( int)y;
            y2 = y1 + 1;

            QColor oldcolor1;
            QColor oldcolor2;
            QColor oldcolor3;
            QColor oldcolor4;
            double u, v;
            u = x - x1;
            v = y - y1;
            if ((x >=iWidth_temp - 1 ) && (y >=iHeight_temp - 1 ))
            {
                oldcolor1 = QColor(image.pixel(x1,y1));
                r = oldcolor1.red();
                g = oldcolor1.green();
                b = oldcolor1.blue();
            }
            else if (x >= iWidth_temp - 1)
            {
                oldcolor1 = QColor(image.pixel(x1,y1));
                oldcolor3 = QColor(image.pixel(x1,y2));
                r = oldcolor1.red() * (1 - v) + oldcolor3.red() * v;
                g = oldcolor1.green() * (1 - v) + oldcolor3.green() * v;
                b = oldcolor1.blue() * (1 - v) + oldcolor3.blue() * v;
            }
            else if (x >=iHeight_temp - 1)
            {
                oldcolor1 = QColor(image.pixel(x1,y1));
                oldcolor2 = QColor(image.pixel(x2,y1));
                r = oldcolor1.red() * (1 - u) + oldcolor2.red() * u;
                g = oldcolor1.green() * (1 - u) + oldcolor2.green() * u;
                b = oldcolor1.blue() * (1 - u) + oldcolor2.blue() * u;
            }
            else
            {
                oldcolor1 = QColor(image.pixel(x1,y1));
                oldcolor2 = QColor(image.pixel(x2,y1));
                oldcolor3 = QColor(image.pixel(x1,y2));
                oldcolor4 = QColor(image.pixel(x2,y2));
                int r1,g1,b1;
                r = oldcolor1.red() * (1 - u) + oldcolor2.red() * u;
                g = oldcolor1.green() * (1 - u) + oldcolor2.green() * u;
                b = oldcolor1.blue() * (1 - u) + oldcolor2.blue() * u;

                r1 = oldcolor3.red() * (1 - u) + oldcolor4.red() * u;
                g1 = oldcolor3.green() * (1 - u) + oldcolor4.green() * u;
                b1 = oldcolor3.blue() * (1 - u) + oldcolor4.blue() * u;

                r = r * (1 - v) + r1 * v;
                g = g * (1 - v) + g1 * v;
                b = b * (1 - v) + b1 * v;
            }

            newImage->setPixel(i, j, qRgb(r, g, b));
        }
    }
    iWidth_temp = OutWidth;
    iHeight_temp = OutHeight;
    dst_img = *newImage;
}
*/
void Browser::scale(QImage image, double cx, double cy){
    unsigned int OutWidth = (unsigned int)(iWidth_temp * cx +0.5);
    unsigned int OutHeight = (unsigned int)(iHeight_temp * cy +0.5 );
    QImage* newImage = new QImage(OutWidth, OutHeight , QImage::Format_RGB16);
    raw_temp_ptr = (int *)malloc(sizeof(int) * OutWidth * OutHeight);

    double  x = 0;
    double  y = 0;

    int r,g,b;
    float color_temp;
    for (unsigned int  j = 0; j < OutHeight- cy; j++) //    最后一行会溢出，所以去掉
    {
        y = j / cy  ;

        for(unsigned int i =0; i < OutWidth; i++)
        {
            x = i / cx ;

            int x1, x2, y1, y2;
            x1= ( int)x;
            x2 = x1 + 1;
            y1 = ( int)y;
            y2 = y1 + 1;

            float color1, color2, color3, color4;
            double u, v;
            u = x - x1;
            v = y - y1;
            if ((x >=iWidth_temp - 1 ) && (y >=iHeight_temp - 1 ))
            {
                color1 = raw_latter[y1 * iWidth_temp + x1];
                color_temp = color1;
            }
            else if (x >= iWidth_temp - 1)
            {
                color1 = raw_latter[y1 * iWidth_temp + x1];
                color3 = raw_latter[y2 * iWidth_temp + x1];
                color_temp = (1 - v)*color1 + color3 * v;
            }
            else if (x >=iHeight_temp - 1)
            {

                color1 = raw_latter[y1 * iWidth_temp + x1];
                color2 = raw_latter[y1 * iWidth_temp + x2];
                color_temp = (1 - u) * color1 + u * color2;
            }
            else
            {
                color1 = raw_latter[y1 * iWidth_temp + x1];
                color2 = raw_latter[y1 * iWidth_temp + x2];
                color3 = raw_latter[y2 * iWidth_temp + x1];
                color4 = raw_latter[y2 * iWidth_temp + x2];
                float color_temp_1;
                color_temp = color1 * (1 - u) + color2 * u;
                color_temp_1 = color3 * (1 - u) + color4 * u;
                color_temp = (1 - v) * color_temp + v * color_temp_1;
            }

            newImage->setPixel(i, j, qRgb(color_temp, color_temp, color_temp));
            raw_temp_ptr[j * OutWidth + i] = color_temp;
        }
    }
    iWidth_temp = OutWidth;
    iHeight_temp = OutHeight;
    raw_latter = (int *)malloc(sizeof(int) * iWidth_temp * iHeight_temp);
    for(int i = 0; i < iWidth_temp * iHeight_temp; i++)
        raw_latter[i] = raw_temp_ptr[i];
    dst_img = *newImage;
    raw_temp_ptr = nullptr;
}

void Browser::inverse(QImage image){
    QImage newImage(iWidth_temp, iHeight_temp , QImage::Format_RGB16);
    int r,g,b;
    for (unsigned int  j = 0; j < (unsigned int)iHeight_temp; j++) //    最后一行会溢出，所以去掉
    {
        for(unsigned int i =0; i < (unsigned int)iWidth_temp; i++)
        {
            QColor oldcolor1 = QColor(image.pixel(i,j));
            r = oldcolor1.red();
            g = oldcolor1.green();
            b = oldcolor1.blue();

            r = 4095 - r;
            g = 4095 - g;
            b = 4095 - b;
            newImage.setPixel(i, j, qRgb(r, g, b));
        }
    }
    dst_img = newImage;
}

void Browser::inverse(){
    QImage newImage(iWidth_temp, iHeight_temp , QImage::Format_RGB16);

    for (unsigned int  j = 0; j < (unsigned int)iHeight_temp; j++) //    最后一行会溢出，所以去掉
    {
        for(unsigned int i =0; i < (unsigned int)iWidth_temp; i++)
        {
            float temp = raw_latter[j * iWidth_temp + i];
            temp = 4095 - temp;

            newImage.setPixel(i, j, qRgb(temp, temp, temp));
            raw_latter[j * iWidth_temp + i] = temp;
        }
    }
    dst_img = newImage;
}

void Browser::reverse(QImage image){

    QImage newImage(iWidth_temp, iHeight_temp , QImage::Format_RGB16);
    for (unsigned int  j = 0; j < (unsigned int)iHeight_temp; j++) //    最后一行会溢出，所以去掉
    {
        for(unsigned int i =0; i < (unsigned int)iWidth_temp; i++)
        {
            float temp = raw_latter[j * iWidth_temp + i];
            QColor oldcolor1 = QColor(image.pixel(iWidth_temp - i - 1,j));

            newImage.setPixel(i, j, qRgb(oldcolor1.red(), oldcolor1.green(), oldcolor1.blue()));
        }
    }
    dst_img = newImage;
}

void Browser::reverse(){
    raw_temp_ptr = (int *)malloc(sizeof(int) * iWidth_temp * iHeight_temp);
    QImage newImage(iWidth_temp, iHeight_temp , QImage::Format_RGB16);
    for (unsigned int  j = 0; j < (unsigned int)iHeight_temp; j++) //    最后一行会溢出，所以去掉
    {
        for(unsigned int i =0; i < (unsigned int)iWidth_temp; i++)
        {
            float temp = raw_latter[j*iWidth_temp + iWidth_temp - i - 1];

            newImage.setPixel(i, j, qRgb(temp, temp, temp));
            raw_temp_ptr[j*iWidth_temp + i] = temp;
        }
    }

    for(int i = 0; i < iWidth_temp * iHeight_temp; i++)
        raw_latter[i] = raw_temp_ptr[i];
    raw_temp_ptr = nullptr;
    dst_img = newImage;
}

void Browser::rotate(double Angle){
    double angle = Angle*CV_PI / 180.0;
    //构造输出图像
    int OutHeight = round(fabs(iHeight_temp * cos(angle)) + fabs(iWidth_temp * sin(angle)));//图像高度
    int OutWidth = round(fabs(iWidth_temp * cos(angle)) + fabs(iHeight_temp * sin(angle)));//图像宽度
    QImage* newImage = new QImage(OutWidth, OutHeight , QImage::Format_RGB16);
    raw_temp_ptr = (int *)malloc(sizeof(int) * OutWidth * OutHeight);
    for(int i = 0; i < OutWidth * OutHeight; i++)
        raw_temp_ptr[i] = 0;

    cv::Mat tran1 = (cv::Mat_<double>(3,3) << 1.0,0.0,0.0 , 0.0,-1.0,0.0, -0.5*iWidth_temp , 0.5*iHeight_temp , 1.0); // 将原图像坐标映射到数学笛卡尔坐标
    cv::Mat tran2 = (cv::Mat_<double>(3,3) << cos(angle),-sin(angle),0.0 , sin(angle), cos(angle),0.0, 0.0,0.0,1.0); //数学笛卡尔坐标下顺时针旋转的变换矩阵
    double t3[3][3] = { { 1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }, { 0.5*OutWidth, 0.5*OutHeight ,1.0} }; // 将数学笛卡尔坐标映射到旋转后的图像坐标
    cv::Mat tran3 = cv::Mat(3.0,3.0,CV_64FC1,t3);
    cv::Mat T = tran1*tran2*tran3;
    cv::Mat T_inv = T.inv(); // 求逆矩阵

    for (double i = 0.0; i < OutHeight; i++){
        for (double j = 0.0; j < OutWidth; j++){
            cv::Mat dst_coordinate = (cv::Mat_<double>(1, 3) << j, i, 1.0);
            cv::Mat src_coordinate = dst_coordinate * T_inv;
            double v = src_coordinate.at<double>(0, 0);
            double w = src_coordinate.at<double>(0, 1);

            //双线性插值
            if (int(Angle) % 90 == 0) {
                if (v < 0)
                    v = 0;
                if (v > iWidth_temp - 1)
                    v = iWidth_temp - 1;
                if (w < 0)
                    w = 0;
                if (w > iHeight_temp - 1)
                    w = iHeight_temp - 1;
            }

            if (v >= 0 && w >= 0 && v <= iWidth_temp - 1 && w <= iHeight_temp - 1){
                int top = floor(w), bottom = ceil(w), left = floor(v), right = ceil(v);
                double pw = w - top ;
                double pv = v - left;
                int color_temp = (1 - pw) * (1 - pv) * raw_latter[top * iWidth_temp + left] + (1 - pw) * pv * raw_latter[top * iWidth_temp + right]
                        + pw * (1 - pv) * raw_latter[bottom * iWidth_temp + left] + pw * pv * raw_latter[bottom * iWidth_temp + right];
                newImage->setPixel(j, i, qRgb(color_temp, color_temp, color_temp));
                raw_temp_ptr[(int)(i * OutWidth + j)] = color_temp;
            }
        }
    }
    iWidth_temp = OutWidth;
    iHeight_temp = OutHeight;
    raw_latter = (int *)malloc(sizeof(int) * iWidth_temp * iHeight_temp);
    for(int i = 0; i < iWidth_temp * iHeight_temp; i++)
        raw_latter[i] = raw_temp_ptr[i];
    dst_img = *newImage;
    raw_temp_ptr = nullptr;
}

void Browser::clear(){
    ui->in_name_bmp->clear();
    ui->in_name_jpg->clear();
    ui->in_scale_x->clear();
    ui->in_scale_y->clear();
    ui->in_wl->clear();
    ui->in_ww->clear();
    ui->in_rotate->clear();

    ui->radio_intensify->setChecked(false);
    ui->radio_inverse->setChecked(false);
    ui->radio_reverse->setChecked(false);
    ui->radio_save->setChecked(false);
    ui->radio_scale->setChecked(false);
    ui->radio_window->setChecked(false);
    ui->radio_rotate->setChecked(false);
}

void Browser::on_actionraw_triggered()
{
    QString str = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg;*.raw");
    this->filepath = str;

    mytype = _RAW;
    this->getbit();

    back_find.clear();
    clear();
}

void Browser::on_actionother_triggered()
{
    QString str = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg;*.raw");
    this->filepath = str;

    mytype = _OTHER;
    src = cv::imread(filepath.toStdString());
    dst = src;
    this->show_image(src);

    back_find_mat.clear();
    clear();
}

void Browser::on_btn_ok_clicked()
{
    //ok
    QImage temp_img = dst_img;

    if(ui->radio_intensify->isChecked()){
        //intensify
        this->intensify();
        //this->sharp(temp_img);
        temp_img = dst_img;
        back_find.push_back(std::pair<funcType, std::vector<float>>(FUNC_INTENSIFY,
                                                                    std::vector<float>()));
    }

    if(ui->radio_window->isChecked()){
        if(!ui->in_wl->text().isEmpty() && !ui->in_ww->text().isEmpty()){
            if(!isWindowed)
            {
                isWindowed = true;
                iWidth_window = iWidth_temp;
                iHeight_window = iHeight_temp;
                raw_window = (int *)malloc(sizeof(int) * iWidth_temp * iHeight_temp);
                for(int i = 0; i < iWidth_temp * iHeight_temp; i++)
                    raw_window[i] = raw_latter[i];
            }else{
                iHeight_temp = iHeight_window;
                iWidth_temp = iWidth_window;

                raw_latter = (int *)malloc(sizeof(int) * iWidth_temp * iHeight_temp);
            }
            this->my_window(raw_latter, ui->in_ww->text().toInt(), ui->in_wl->text().toInt());
            //this->intensify(temp_img, ui->w_wid->toPlainText().toInt(), ui->w_loc->toPlainText().toInt());
            temp_img = dst_img;
            std::vector<float> temp_par;  temp_par.push_back(ui->in_ww->text().toInt());  temp_par.push_back(ui->in_wl->text().toInt());
            back_find.push_back(std::pair<funcType, std::vector<float>>(FUNC_WINDOW, temp_par));
        }else{
            QMessageBox::warning(this, tr("WARNING"),  tr("         请填写必要的参数！         "),  QMessageBox::Cancel,  QMessageBox::Cancel);
        }
    }

    if(ui->radio_scale->isChecked()){
        if(!ui->in_scale_x->text().isEmpty() && ! ui->in_scale_y->text().isEmpty()){
            this->scale(temp_img, ui->in_scale_x->text().toDouble(), ui->in_scale_y->text().toDouble());
            temp_img = dst_img;
            std::vector<float> temp_par;  temp_par.push_back(ui->in_scale_x->text().toDouble());  temp_par.push_back(ui->in_scale_y->text().toDouble());
            back_find.push_back(std::pair<funcType, std::vector<float>>(FUNC_SCALE, temp_par));
        }else{
            QMessageBox::warning(this, tr("WARNING"),  tr("         请填写必要的参数！         "),  QMessageBox::Cancel,  QMessageBox::Cancel);
        }
    }

    if(ui->radio_inverse->isChecked()){
        this->inverse();
        temp_img = dst_img;
        back_find.push_back(std::pair<funcType, std::vector<float>>(FUNC_INVERSE,
                                                                    std::vector<float>()));
    }

    if(ui->radio_reverse->isChecked()){
        this->reverse();
        temp_img = dst_img;
        back_find.push_back(std::pair<funcType, std::vector<float>>(FUNC_REVERSE,
                                                                    std::vector<float>()));
    }

    if(ui->radio_rotate->isChecked()){
        if(!ui->in_rotate->text().isEmpty()){
            this->rotate(ui->in_rotate->text().toDouble());
            temp_img = dst_img;
            std::vector<float> temp_par;  temp_par.push_back(ui->in_rotate->text().toDouble());
            back_find.push_back(std::pair<funcType, std::vector<float>>(FUNC_SCALE, temp_par));
        }else{
            QMessageBox::warning(this, tr("WARNING"),  tr("         请填写必要的参数！         "),  QMessageBox::Cancel,  QMessageBox::Cancel);
        }
    }

    show_image(dst_img);
    clear();

    /*if(back_find.size() > 7){
        for(int i = 0; i < 3; i++){
            back_find.erase(back_find.begin());
        }
    }*/
}

void Browser::on_btn_recover_clicked()
{
    ui->in_name_bmp->clear();
    ui->in_name_jpg->clear();
    ui->in_scale_x->clear();
    ui->in_scale_y->clear();
    ui->in_wl->clear();
    ui->in_ww->clear();

    ui->radio_intensify->setChecked(false);
    ui->radio_inverse->setChecked(false);
    ui->radio_reverse->setChecked(false);
    ui->radio_save->setChecked(false);
    ui->radio_scale->setChecked(false);
    ui->radio_window->setChecked(false);

    if(mytype == _RAW){
        show_image(src_img);
        raw_latter = nullptr;
        raw_latter = (int *)malloc(sizeof(int) * iWidth * iHeight);
        raw_window = nullptr;
        raw_window = (int *)malloc(sizeof(int) * iWidth * iHeight);
        for(int i = 0; i < iWidth * iHeight; i++){
            raw_latter[i] = raw[i];
            raw_window[i] = raw[i];
        }
        iWidth_temp = iWidth;
        iHeight_temp = iHeight;
        iWidth_window = iWidth;
        iHeight_window = iHeight;
    }else{
        show_image(src);
    }
    back_find.clear();
    back_find_mat.clear();

    isWindowed = false;
}

void Browser::on_btn_back_clicked()
{
    if(back_find.size() != 0)
        back_find.erase(back_find.end());
    dst_img = src_img;
    raw_latter = nullptr;
    raw_latter = (int *)malloc(sizeof(int) * iWidth * iHeight);
    raw_window = nullptr;
    raw_window = (int *)malloc(sizeof(int) * iWidth * iHeight);
    for(int i = 0; i < iWidth * iHeight; i++){
        raw_latter[i] = raw[i];
        raw_window[i] = raw[i];
    }
    iWidth_temp = iWidth;
    iHeight_temp = iHeight;
    iWidth_window = iWidth;
    iHeight_window = iHeight;
    isWindowed = false;

    for(int i = 0; i < (int)back_find.size(); i++){
        std::vector<float> temp_par;
        temp_par = back_find[i].second;
        switch (back_find[i].first) {
        case FUNC_WINDOW:
        {
            if(!isWindowed)
            {
                isWindowed = true;
                iWidth_window = iWidth_temp;
                iHeight_window = iHeight_temp;
                raw_window = (int *)malloc(sizeof(int) * iWidth_temp * iHeight_temp);
                for(int i = 0; i < iWidth_temp * iHeight_temp; i++)
                    raw_window[i] = raw_latter[i];
            }else{
                iHeight_temp = iHeight_window;
                iWidth_temp = iWidth_window;

                raw_latter = (int *)malloc(sizeof(int) * iWidth_temp * iHeight_temp);
            }
            this->my_window(raw_latter, (int)temp_par[0], (int)temp_par[1]);
            break;
        }
        case FUNC_INVERSE:
        {
            this->inverse();
            break;
        }
        case FUNC_SCALE:
        {
            this->scale(dst_img, temp_par[0], temp_par[1]);
            break;
        }
        case FUNC_REVERSE:
        {
            this->reverse();
            break;
        }
        case FUNC_INTENSIFY:
        {
            this->intensify();
            break;
        }
        case FUNC_ROTATE:
        {
            this->rotate(temp_par[0]);
            break;
        }
        default:
            break;
        }
    }
    show_image(dst_img);
}

void Browser::on_btn_save_clicked()
{
    //save
    if(!ui->in_name_bmp->text().isEmpty()){
        this->save_img(dst_img, QString::fromStdString(ui->in_name_bmp->text().toStdString()));
    }
    if(!ui->in_name_jpg->text().isEmpty()){
        this->save_img2(dst_img, QString::fromStdString(ui->in_name_jpg->text().toStdString()));
    }
    QMessageBox::about(this, tr("提示"),tr("图片已保存"));
    ui->in_name_bmp->clear();
    ui->in_name_jpg->clear();
    ui->radio_save->setChecked(false);
}
