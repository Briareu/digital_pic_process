#include "lab4.h"
#include "ui_lab4.h"


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


int *raw_p;

lab4::lab4(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::lab4)
{
    ui->setupUi(this);
}

void lab4::show_image(const cv::Mat &image){
    QImage Img = cvMat2QImage(image);

    QGraphicsScene *scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(Img));
    ui->pic->setScene(scene);
    ui->pic->show();
}

void lab4::show_image(QImage Img){
    QGraphicsScene *scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(Img));
    ui->pic->setScene(scene);
    ui->pic->show();
}

void lab4::getbit(){
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

    fread(data, 1, 4, pf);
    t1 = data[0] - '0' + 48;
    t2 = data[1] - '0' + 48;
    t2 *= 256;
    iHeight = t1 + t2;

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
        //std::cout<<t1 + t2<<std::endl;
    }

    src_img = this->Matrix2QImage(raw, iWidth, iHeight);
    dst_img = src_img;
    this->show_image(src_img);

    return;
}

QImage lab4::cvMat2QImage(const cv::Mat &mat){
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

QImage lab4::Matrix2QImage(int *data, int width, int height){
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

void lab4::my_window(cv::Mat &src, int ww, int wl){
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

int lab4::convert(int ww, int wl, int tar){
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

void lab4::my_window(QImage &src_img, int ww, int wl){
    QImage tempImg = QImage(src_img.width(), src_img.height(), QImage::Format_RGB16);

    int num = 0;
    for (int i = 0; i < src_img.height(); ++i)
    {
        for (int j = 0; j < src_img.width(); ++j)
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

void lab4::my_window(int *raw_p, int ww, int wl){
    QImage tempImg = QImage(src_img.width(), src_img.height(), QImage::Format_RGB16);

    int num = 0;
    for (int i = 0; i < src_img.height(); ++i)
    {
        for (int j = 0; j < src_img.width(); ++j)
        {
            int temp = raw_p[num];
            //tempImg.setPixel(j, i, temp);
            temp = convert(ww, wl, temp);
            tempImg.setPixelColor(j, i, qRgb(temp, temp, temp));
            num++;
            //tempImg.setPixel(i, j, qRgba64(4000, 4000, 4000, 4000));
        }
    }

    dst_img = tempImg;
}

int lab4::convert2(int ww, int wl, int tar){
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
/*
void lab4::intensify(QImage &Img){
    QImage tempImg = QImage(iWidth, iHeight, QImage::Format_RGB16);

    raw_p = (int *)malloc(sizeof(int) * iWidth * iHeight);

    int kernel[3][3] = {
        {-1, -1, -1},
        {-1, 8, -1},
        {-1, -1, -1}
    };

    int sizeKernel = 3;
    int sumKernel = 3;
    QColor color;
    int num = 0;
    int temp = 0;

    for(int x = sizeKernel/2; x < Img.width() - sizeKernel/2; x++){
        for(int y = sizeKernel/2; y < Img.height() - sizeKernel; y++){
            int r = 0, g = 0, b = 0;
            int data = 0;
            for(int i = -sizeKernel/2; i <= sizeKernel/2; i++){
                for(int j = -sizeKernel/2; j <= sizeKernel/2; j++){
                    color = QColor(Img.pixel(x + i, y + j));
                    r += color.red()*kernel[sizeKernel/2 + i][sizeKernel/2 + j];
                    g += color.green()*kernel[sizeKernel/2 + i][sizeKernel/2 + j];
                    b += color.blue()*kernel[sizeKernel/2 + i][sizeKernel/2 + j];

                    temp = raw[(x + i)*iWidth + y + j];
                    data += temp*kernel[sizeKernel/2 + i][sizeKernel/2 + j];
                }
            }

            r = r/sumKernel + color.red();
            g = g/sumKernel + color.green();
            b = b/sumKernel + color.blue();
            r = qBound(0,r,255) ;
            g = qBound(0,g,255) ;
            b = qBound(0,b,255) ;

            data = data/sumKernel + temp;

            tempImg.setPixel(x, y, qRgb(r, g, b));
            raw_p[x * iWidth + y] = data;
            if(num <= 100){
                std::cout<<raw[x*iWidth + y]<<":      "<<data<<std::endl;
                num++;
            }
        }
    }
    dst_img = tempImg;
}*/

void lab4::intensify(QImage &Img){
    QImage tempImg = QImage(iWidth, iHeight, QImage::Format_RGB16);

    raw_p = nullptr;
    raw_p = (int *)malloc(sizeof(int) * iWidth * iHeight);
    //temp = (int *)malloc(sizeof(int) * iWidth * iHeight);

    for (int i = 1; i < iHeight - 1; i++)
    {
        for (int j = 1; j < iWidth - 1; j++)
        {
            unsigned char imageKernel[9] = { 0 };
            imageKernel[0] = raw[(i - 1)*iWidth + j - 1];
            imageKernel[1] = raw[(i - 1)*iWidth + j];
            imageKernel[2] = raw[(i - 1)*iWidth + j + 1];
            imageKernel[3] = raw[(i)*iWidth + j - 1];
            imageKernel[4] = raw[(i)*iWidth + j];
            imageKernel[5] = raw[(i)*iWidth + j + 1];
            imageKernel[6] = raw[(i + 1)*iWidth + j - 1];
            imageKernel[7] = raw[(i + 1)*iWidth + j];
            imageKernel[8] = raw[(i + 1)*iWidth + j + 1];

            //化简后结果   这里使用了 1,1.414,1 的模板（各向同性Sobel算子），与 1,2,1的模板区别不是很大
            float GX = imageKernel[2] - imageKernel[0] + (imageKernel[5] - imageKernel[3]) * 1.414 + imageKernel[8] - imageKernel[6];
            float GY = imageKernel[0] + imageKernel[2] + (imageKernel[1] - imageKernel[7]) * 1.414 - imageKernel[6] - imageKernel[8];

            //int val = LimitValue(sqrt(GX*GX + GY*GY) + 0.5);

            raw_p[i*iWidth + j] = sqrt(GX*GX + GY*GY) + raw[i*iWidth + j];
            //temp[i*iWidth + j] = sqrt(GX*GX + GY*GY) + imageKernel[4];
        }
    }

    tempImg = this->Matrix2QImage(raw_p, iWidth, iHeight);
    dst_img = tempImg;
}

void lab4::intensify(QImage &Img, int ww, int wl){
    QImage tempImg = QImage(iWidth, iHeight, QImage::Format_RGB16);

    raw_p = nullptr;
    raw_p = (int *)malloc(sizeof(int) * iWidth * iHeight);
    //temp = (int *)malloc(sizeof(int) * iWidth * iHeight);

    int num1 = 0;
    for (int i = 1; i < iHeight - 1; i++)
    {
        for (int j = 1; j < iWidth - 1; j++)
        {
            unsigned char imageKernel[9] = { 0 };
            imageKernel[0] = raw[(i - 1)*iWidth + j - 1];
            imageKernel[1] = raw[(i - 1)*iWidth + j];
            imageKernel[2] = raw[(i - 1)*iWidth + j + 1];
            imageKernel[3] = raw[(i)*iWidth + j - 1];
            imageKernel[4] = raw[(i)*iWidth + j];
            imageKernel[5] = raw[(i)*iWidth + j + 1];
            imageKernel[6] = raw[(i + 1)*iWidth + j - 1];
            imageKernel[7] = raw[(i + 1)*iWidth + j];
            imageKernel[8] = raw[(i + 1)*iWidth + j + 1];

            //化简后结果   这里使用了 1,1.414,1 的模板（各向同性Sobel算子），与 1,2,1的模板区别不是很大
            float GX = imageKernel[2] - imageKernel[0] + (imageKernel[5] - imageKernel[3]) * 1.414 + imageKernel[8] - imageKernel[6];
            float GY = imageKernel[0] + imageKernel[2] + (imageKernel[1] - imageKernel[7]) * 1.414 - imageKernel[6] - imageKernel[8];

            //int val = LimitValue(sqrt(GX*GX + GY*GY) + 0.5);

            raw_p[i*iWidth + j] = sqrt(GX*GX + GY*GY) + imageKernel[4];
            if(num1 < 100){
                std::cout<<raw_p[i*iWidth + j]<<std::endl;
                num1++;
            }
            //temp[i*iWidth + j] = sqrt(GX*GX + GY*GY) + imageKernel[4];
        }
    }

    int num = 0;
    for (int i = 0; i < src_img.height(); ++i)
    {
        for (int j = 0; j < src_img.width(); ++j)
        {
            int temp = raw_p[num];
            //tempImg.setPixel(j, i, temp);
            temp = convert(ww, wl, temp);
            raw_p[num] = temp;
            num++;
            //tempImg.setPixel(i, j, qRgba64(4000, 4000, 4000, 4000));
        }
    }

    tempImg = this->Matrix2QImage(raw_p, iWidth, iHeight);
    dst_img = tempImg;
}

void lab4::intensify(cv::Mat &Img){
    cv::Mat tempImg = Img;
    /*
    for (int i = 1; i < Img.rows - 1; i++)
    {
        for (int j = 1; j < Img.cols - 1; j++)
        {
            if(Img.channels() == 3){
                unsigned char imageKernel[9] = { 0 };
                unsigned char imageKernel_g[9] = { 0 };
                unsigned char imageKernel_b[9] = { 0 };
                imageKernel[0] = Img.at<cv::Vec3b>(i - 1, j - 1)[0];
                imageKernel_g[0] = Img.at<cv::Vec3b>(i - 1, j - 1)[1];
                imageKernel_b[0] = Img.at<cv::Vec3b>(i - 1, j - 1)[2];

                imageKernel[1] = Img.at<cv::Vec3b>(i - 1, j)[0];
                imageKernel_g[1] = Img.at<cv::Vec3b>(i - 1, j)[1];
                imageKernel_b[1] = Img.at<cv::Vec3b>(i - 1, j)[2];

                imageKernel[2] = Img.at<cv::Vec3b>(i - 1, j + 1)[0];
                imageKernel_g[2] = Img.at<cv::Vec3b>(i - 1, j + 1)[1];
                imageKernel_b[2] = Img.at<cv::Vec3b>(i - 1, j + 1)[2];

                imageKernel[3] = Img.at<cv::Vec3b>(i, j - 1)[0];
                imageKernel_g[3] = Img.at<cv::Vec3b>(i, j - 1)[1];
                imageKernel_b[3] = Img.at<cv::Vec3b>(i, j - 1)[2];

                imageKernel[4] = Img.at<cv::Vec3b>(i, j)[0];
                imageKernel_g[4] = Img.at<cv::Vec3b>(i, j)[1];
                imageKernel_b[4] = Img.at<cv::Vec3b>(i, j)[2];

                imageKernel[5] = Img.at<cv::Vec3b>(i, j + 1)[0];
                imageKernel_g[5] = Img.at<cv::Vec3b>(i, j + 1)[1];
                imageKernel_b[5] = Img.at<cv::Vec3b>(i, j + 1)[2];

                imageKernel[6] = Img.at<cv::Vec3b>(i + 1, j - 1)[0];
                imageKernel_g[6] = Img.at<cv::Vec3b>(i + 1, j - 1)[1];
                imageKernel_b[6] = Img.at<cv::Vec3b>(i + 1, j - 1)[2];

                imageKernel[7] = Img.at<cv::Vec3b>(i + 1, j)[0];
                imageKernel_g[7] = Img.at<cv::Vec3b>(i + 1, j)[1];
                imageKernel_b[7] = Img.at<cv::Vec3b>(i + 1, j)[2];

                imageKernel[8] = Img.at<cv::Vec3b>(i + 1, j + 1)[0];
                imageKernel_g[8] = Img.at<cv::Vec3b>(i + 1, j + 1)[1];
                imageKernel_b[8] = Img.at<cv::Vec3b>(i + 1, j + 1)[2];


                //化简后结果   这里使用了 1,1.414,1 的模板（各向同性Sobel算子），与 1,2,1的模板区别不是很大
                float GX = imageKernel[2] - imageKernel[0] + (imageKernel[5] - imageKernel[3]) * 1.414 + imageKernel[8] - imageKernel[6];
                float GY = imageKernel[0] + imageKernel[2] + (imageKernel[1] - imageKernel[7]) * 1.414 - imageKernel[6] - imageKernel[8];

                float GX_g = imageKernel_g[2] - imageKernel_g[0] + (imageKernel_g[5] - imageKernel_g[3]) * 1.414 + imageKernel_g[8] - imageKernel_g[6];
                float GY_g = imageKernel_g[0] + imageKernel_g[2] + (imageKernel_g[1] - imageKernel_g[7]) * 1.414 - imageKernel_g[6] - imageKernel_g[8];

                float GX_b = imageKernel_b[2] - imageKernel_b[0] + (imageKernel_b[5] - imageKernel_b[3]) * 1.414 + imageKernel_b[8] - imageKernel_b[6];
                float GY_b = imageKernel_b[0] + imageKernel_b[2] + (imageKernel_b[1] - imageKernel_b[7]) * 1.414 - imageKernel_b[6] - imageKernel_b[8];

                //int val = LimitValue(sqrt(GX*GX + GY*GY) + 0.5);

                int val_r = sqrt(GX*GX + GY*GY);
                int val_g = sqrt(GX_g*GX_g + GY_g*GY_g);
                int val_b = sqrt(GX_b*GX_b + GY_b*GY_b);

                tempImg.at<cv::Vec3b>(i, j)[0] = val_r;
                tempImg.at<cv::Vec3b>(i, j)[1] = val_g;
                tempImg.at<cv::Vec3b>(i, j)[2] = val_b;
            }
            else if(Img.channels() == 1){
                unsigned char imageKernel[9] = { 0 };
                imageKernel[0] = Img.at<uchar>(i - 1, j - 1);
                imageKernel[1] = Img.at<uchar>(i - 1, j);
                imageKernel[2] = Img.at<uchar>(i - 1, j + 1);
                imageKernel[3] = Img.at<uchar>(i, j - 1);
                imageKernel[4] = Img.at<uchar>(i, j);
                imageKernel[5] = Img.at<uchar>(i, j + 1);
                imageKernel[6] = Img.at<uchar>(i + 1, j - 1);
                imageKernel[7] = Img.at<uchar>(i + 1, j);
                imageKernel[8] = Img.at<uchar>(i + 1, j + 1);


                //化简后结果   这里使用了 1,1.414,1 的模板（各向同性Sobel算子），与 1,2,1的模板区别不是很大
                float GX = imageKernel[2] - imageKernel[0] + (imageKernel[5] - imageKernel[3]) * 1.414 + imageKernel[8] - imageKernel[6];
                float GY = imageKernel[0] + imageKernel[2] + (imageKernel[1] - imageKernel[7]) * 1.414 - imageKernel[6] - imageKernel[8];

                int val_r = sqrt(GX*GX + GY*GY);

                tempImg.at<uchar>(i, j) = val_r;
            }
        }
    }
*/

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

void lab4::sharp(QImage &Img){
    QImage tempImg = QImage(iWidth, iHeight, QImage::Format_RGB16);

    int* raw2;
    raw2 = (int *)malloc(sizeof(int) * iWidth * iHeight);
    int templates[25] = {
        -1, -4, -7, -4, -1,
        -4, -16, -26, -16, -4,
        -7, -26, 505, -26, -7,
        -4, -16, -26, -16, -4,
        -1, -4, -7, -4, -1
    };

    for(int j = 2; j < iHeight -2; j++){
        for(int i = 2; i < iWidth - 2; i++){
            int sum = 0;
            int index = 0;
            for(int m = j - 2; m < j + 3; m++){
                for(int n = i - 2; n < i + 3; n++){
                    sum += raw[m*iWidth + n]*templates[index++];
                }
            }
            sum /= 273;
            if(sum > 255)
                sum = 255;
            if(sum < 0)
                sum = 0;
            raw2[j*iWidth + i] = sum;
        }
    }

    dst_img = this->Matrix2QImage(raw2, iWidth, iHeight);
}

void lab4::save_img(QImage &Img, QString name){
    QString finale_name = name;
    finale_name.append(".bmp");
    Img.save(finale_name);
}

lab4::~lab4()
{
    delete ui;
}

void lab4::on_pushButton_clicked()
{
    //ok
    switch (mytype) {
    case _RAW:
    {
        QImage temp_img = dst_img;

        if(ui->img_intensify->isChecked()){
            //intensify
            this->intensify(temp_img);
            //this->sharp(temp_img);
            temp_img = dst_img;
        }

        if(ui->img_window->isChecked()){
            if(!ui->w_loc->document()->isEmpty() && !ui->w_wid->document()->isEmpty()){
                if(dst_img == src_img){
                    this->my_window(temp_img, ui->w_wid->toPlainText().toInt(), ui->w_loc->toPlainText().toInt());
                    temp_img = dst_img;
                }else{
                    this->my_window(raw_p, ui->w_wid->toPlainText().toInt(), ui->w_loc->toPlainText().toInt());
                    //this->intensify(temp_img, ui->w_wid->toPlainText().toInt(), ui->w_loc->toPlainText().toInt());
                    temp_img = dst_img;
                }
            }
        }

        show_image(dst_img);

        break;
    }
    case _OTHER:
    {
        cv::Mat temp = dst;

        if(ui->img_window->isChecked()){
            if(!ui->w_loc->document()->isEmpty() && !ui->w_wid->document()->isEmpty()){
                this->my_window(temp, ui->w_wid->toPlainText().toInt(), ui->w_loc->toPlainText().toInt());
                temp = dst;
            }
        }

        if(ui->img_intensify->isChecked()){
            //intensify
            this->intensify(temp);
            temp = dst;
        }

        show_image(dst);

        break;
    }
    default:
        break;
    }
}

void lab4::on_pushButton_2_clicked()
{
    ui->img_intensify->setChecked(false);
    ui->img_window->setChecked(false);

    ui->w_loc->clear();
    ui->w_wid->clear();

    dst_img = src_img;
    dst = src;
}

void lab4::on_actionraw_triggered()
{
    QString str = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg;*.raw");
    this->filepath = str;

    mytype = _RAW;
    this->getbit();
}

void lab4::on_actionother_triggered()
{
    QString str = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择图片"), ".", "*.bmp;*.png;*.jpg;*.raw");
    this->filepath = str;

    mytype = _OTHER;
    src = cv::imread(filepath.toStdString());
    dst = src;
    this->show_image(src);
}

void lab4::on_pushButton_3_clicked()
{
    //save
    this->save_img(dst_img, QString::fromStdString(ui->save_name->toPlainText().toStdString()));
    QMessageBox::about(this, tr("提示"),tr("图片已保存"));
    ui->save_name->clear();
    ui->img_save->setChecked(false);
}
