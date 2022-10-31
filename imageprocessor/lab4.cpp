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

void lab4::my_window(QImage &src, int ww, int wl){
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

void lab4::intensify(QImage &Img){
    QImage tempImg = QImage(iWidth, iHeight, QImage::Format_RGB16);

    int kernel[3][3] = {
        {-1, -1, -1},
        {-1, 8, -1},
        {-1, -1, -1}
    };

    int sizeKernel = 3;
    int sumKernel = 3;
    QColor color;

    for(int x = sizeKernel/2; x < Img.width() - sizeKernel/2; x++){
        for(int y = sizeKernel/2; y < Img.height() - sizeKernel; y++){
            int r = 0, g = 0, b = 0;
            for(int i = -sizeKernel/2; i <= sizeKernel/2; i++){
                for(int j = -sizeKernel/2; j <= sizeKernel/2; j++){
                    color = QColor(Img.pixel(x + i, y + i));
                    r += color.red()*kernel[sizeKernel/2 + i][sizeKernel/2 + j];
                    g += color.green()*kernel[sizeKernel/2 + i][sizeKernel/2 + j];
                    b += color.blue()*kernel[sizeKernel/2 + i][sizeKernel/2 + j];
                }
            }

            r = r/sumKernel + color.red();
            g = g/sumKernel + color.green();
            b = b/sumKernel + color.blue();
            r = qBound(0,r,255) ;
            g = qBound(0,g,255) ;
            b = qBound(0,b,255) ;

            tempImg.setPixel(x, y, qRgb(r, g, b));
        }
    }
    dst_img = tempImg;
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
        QImage temp_img = src_img;

        if(ui->img_window->isChecked()){
            if(!ui->w_loc->document()->isEmpty() && !ui->w_wid->document()->isEmpty()){
                this->my_window(temp_img, ui->w_wid->toPlainText().toInt(), ui->w_loc->toPlainText().toInt());
                temp_img = dst_img;
            }
        }

        if(ui->img_intensify->isChecked()){
            //intensify
            this->intensify(temp_img);
            //this->sharp(temp_img);
            temp_img = dst_img;
        }

        show_image(dst_img);

        break;
    }
    case _OTHER:
    {
        cv::Mat temp = src;

        if(ui->img_window->isChecked()){
            if(!ui->w_loc->document()->isEmpty() && !ui->w_wid->document()->isEmpty()){
                this->my_window(temp, ui->w_wid->toPlainText().toInt(), ui->w_loc->toPlainText().toInt());
                temp = dst;
            }
        }

        if(ui->img_intensify->isChecked()){
            //intensify
            //this->intensify(temp);
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
