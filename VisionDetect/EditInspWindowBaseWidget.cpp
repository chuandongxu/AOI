#include "EditInspWindowBaseWidget.h"
#include "InspWindowWidget.h"

#include "../Common/SystemData.h"
#include "../DataModule/QDetectObj.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"

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

void EditInspWindowBaseWidget::setMask(const cv::Mat& maskMat)
{
    m_maskMat = maskMat;
}

cv::Mat EditInspWindowBaseWidget::convertMaskBny2Mat(Binary maskBinary)
{
    cv::Mat matMask;
    if (!maskBinary.empty())
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

void EditInspWindowBaseWidget::updateWindowToUI(const Engine::Window &window, OPERATION enOperation) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);
    float dCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", dCombinedImageScale, 1.f);

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto matBigImage = pUI->getImage();
    int nBigImgWidth  = matBigImage.cols / dCombinedImageScale;
    int nBigImgHeight = matBigImage.rows / dCombinedImageScale;

    QDetectObj detectObj(window.Id, window.name.c_str());
    cv::Point2f ptCenter(window.x / dResolutionX, window.y / dResolutionY);
    if (bBoardRotated)
        ptCenter.x = nBigImgWidth  - ptCenter.x;
    else
        ptCenter.y = nBigImgHeight - ptCenter.y; //In cad, up is positive, but in image, down is positive.
    cv::Size winSizeUI(window.width / dResolutionX, window.height / dResolutionY);
    cv::Size srchWinSizeUI(window.srchWidth / dResolutionX, window.srchHeight / dResolutionY);
    detectObj.setFrame(cv::RotatedRect(ptCenter, winSizeUI, window.angle));
    detectObj.setSrchWindow(cv::RotatedRect(ptCenter, srchWinSizeUI, window.angle));
    auto vecDetectObjs = pUI->getDetectObjs();

    if (OPERATION::ADD == enOperation)
        vecDetectObjs.push_back(detectObj);
    else {
        auto iter = std::find_if(vecDetectObjs.begin(), vecDetectObjs.end(), [window](const QDetectObj& obj) { return window.Id == obj.getID(); });
        if (iter != vecDetectObjs.end()) {
            *iter = detectObj;
        }
    }
    pUI->setDetectObjs(vecDetectObjs);
}
