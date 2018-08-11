#include "EditInspWindowBaseWidget.h"
#include "InspWindowWidget.h"

#include "../Common/SystemData.h"

EditInspWindowBaseWidget::EditInspWindowBaseWidget(InspWindowWidget *parent) : QWidget(parent), m_pParent(parent), m_bSupportMask(false)
{
}

void EditInspWindowBaseWidget::setWindowGroup(const Engine::WindowGroup &windowGroup)
{
    m_windowGroup = windowGroup;
}

void EditInspWindowBaseWidget::setMask(cv::Mat& maskMat)
{ 
    m_maskMat = maskMat;    
}

cv::Mat EditInspWindowBaseWidget::convertMaskBny2Mat(Binary maskBinary)
{
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    auto width = m_currentWindow.width / dResolutionX;
    auto height = m_currentWindow.height / dResolutionY;

    int rows = static_cast<int>(height);
    int cols = static_cast<int>(width);  

    if (maskBinary.size() > 0)
    {
        cv::Mat maskMat = cv::Mat::zeros(rows, cols, CV_8UC1);

        for (int row = 0; row < rows; row++)
        {
            for (int col = 0; col < cols; col++)
            {
                uchar& mask = maskMat.at<uchar>(row, col);
                mask = maskBinary[row*cols + col];
            }
        }

        return maskMat;
    }  

    return cv::Mat();
}

Binary EditInspWindowBaseWidget::convertMaskMat2Bny(cv::Mat& maskMat)
{
    Binary maskBinary;

    if (!maskMat.empty())
    {
        cv::Mat matMaskImg = maskMat;
        for (int row = 0; row < matMaskImg.rows; ++row) {
            for (int col = 0; col < matMaskImg.cols; ++col) {
                AOI::Vision::Byte& mask = matMaskImg.at<AOI::Vision::Byte>(row, col);
                maskBinary.push_back(mask);
            }
        }
    }   

    return maskBinary;
}