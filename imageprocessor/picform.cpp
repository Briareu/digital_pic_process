#include "picform.h"
#include "ui_picform.h"
#include "picType.h"
#include <QTextCodec>
#include <malloc.h>
#include <windows.h>
#include <QMessageBox>
#include "mywidget.h"
#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>

using namespace std;
using namespace cv;

picForm::picForm(QString path, int type,  QWidget *parent) :
    QWidget(parent),
    ui(new Ui::picForm)
{
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
        break;
    }
    case 3:
    {
        m_type = _EQUAL;
        this->_equalize();

       // ui->setupUi(this);
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

picForm::~picForm()
{
    delete ui;
}

void picForm::on_pushButton_clicked()
{
    myWidget *w = new myWidget();
    this->close();
    w->show();
}
