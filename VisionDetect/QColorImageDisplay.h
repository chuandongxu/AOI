#pragma once

#include <QWidget>
#include "ui_QColorImageDisplay.h"

#include "opencv/cv.h"
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

class QColorImageDisplay : public QWidget
{
    Q_OBJECT

public:
    QColorImageDisplay(QWidget *parent = Q_NULLPTR);
    ~QColorImageDisplay();

public:
    void getSize(int& nSizeX, int& nSizeY);
    void setImage(cv::Mat& matImage);

private:
    void A_Transform(Mat& src, Mat& dst, int dx, int dy);
    void displayImage(cv::Mat& image);

private:
    int   m_windowWidth;
    int   m_windowHeight;
    int   m_imageWidth;
    int   m_imageHeight;

private:
    Ui::QColorImageDisplay ui;
    cv::Mat    m_hoImage;
};
