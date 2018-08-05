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

cv::Mat EditInspWindowBaseWidget::getMaskMat()
{
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    auto width = m_currentWindow.width / dResolutionX;
    auto height = m_currentWindow.height / dResolutionY;

    int rows = height;
    int cols = width;

    cv::Mat maskMat = cv::Mat::zeros(rows, cols, CV_8UC1);
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            uchar& mask = maskMat.at<uchar>(row, col);
            mask = m_maskBinary[row*cols + col];
        }
    }

    return maskMat;
}