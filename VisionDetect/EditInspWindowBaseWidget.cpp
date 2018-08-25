#include "EditInspWindowBaseWidget.h"
#include "InspWindowWidget.h"

#include "../Common/SystemData.h"

#include "VisionAPI.h"
#include "opencv2/opencv.hpp"

using namespace AOI;

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
    cv::Mat matMask;
    if (maskBinary.size() > 0)
    {
        matMask = cv::imdecode(maskBinary, CV_LOAD_IMAGE_GRAYSCALE);
    }
    return matMask;
}

Binary EditInspWindowBaseWidget::convertMaskMat2Bny(cv::Mat& maskMat)
{
    Binary maskBinary;

    if (!maskMat.empty())
    {
        cv::imencode(".bmp", maskMat, maskBinary);
    }

    return maskBinary;
}