#include <QMessageBox>
#include "FiducialMarkWidget.h"
#include "../include/IVisionUI.h"
#include "../include/ICamera.h"
#include "../include/IVision.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../Common/SystemData.h"
#include "DataUtils.h"
#include "CalcUtils.hpp"
#include "SetFiducialMarkDialog.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/video.hpp"
#include "../Common/CommonFunc.h"
#include "QDetectObj.h"
#include "DataUtils.h"

FiducialMarkWidget::FiducialMarkWidget(DataCtrl *pDataCtrl, QWidget *parent)
:   m_pDataCtrl(pDataCtrl), 
    QWidget(parent) {
    ui.setupUi(this);

    connect(ui.comboBoxChooseImage, SIGNAL(currentIndexChanged(int)), SLOT(on_comboBoxChooseImage_indexChanged(int)));
}

FiducialMarkWidget::~FiducialMarkWidget() {
}

/*static*/ QString FiducialMarkWidget::formatAlignmentName(const Engine::Alignment &alignment, int No) {
    QString strAlignmentName = QString("FM_") + QString::number(No) + "_";
    strAlignmentName += alignment.isFM ? QString("Standard Shape_") : QString("Template");
    if (alignment.isFM) {
        if (alignment.fmShape == static_cast<int>(Vision::PR_FIDUCIAL_MARK_TYPE::SQUARE))
            strAlignmentName += "Square";
        else
            strAlignmentName += "Circle";
    }
    return strAlignmentName;
}

void FiducialMarkWidget::showEvent(QShowEvent *event) {
    auto pUI = getModule<IVisionUI>(UI_MODEL);

    auto vecCombinedBigImage = m_pDataCtrl->getCombinedBigImages();
    if (System->isRunOffline() && vecCombinedBigImage.empty()){
        float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);
        auto matImage = pUI->getImage();
        m_nBigImageWidth  = matImage.cols / fCombinedImageScale;
        m_nBigImageHeight = matImage.rows / fCombinedImageScale;
    }else {        
        int index = ui.comboBoxChooseImage->currentIndex();
        if (index >= 0 && index < vecCombinedBigImage.size() && !vecCombinedBigImage[index].empty())
            pUI->setImage(vecCombinedBigImage[index]);

        if (index >= 0 && index < vecCombinedBigImage.size())
        {
            m_nBigImageWidth = vecCombinedBigImage[index].cols;
            m_nBigImageHeight = vecCombinedBigImage[index].rows;
        }
    } 

    m_vecFMBigImagePos.clear();
    refreshFMWindow();
}

void FiducialMarkWidget::on_btnSelectFiducialMark_clicked() {
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SET_FIDUCIAL_MARK);
}

static std::vector<cv::Point> getCornerOfRotatedRect(const cv::RotatedRect &rotatedRect) {
    std::vector<cv::Point> vecPoint;
    cv::Point2f arrPt[4];
    rotatedRect.points(arrPt);
    for (int i = 0; i < 4; ++i)
        vecPoint.push_back(arrPt[i]);
    return vecPoint;
}

void FiducialMarkWidget::on_btnConfirmFiducialMark_clicked() {
    int iReturn = 0;
    Int32 nCountOfFrameX = 0, nCountOfFrameY = 0, nScanDirection = 0;
    Engine::GetParameter("ScanImageFrameCountX", nCountOfFrameX, 0);
    Engine::GetParameter("ScanImageFrameCountY", nCountOfFrameY, 0);
    Engine::GetParameter("ScanImageDirection", nScanDirection, 0);

    float dOverlapUmX = 0.f, dOverlapUmY = 0.f;
    Engine::GetParameter("ScanImageOverlapX", dOverlapUmX, 0.f);
    Engine::GetParameter("ScanImageOverlapY", dOverlapUmY, 0.f);

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY  = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);

    int nOverlapX = static_cast<int> (dOverlapUmX / dResolutionX + 0.5);
    int nOverlapY = static_cast<int> (dOverlapUmY / dResolutionY + 0.5);

    float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);
    

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto pCamera = getModule<ICamera>(CAMERA_MODEL);

    cv::RotatedRect rrectCadWindow, rrectImageWindow;
    pUI->getSelectDeviceWindow(rrectCadWindow, rrectImageWindow);

    cv::Rect rectCadFMWindow = rrectCadWindow.boundingRect();
    cv::RotatedRect rrSrchWindow(rrectCadWindow);
    rrSrchWindow.size.width  *= 4;
    rrSrchWindow.size.height *= 4;
    cv::Rect rectFMSrchWindow = rrSrchWindow.boundingRect();

    rrectCadWindow.center.x /= fCombinedImageScale;
    rrectCadWindow.center.y /= fCombinedImageScale;
    rrectCadWindow.size.width  /= fCombinedImageScale;
    rrectCadWindow.size.height /= fCombinedImageScale;

    bool bFound = false;
    for (const auto &rotatedRect : m_vecFMCadWindow) {
        auto contour = getCornerOfRotatedRect(rotatedRect);
        contour.push_back(contour.front());
        auto distance = cv::pointPolygonTest(contour, rrectCadWindow.center, false);
        if (distance >= 0) {
            bFound = true;
            break;
        }
    }

    if (bFound) {
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), QStringLiteral("This ficucial mark already added"), QStringLiteral("Quit"));
        return;
    }

    auto matBigImage = pUI->getImage();
    int nBigImgWidth  = matBigImage.cols / fCombinedImageScale;
    int nBigImgHeight = matBigImage.rows / fCombinedImageScale;

    int nImageWidth = 0, nImageHeight = 0;
    pCamera->getCameraScreenSize(nImageWidth, nImageHeight);

    int nSelectPtX = rrectImageWindow.center.x / fCombinedImageScale;
    int nSelectPtY = rrectImageWindow.center.y / fCombinedImageScale;

    int nFrameX, nFrameY;
    cv::Point ptInFrame;
    DataUtils::getFrameFromCombinedImage(nBigImgWidth, nBigImgHeight, nImageWidth, nImageHeight,
        nOverlapX, nOverlapY, nSelectPtX, nSelectPtY, nFrameX, nFrameY, ptInFrame.x, ptInFrame.y,
        static_cast<Vision::PR_SCAN_IMAGE_DIR>(nScanDirection));

    if (nFrameX < 0 || nFrameX >= nCountOfFrameX || nFrameY < 0 || nFrameY >= nCountOfFrameY) {
        System->showMessage(QStringLiteral("Fiducial Mark"), QStringLiteral("Frame计算错误, 请确定Frame overlap是否正确!"));
        return;
    }

    cv::Mat matFrameImg;
    auto vecBigImages = m_pDataCtrl->getCombinedBigImages();
    if (System->isRunOffline() && vecBigImages.empty()) {
        //matFrameImg = _readFrameImageFromFolder(nFrameX, nFrameY);
        matFrameImg = _getFrameImageFromBigImage(pUI->getImage(),
            nFrameX, nFrameY, nImageWidth, nImageHeight, nOverlapX, nOverlapY);
    }else {
        if (vecBigImages.empty()) {
            System->showMessage(QStringLiteral("Fiducial Mark"), QStringLiteral("请先扫描电路板!"));
            return;
        }
        int nImageIndex = ui.comboBoxChooseImage->currentIndex();
        matFrameImg = _getFrameImageFromBigImage(vecBigImages[nImageIndex],
            nFrameX, nFrameY, nImageWidth, nImageHeight, nOverlapX, nOverlapY);
    }

    SetFiducialMarkDialog dialogSetFM;
    dialogSetFM.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialogSetFM.show();
    dialogSetFM.raise();
    dialogSetFM.activateWindow();
    iReturn = dialogSetFM.exec();
    if (iReturn != QDialog::Accepted)
        return;

    bool bUseStandardShape = dialogSetFM.getUseStandardShape();
    if (bUseStandardShape) {
        iReturn = _learnStandardFM(dialogSetFM.getFiducialMarkSize(),
                                   dialogSetFM.getFiducialMarkShape(),
                                   dialogSetFM.getIsFiducialMarkDark(),
                                   matFrameImg,
                                   rrectCadWindow,
                                   nFrameX,
                                   nFrameY,
                                   ptInFrame);
        if (OK != iReturn)
            return;
    }else {
        iReturn = _learnRealImageFM(matFrameImg,
                                    rrectCadWindow,
                                    nFrameX,
                                    nFrameY,
                                    ptInFrame);
        if (OK != iReturn)
            return;
    }
    pUI->setCurrentFM(VisionViewFM(0, cv::Rect(), cv::Rect())); //Clear selection.
    refreshFMWindow();
}

int FiducialMarkWidget::_learnStandardFM(float                          fFMSizeMM, 
                                         Vision::PR_FIDUCIAL_MARK_TYPE  enType,
                                         bool                           bDark,
                                         const cv::Mat                 &matFrameImg,
                                         const cv::RotatedRect         &rrectCadWindow,
                                         int                            nFrameX,
                                         int                            nFrameY,
                                         const cv::Point               &ptInFrame) {
    float fOverlapUmX = 0.f, fOverlapUmY = 0.f;
    Engine::GetParameter("ScanImageOverlapX", fOverlapUmX, 0.f);
    Engine::GetParameter("ScanImageOverlapY", fOverlapUmY, 0.f);

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    int nOverlapX = static_cast<int> (fOverlapUmX / dResolutionX + 0.5);
    int nOverlapY = static_cast<int> (fOverlapUmY / dResolutionY + 0.5);

    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);
    float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);
    Int32 nScanDirection = 0; Engine::GetParameter("ScanImageDirection", nScanDirection, 0);

    int nImageWidth = 0, nImageHeight = 0;
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto pCamera = getModule<ICamera>(CAMERA_MODEL);
    pCamera->getCameraScreenSize(nImageWidth, nImageHeight);

    auto matBigImage = pUI->getImage();
    int nBigImgWidth  = matBigImage.cols / fCombinedImageScale;
    int nBigImgHeight = matBigImage.rows / fCombinedImageScale;

    cv::Rect rectCadFMWindow = rrectCadWindow.boundingRect();
    cv::RotatedRect rrSrchWindow(rrectCadWindow);
    rrSrchWindow.size.width  *= 4;
    rrSrchWindow.size.height *= 4;
    cv::Rect rectFMSrchWindow = rrSrchWindow.boundingRect();

    float fFMImgSizePixel = fFMSizeMM * MM_TO_UM / dResolutionX;
    rectCadFMWindow = CalcUtils::resizeRect(rectCadFMWindow, cv::Size(fFMImgSizePixel * fCombinedImageScale, fFMImgSizePixel * fCombinedImageScale));
    VisionViewFM fm(0, rectCadFMWindow, rectFMSrchWindow);
    
    pUI->setCurrentFM(fm);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_FM_SRCH_WINDOW);
    int nReturn = System->showInteractMessage(QStringLiteral("Fiducial Mark"), QStringLiteral("Please select the search window of the fiducial mark"));
    if (nReturn != QDialog::Accepted)
        return NOK;

    rectFMSrchWindow = pUI->getCurrentFM().getSrchWindow();
    rectFMSrchWindow.width  /= fCombinedImageScale;
    rectFMSrchWindow.height /= fCombinedImageScale;
    Vision::PR_SRCH_FIDUCIAL_MARK_CMD stCmd;
    Vision::PR_SRCH_FIDUCIAL_MARK_RPY stRpy;
    stCmd.matInputImg = matFrameImg;
    stCmd.enType = enType;
    stCmd.fSize = fFMImgSizePixel;
    stCmd.fMargin = stCmd.fSize / 2.f;
    stCmd.rectSrchWindow = cv::Rect(ptInFrame.x - rectFMSrchWindow.width / 2, ptInFrame.y - rectFMSrchWindow.height / 2,
        rectFMSrchWindow.width, rectFMSrchWindow.height);
    if (stCmd.rectSrchWindow.x < 0) stCmd.rectSrchWindow.x = 0;
    if (stCmd.rectSrchWindow.y < 0) stCmd.rectSrchWindow.y = 0;
    if ((stCmd.rectSrchWindow.x + stCmd.rectSrchWindow.width) > stCmd.matInputImg.cols)
        stCmd.rectSrchWindow.width = stCmd.matInputImg.cols - stCmd.rectSrchWindow.x;
    if ((stCmd.rectSrchWindow.y + stCmd.rectSrchWindow.height) > stCmd.matInputImg.rows)
        stCmd.rectSrchWindow.height = stCmd.matInputImg.rows - stCmd.rectSrchWindow.y;

    PR_SrchFiducialMark(&stCmd, &stRpy);
    if (stRpy.enStatus != Vision::VisionStatus::OK) {
        Vision::PR_GET_ERROR_INFO_RPY stErrStrRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrStrRpy);
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), stErrStrRpy.achErrorStr, QStringLiteral("Quit"));
        return NOK;
    }
    QString qstrMsg;
    qstrMsg.sprintf("Success to search fiducial mark, match score %f, pos in original image [%f, %f].", stRpy.fMatchScore, stRpy.ptPos.x, stRpy.ptPos.y);
    System->setTrackInfo(qstrMsg);

    int nPosInCombineImageX, nPosInCombineImageY;
    DataUtils::getCombinedImagePosFromFramePos(nBigImgWidth, nBigImgHeight, nImageWidth, nImageHeight,
        nOverlapX, nOverlapY, nFrameX, nFrameY, stRpy.ptPos.x, stRpy.ptPos.y, nPosInCombineImageX, nPosInCombineImageY,
        static_cast<Vision::PR_SCAN_IMAGE_DIR>(nScanDirection));

    m_vecFMBigImagePos.push_back(cv::Point2f(nPosInCombineImageX, nPosInCombineImageY));

    Engine::Alignment alignment;
    if (bBoardRotated) {
        alignment.tmplPosX = (nBigImgWidth - rrectCadWindow.center.x)  * dResolutionX;
        alignment.tmplPosY = rrectCadWindow.center.y * dResolutionY;
    }
    else {
        alignment.tmplPosX = rrectCadWindow.center.x * dResolutionX;
        alignment.tmplPosY = (nBigImgHeight - rrectCadWindow.center.y) * dResolutionY;
    }

    alignment.tmplWidth  = fFMSizeMM * MM_TO_UM;
    alignment.tmplHeight = fFMSizeMM * MM_TO_UM;
    alignment.srchWinWidth  = rectFMSrchWindow.width  * dResolutionX;
    alignment.srchWinHeight = rectFMSrchWindow.height * dResolutionY;
    alignment.isFM = true;
    alignment.fmShape = static_cast<int>(enType);
    alignment.isFMDark = bDark;
    alignment.lightId = ui.comboBoxChooseImage->currentIndex() + 1;
    auto result = Engine::CreateAlignment(alignment);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString strMsg(QStringLiteral("添加模块到数据库失败, 错误消息: "));
        strMsg += errorMessage.c_str();
        System->showMessage(QStringLiteral("Fiducial Mark"), strMsg);
        return NOK;
    }

    return OK;
}

int FiducialMarkWidget::_learnRealImageFM(const cv::Mat                 &matFrameImg,
                                          const cv::RotatedRect         &rrectCadWindow,
                                          int                            nFrameX,
                                          int                            nFrameY,
                                          const cv::Point               &ptInFrame) {
    float fOverlapUmX = 0.f, fOverlapUmY = 0.f;
    Engine::GetParameter("ScanImageOverlapX", fOverlapUmX, 0.f);
    Engine::GetParameter("ScanImageOverlapY", fOverlapUmY, 0.f);

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    int nOverlapX = static_cast<int> (fOverlapUmX / dResolutionX + 0.5);
    int nOverlapY = static_cast<int> (fOverlapUmY / dResolutionY + 0.5);

    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);
    float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);
    Int32 nScanDirection = 0; Engine::GetParameter("ScanImageDirection", nScanDirection, 0);

    int nImageWidth = 0, nImageHeight = 0;
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto pCamera = getModule<ICamera>(CAMERA_MODEL);
    pCamera->getCameraScreenSize(nImageWidth, nImageHeight);

    auto matBigImage = pUI->getImage();
    int nBigImgWidth  = matBigImage.cols / fCombinedImageScale;
    int nBigImgHeight = matBigImage.rows / fCombinedImageScale;

    cv::Rect rectCadFMWindow = rrectCadWindow.boundingRect();
    cv::RotatedRect rrSrchWindow(rrectCadWindow);
    rrSrchWindow.size.width  *= 2;
    rrSrchWindow.size.height *= 2;
    cv::Rect rectFMSrchWindow = rrSrchWindow.boundingRect();

    VisionViewFM fm(0, rectCadFMWindow, rectFMSrchWindow);
    pUI->setCurrentFM(fm);

    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_FIDUCIAL_MARK);
    int nReturn = System->showInteractMessage(QStringLiteral("Fiducial Mark"), QStringLiteral("请拖动鼠标调整基准块区域!"));
    if (nReturn != QDialog::Accepted)
        return NOK;

    // Get FM here
    fm = pUI->getCurrentFM();
    cv::Rect rectFM = fm.getFM();
    cv::Rect rectSrchWindow = CalcUtils::resizeRect(rectFM, cv::Size(rectFM.width * 2, rectFM.height * 2));
    fm.setSrchWindow(rectSrchWindow);
    pUI->setCurrentFM(fm);

    // Ask user to select the search window
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_FM_SRCH_WINDOW);
    nReturn = System->showInteractMessage(QStringLiteral("Fiducial Mark"), QStringLiteral("Please select the search window of the fiducial mark"));
    if (nReturn != QDialog::Accepted)
        return NOK;

    // Get the search window
    fm = pUI->getCurrentFM();
    rectFM = fm.getFM();
    rectFMSrchWindow = fm.getSrchWindow();
    
    Vision::PR_LRN_TEMPLATE_CMD stLrnTmplCmd;
    Vision::PR_LRN_TEMPLATE_RPY stLrnTmplRpy;
    stLrnTmplCmd.matInputImg = matFrameImg;
    stLrnTmplCmd.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
    stLrnTmplCmd.rectROI = cv::Rect(ptInFrame.x - rectFM.width / 2, ptInFrame.y - rectFM.height / 2,
        rectFM.width, rectFM.height);
    Vision::PR_LrnTmpl(&stLrnTmplCmd, &stLrnTmplRpy);
    if (Vision::VisionStatus::OK != stLrnTmplRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stErrStrRpy;
        Vision::PR_GetErrorInfo(stLrnTmplRpy.enStatus, &stErrStrRpy);
        QString strMsg(QStringLiteral("学习模板失败, 错误消息: "));
        strMsg += stErrStrRpy.achErrorStr;
        System->showMessage(QStringLiteral("Fiducial Mark"), strMsg);
        return NOK;
    }

    Vision::PR_MATCH_TEMPLATE_CMD stMatchTmplCmd;
    Vision::PR_MATCH_TEMPLATE_RPY stMatchTmplRpy;
    stMatchTmplCmd.matInputImg = matFrameImg;
    stMatchTmplCmd.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
    stMatchTmplCmd.nRecordId = stLrnTmplRpy.nRecordId;
    stMatchTmplCmd.rectSrchWindow = cv::Rect(ptInFrame.x - rectFMSrchWindow.width / 2, ptInFrame.y - rectFMSrchWindow.height / 2,
        rectFMSrchWindow.width, rectFMSrchWindow.height);
    if (stMatchTmplCmd.rectSrchWindow.x < 0) stMatchTmplCmd.rectSrchWindow.x = 0;
    if (stMatchTmplCmd.rectSrchWindow.y < 0) stMatchTmplCmd.rectSrchWindow.y = 0;
    if ((stMatchTmplCmd.rectSrchWindow.x + stMatchTmplCmd.rectSrchWindow.width) > stMatchTmplCmd.matInputImg.cols)
        stMatchTmplCmd.rectSrchWindow.width = stMatchTmplCmd.matInputImg.cols - stMatchTmplCmd.rectSrchWindow.x;
    if ((stMatchTmplCmd.rectSrchWindow.y + stMatchTmplCmd.rectSrchWindow.height) > stMatchTmplCmd.matInputImg.rows)
        stMatchTmplCmd.rectSrchWindow.height = stMatchTmplCmd.matInputImg.rows - stMatchTmplCmd.rectSrchWindow.y;

    PR_MatchTmpl(&stMatchTmplCmd, &stMatchTmplRpy);
    if (stMatchTmplRpy.enStatus != Vision::VisionStatus::OK) {
        Vision::PR_GET_ERROR_INFO_RPY stErrStrRpy;
        Vision::PR_GetErrorInfo(stLrnTmplRpy.enStatus, &stErrStrRpy);
        QString strMsg(QStringLiteral("搜寻模板失败, 错误消息: "));
        strMsg += stErrStrRpy.achErrorStr;
        System->showMessage(QStringLiteral("Fiducial Mark"), strMsg);
        return NOK;
    }

    QString qstrMsg;
    qstrMsg.sprintf("Success to search fiducial mark, match score %f, pos in original image [%f, %f].", stMatchTmplRpy.fMatchScore, stMatchTmplRpy.ptObjPos.x, stMatchTmplRpy.ptObjPos.y);
    System->setTrackInfo(qstrMsg);

    int nPosInCombineImageX, nPosInCombineImageY;
    DataUtils::getCombinedImagePosFromFramePos(nBigImgWidth, nBigImgHeight, nImageWidth, nImageHeight,
        nOverlapX, nOverlapY, nFrameX, nFrameY, stMatchTmplRpy.ptObjPos.x, stMatchTmplRpy.ptObjPos.y, nPosInCombineImageX, nPosInCombineImageY,
        static_cast<Vision::PR_SCAN_IMAGE_DIR>(nScanDirection));

    m_vecFMBigImagePos.push_back(cv::Point2f(nPosInCombineImageX, nPosInCombineImageY));

    Engine::Alignment alignment;
    if (bBoardRotated) {
        alignment.tmplPosX = (nBigImgWidth - rrectCadWindow.center.x)  * dResolutionX;
        alignment.tmplPosY = rrectCadWindow.center.y * dResolutionY;
    }
    else {
        alignment.tmplPosX = rrectCadWindow.center.x * dResolutionX;
        alignment.tmplPosY = (nBigImgHeight - rrectCadWindow.center.y) * dResolutionY;
    }

    alignment.tmplWidth  = rectFM.width  * dResolutionX;
    alignment.tmplHeight = rectFM.height * dResolutionY;
    alignment.srchWinWidth  = rectFMSrchWindow.width  * dResolutionX;
    alignment.srchWinHeight = rectFMSrchWindow.height * dResolutionY;
    alignment.isFM = false;
    alignment.lightId = ui.comboBoxChooseImage->currentIndex() + 1;
    alignment.recordId = stLrnTmplRpy.nRecordId;

    if (ReadBinaryFile(FormatRecordName(alignment.recordId), alignment.recordData) != 0) {
        QString strMsg(QStringLiteral("读取模板Id %1 失败!").arg(alignment.recordId));
        System->showMessage(QStringLiteral("Fiducial Mark"), strMsg);
        return NOK;
    }
    auto result = Engine::CreateAlignment(alignment);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString strMsg(QStringLiteral("添加模块到数据库失败, 错误消息: "));
        strMsg += errorMessage.c_str();
        System->showMessage(QStringLiteral("Fiducial Mark"), strMsg);
        return NOK;
    }

    return OK;
}

//Search all the fiducial mark.
int FiducialMarkWidget::srchFiducialMark() {
    float fOverlapUmX = 0.f, fOverlapUmY = 0.f;
    Engine::GetParameter("ScanImageOverlapX", fOverlapUmX, 0.f);
    Engine::GetParameter("ScanImageOverlapY", fOverlapUmY, 0.f);

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    int nOverlapX = static_cast<int> (fOverlapUmX / dResolutionX + 0.5);
    int nOverlapY = static_cast<int> (fOverlapUmY / dResolutionY + 0.5);

    Int32 nScanDirection = 0;
    Engine::GetParameter("ScanImageDirection", nScanDirection, 0);
    float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);
    auto strImageFolder = System->getParam("scan_image_Folder").toString();

    auto result = Engine::GetAllAlignments(m_vecAlignmentDB);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get alignment from data base, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Do alignment"), errorMessage.c_str(), QStringLiteral("Quit"));
        return NOK;
    }

    if (m_vecAlignmentDB.size() != m_vecFMCadWindow.size()) {
        QString strMsg;
        strMsg.sprintf("Software error, the DB alignment count %d not match with CAD alignment count %d.",
            m_vecAlignmentDB.size(), m_vecFMCadWindow.size());
        QMessageBox::critical(nullptr, QStringLiteral("Do alignment"), strMsg, QStringLiteral("Quit"));
        return NOK;
    }

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto pCamera = getModule<ICamera>(CAMERA_MODEL);

    auto matImage = pUI->getImage();
    int nBigImgWidth  = matImage.cols / fCombinedImageScale;
    int nBigImgHeight = matImage.rows / fCombinedImageScale;

    int nImageWidth = 0, nImageHeight = 0;
    pCamera->getCameraScreenSize(nImageWidth, nImageHeight);

    //Research for the fiducial mark again to do alignment. There is one problem here, the fiducial shape is not remembered.
    for (size_t i = 0; i < m_vecFMCadWindow.size(); ++ i) {
        auto rrectCadWindow = m_vecFMCadWindow[i];
        int nSelectPtX = rrectCadWindow.center.x;
        int nSelectPtY = rrectCadWindow.center.y;

        int nFrameX, nFrameY;
        cv::Point ptInFrame;
        DataUtils::getFrameFromCombinedImage(nBigImgWidth, nBigImgHeight, nImageWidth, nImageHeight,
            nOverlapX, nOverlapY, nSelectPtX, nSelectPtY, nFrameX, nFrameY, ptInFrame.x, ptInFrame.y,
            static_cast<Vision::PR_SCAN_IMAGE_DIR> (nScanDirection));

        cv::Mat matFrameImg;
        auto vecBigImages = m_pDataCtrl->getCombinedBigImages();
        if (System->isRunOffline() && vecBigImages.empty())
            //matFrameImg = _readFrameImageFromFolder(nFrameX, nFrameY);
            matFrameImg = _getFrameImageFromBigImage(pUI->getImage(),
            nFrameX, nFrameY, nImageWidth, nImageHeight, nOverlapX, nOverlapY);
        else {
            if (vecBigImages.empty()) {
                System->showMessage(QStringLiteral("Fiducial Mark"), QStringLiteral("请先扫描电路板!"));
                return NOK;
            }
            int nImageIndex = ui.comboBoxChooseImage->currentIndex();
            matFrameImg = _getFrameImageFromBigImage(vecBigImages[nImageIndex],
                nFrameX, nFrameY, nImageWidth, nImageHeight, nOverlapX, nOverlapY);
        }

        cv::Point2f ptResult;
        int nReturn = OK;
        if (m_vecAlignmentDB[i].isFM)
            nReturn = _srchStandardFM(matFrameImg, m_vecAlignmentDB[i], ptInFrame, ptResult);
        else
            nReturn = _srchRealImageFM(matFrameImg, m_vecAlignmentDB[i], ptInFrame, ptResult);

        if (nReturn != OK)
            return nReturn;

        int nPosInCombineImageX, nPosInCombineImageY;
        DataUtils::getCombinedImagePosFromFramePos(nBigImgWidth, nBigImgHeight, nImageWidth, nImageHeight,
            nOverlapX, nOverlapY, nFrameX, nFrameY, ptResult.x, ptResult.y, nPosInCombineImageX, nPosInCombineImageY,
            static_cast<Vision::PR_SCAN_IMAGE_DIR> (nScanDirection));
        m_vecFMBigImagePos.push_back(cv::Point(nPosInCombineImageX, nPosInCombineImageY));
    }
    return OK;
}

int FiducialMarkWidget::_srchStandardFM(const cv::Mat           &matFrameImg,
                                        const Engine::Alignment &alignment,
                                        const cv::Point         &ptInFrame,
                                        cv::Point2f             &ptResult)
{
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    Vision::PR_SRCH_FIDUCIAL_MARK_CMD stCmd;
    Vision::PR_SRCH_FIDUCIAL_MARK_RPY stRpy;
    stCmd.matInputImg = matFrameImg;
    stCmd.enType = static_cast<Vision::PR_FIDUCIAL_MARK_TYPE> (alignment.fmShape);
    stCmd.fSize = alignment.tmplWidth / dResolutionX;
    stCmd.fMargin = stCmd.fSize / 2.f;
    int nSrchWinWidth  = alignment.srchWinWidth  / dResolutionX;
    int nSrchWinHeight = alignment.srchWinHeight / dResolutionY;
    stCmd.rectSrchWindow = cv::Rect(ptInFrame.x - nSrchWinWidth / 2, ptInFrame.y - nSrchWinHeight / 2,
        nSrchWinWidth, nSrchWinHeight);
    if (stCmd.rectSrchWindow.x < 0) stCmd.rectSrchWindow.x = 0;
    if (stCmd.rectSrchWindow.y < 0) stCmd.rectSrchWindow.y = 0;
    if ((stCmd.rectSrchWindow.x + stCmd.rectSrchWindow.width) > stCmd.matInputImg.cols)
        stCmd.rectSrchWindow.width = stCmd.matInputImg.cols - stCmd.rectSrchWindow.x;
    if ((stCmd.rectSrchWindow.y + stCmd.rectSrchWindow.height) > stCmd.matInputImg.rows)
        stCmd.rectSrchWindow.height = stCmd.matInputImg.rows - stCmd.rectSrchWindow.y;

    PR_SrchFiducialMark(&stCmd, &stRpy);
    if (stRpy.enStatus != Vision::VisionStatus::OK) {
        Vision::PR_GET_ERROR_INFO_RPY stErrStrRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrStrRpy);
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), stErrStrRpy.achErrorStr, QStringLiteral("Quit"));
        return NOK;
    }
    ptResult = stRpy.ptPos;

    QString qstrMsg;
    qstrMsg.sprintf("Success to search FM, score %f, pos in original image [%f, %f].", stRpy.fMatchScore, stRpy.ptPos.x, stRpy.ptPos.y);
    System->setTrackInfo(qstrMsg);
    return OK;
}

int FiducialMarkWidget::_srchRealImageFM(const cv::Mat           &matFrameImg,
                                         const Engine::Alignment &alignment,
                                         const cv::Point         &ptInFrame,
                                         cv::Point2f             &ptResult)
{
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    Vision::PR_MATCH_TEMPLATE_CMD stCmd;
    Vision::PR_MATCH_TEMPLATE_RPY stRpy;
    stCmd.matInputImg = matFrameImg;
    stCmd.nRecordId = alignment.recordId;
    int nSrchWinWidth  = alignment.srchWinWidth  / dResolutionX;
    int nSrchWinHeight = alignment.srchWinHeight / dResolutionY;
    stCmd.rectSrchWindow = cv::Rect(ptInFrame.x - nSrchWinWidth / 2, ptInFrame.y - nSrchWinHeight / 2,
        nSrchWinWidth, nSrchWinHeight);
    if (stCmd.rectSrchWindow.x < 0) stCmd.rectSrchWindow.x = 0;
    if (stCmd.rectSrchWindow.y < 0) stCmd.rectSrchWindow.y = 0;
    if ((stCmd.rectSrchWindow.x + stCmd.rectSrchWindow.width) > stCmd.matInputImg.cols)
        stCmd.rectSrchWindow.width = stCmd.matInputImg.cols - stCmd.rectSrchWindow.x;
    if ((stCmd.rectSrchWindow.y + stCmd.rectSrchWindow.height) > stCmd.matInputImg.rows)
        stCmd.rectSrchWindow.height = stCmd.matInputImg.rows - stCmd.rectSrchWindow.y;

    PR_MatchTmpl(&stCmd, &stRpy);
    if (stRpy.enStatus != Vision::VisionStatus::OK) {
        Vision::PR_GET_ERROR_INFO_RPY stErrStrRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrStrRpy);
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), stErrStrRpy.achErrorStr, QStringLiteral("Quit"));
        return NOK;
    }
    ptResult = stRpy.ptObjPos;

    QString qstrMsg;
    qstrMsg.sprintf("Success to search FM, score %f, pos in original image [%f, %f].", stRpy.fMatchScore, stRpy.ptObjPos.x, stRpy.ptObjPos.y);
    System->setTrackInfo(qstrMsg);
    return OK;
}

int FiducialMarkWidget::refreshFMWindow() {
    float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);

    auto result = Engine::GetAllAlignments(m_vecAlignmentDB);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get alignment from database, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Set Fiducial Mark"), errorMessage.c_str(), QStringLiteral("Quit"));
        return NOK;
    }

    double dResolutionX, dResolutionY;
    bool bBoardRotated = false;
    dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();

    m_vecFMCadWindow.clear();
    int index = 0;
    ui.listFiducialMarkWidget->clear();
    VisionViewFMVector vecFM;
    for (const auto &alignment : m_vecAlignmentDB) {
        auto x = alignment.tmplPosX / dResolutionX;
        auto y = alignment.tmplPosY / dResolutionY;
        if (bBoardRotated)
            x = m_nBigImageWidth  - x;
        else
            y = m_nBigImageHeight - y; //In cad, up is positive, but in image, down is positive.
        auto width  = alignment.tmplWidth  / dResolutionX;
        auto height = alignment.tmplHeight / dResolutionY;
        auto srchWinWidth  = alignment.srchWinWidth  / dResolutionX;
        auto srchWinHeight = alignment.srchWinHeight / dResolutionY;
        cv::RotatedRect alignmentWindow(cv::Point2f(x, y), cv::Size2f(width, height), 0);
        m_vecFMCadWindow.push_back(alignmentWindow);

        alignmentWindow.center.x *= fCombinedImageScale;
        alignmentWindow.center.y *= fCombinedImageScale;
        alignmentWindow.size.width  *= fCombinedImageScale;
        alignmentWindow.size.height *= fCombinedImageScale;

        cv::RotatedRect srchWindow(cv::Point2f(x, y), cv::Size2f(srchWinWidth, srchWinHeight), 0);
        srchWindow.center.x *= fCombinedImageScale;
        srchWindow.center.y *= fCombinedImageScale;
        srchWindow.size.width  *= fCombinedImageScale;
        srchWindow.size.height *= fCombinedImageScale;

        VisionViewFM fm(alignment.Id, alignmentWindow.boundingRect(), srchWindow.boundingRect());
        vecFM.push_back(fm);

        QString strAlignmentName = formatAlignmentName(alignment, ++ index);
        ui.listFiducialMarkWidget->addItem(strAlignmentName);
    }
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setConfirmedFM(vecFM);
    return OK;
}

cv::Mat FiducialMarkWidget::_readFrameImageFromFolder(int nFrameX, int nFrameY) {
    Int32 nCountOfImgPerFrame = 0, nCountOfImgPerRow = 0;
    Engine::GetParameter("ScanImageOneFrameImageCount", nCountOfImgPerFrame, 0);
    Engine::GetParameter("ScanImageRowImageCount", nCountOfImgPerRow, 0);

    auto strImageFolder = System->getParam("scan_image_Folder").toString();

    char arrCharFileName[100];
    int nImageIndex = nFrameX * nCountOfImgPerFrame + nFrameY * nCountOfImgPerRow + 1;
    _snprintf(arrCharFileName, sizeof(arrCharFileName), "/F%d-%d-1.bmp", nFrameY + 1, nImageIndex);

    auto strFrameImagePath = strImageFolder + arrCharFileName;
    auto matFrameImg = cv::imread(strFrameImagePath.toStdString(), cv::IMREAD_GRAYSCALE);
    if (matFrameImg.empty()) {
        QString strMsg = QString("Failed to read frame image: ") + strFrameImagePath;
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), strMsg, QStringLiteral("Quit"));
        return cv::Mat();
    }
    cv::cvtColor(matFrameImg, matFrameImg, CV_BayerGR2BGR);
    return matFrameImg;
}

cv::Mat FiducialMarkWidget::_getFrameImageFromBigImage(const cv::Mat &matBigImage, int nFrameX, int nFrameY, int nImageWidth, int nImageHeight, int nOverlapX, int nOverlapY) {
    if (matBigImage.empty())
        return cv::Mat();

    float frameLeftX = nFrameX * (nImageWidth  - nOverlapX);
    float frameTopY  = nFrameY * (nImageHeight - nOverlapY);
    if ((frameLeftX + nImageWidth  > matBigImage.cols) ||
        (frameTopY  + nImageHeight > matBigImage.rows))
        return cv::Mat();
    cv::Mat matFrame(nImageHeight, nImageWidth, matBigImage.type());
    cv::Mat matROI(matBigImage, cv::Rect(frameLeftX, frameTopY, nImageWidth, nImageHeight));
    matROI.copyTo(matFrame);
    return matFrame;
}

void FiducialMarkWidget::on_btnDoAlignment_clicked() {
    if (m_vecFMCadWindow.size() < 2) {
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), QStringLiteral("Please select at least 2 fiducial mark to do alignment."), QStringLiteral("Quit"));
        return;
    }

    if (m_vecFMBigImagePos.size() < m_vecFMCadWindow.size()) {
        m_vecFMBigImagePos.clear();
        if (srchFiducialMark() != OK)
            return;
    }

    double dResolutionX, dResolutionY;
    bool bBoardRotated = false;
    dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);

    float fRotationInRadian, Tx, Ty, fScale;
    cv::Mat matTransform;
    if (m_vecFMCadWindow.size() >= 3) {
        Vision::VectorOfPoint2f vecPointCad;
        for (const auto &rotatedRect : m_vecFMCadWindow)
            vecPointCad.push_back(rotatedRect.center);

        if (m_vecFMCadWindow.size() == 3)
            matTransform = cv::getAffineTransform(vecPointCad, m_vecFMBigImagePos);
        else
            matTransform = cv::estimateRigidTransform(vecPointCad, m_vecFMBigImagePos, true);

        matTransform.convertTo(matTransform, CV_32FC1);

        m_fCadOffsetX = matTransform.at<float>(0, 2) * dResolutionX;
        m_fCadOffsetY = matTransform.at<float>(1, 2) * dResolutionY;
    }
    else {
        DataUtils::alignWithTwoPoints(m_vecFMCadWindow[0].center,
            m_vecFMCadWindow[1].center,
            m_vecFMBigImagePos[0],
            m_vecFMBigImagePos[1],
            fRotationInRadian, Tx, Ty, fScale);

        m_fCadOffsetX = Tx * dResolutionX;
        m_fCadOffsetY = Ty * dResolutionY;

        cv::Point2f ptCtr(m_vecFMCadWindow[0].center.x / 2.f + m_vecFMCadWindow[1].center.x / 2.f, m_vecFMCadWindow[0].center.y / 2.f + m_vecFMCadWindow[1].center.y / 2.f);
        //cv::Point2f ptCtr( m_nBigImageWidth / 2.f, m_nBigImageHeight / 2.f );
        double fDegree = fRotationInRadian * 180. / CV_PI;
        matTransform = cv::getRotationMatrix2D(ptCtr, fDegree, fScale);
        matTransform.at<double>(0, 2) += Tx;
        matTransform.at<double>(1, 2) += Ty;
        matTransform.convertTo(matTransform, CV_32FC1);
    }

    m_pDataCtrl->setCadTransform(matTransform);

    Engine::BoardVector vecBoard;
    auto result = Engine::GetAllBoards(vecBoard);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get board from project, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    VisionViewDeviceVector vecVisionViewDevices;
    for (const auto &board : vecBoard) {
        Engine::DeviceVector vecDevice;
        auto result = Engine::GetBoardDevice(board.Id, vecDevice);
        if (Engine::OK != result) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            errorMessage = "Failed to get devices from project, error message " + errorMessage;
            QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
            return;
        }

        for (const auto &device : vecDevice) {
            if (device.isBottom)
                continue;

            auto x = (device.x + board.x) / dResolutionX;
            auto y = (device.y + board.y) / dResolutionY;
            if (bBoardRotated)
                x = m_nBigImageWidth  - x;
            else
                y = m_nBigImageHeight - y; //In cad, up is positive, but in image, down is positive.

            std::vector<float> vecSrcPos;
            vecSrcPos.push_back(x);
            vecSrcPos.push_back(y);
            vecSrcPos.push_back(1);
            cv::Mat matSrcPos(vecSrcPos);
            cv::Mat matDestPos = matTransform * matSrcPos;
            x = matDestPos.at<float>(0) * fCombinedImageScale;
            y = matDestPos.at<float>(1) * fCombinedImageScale;

            auto width  = device.width  / dResolutionX * fCombinedImageScale;
            auto height = device.height / dResolutionY * fCombinedImageScale;
            cv::RotatedRect deviceWindow(cv::Point2f(x, y), cv::Size2f(width, height), device.angle);
            vecVisionViewDevices.emplace_back(device.Id, device.name, deviceWindow);
        }
    }
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setDeviceWindows(vecVisionViewDevices);

    QString strTitle(QStringLiteral("更新定位块"));
    //Update the offset to board.
    for (auto &board : vecBoard) {
        if (bBoardRotated) {
            board.x += -m_fCadOffsetX;
            board.y +=  m_fCadOffsetY;
        }
        else {
            board.x +=  m_fCadOffsetX;
            board.y += -m_fCadOffsetY;
        }

        result = Engine::UpdateBoard(board);
        if (Engine::OK != result) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            errorMessage = "Failed to update board, error message " + errorMessage;
            QMessageBox::critical(nullptr, strTitle, errorMessage.c_str(), QStringLiteral("Quit"));
            return;
        }
    }

    Engine::AlignmentVector vecAlignment;
    Engine::GetAllAlignments(vecAlignment);
    for (auto &alignment : vecAlignment) {
        if (bBoardRotated) {
            alignment.tmplPosX += -m_fCadOffsetX;
            alignment.tmplPosY +=  m_fCadOffsetY;
        }
        else {
            alignment.tmplPosX +=  m_fCadOffsetX;
            alignment.tmplPosY += -m_fCadOffsetY;
        }
        result = Engine::UpdateAlignment(alignment);
        if (Engine::OK != result) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString strMsg(QStringLiteral("更新定位块失败, 错误消息: "));
            strMsg += errorMessage.c_str();
            System->showMessage(strTitle, strMsg);
            return;
        }
    }

    Engine::WindowVector vecWindow;
    Engine::GetAllWindows(vecWindow);
    for (auto &window : vecWindow) {
        if (bBoardRotated) {
            window.x += -m_fCadOffsetX;
            window.y +=  m_fCadOffsetY;
        }
        else {
            window.x +=  m_fCadOffsetX;
            window.y += -m_fCadOffsetY;
        }
        result = Engine::UpdateWindow(window);
        if (Engine::OK != result) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString strMsg(QStringLiteral("更新检测框坐标失败, 错误消息: "));
            strMsg += errorMessage.c_str();
            System->showMessage(strTitle, strMsg);
            return;
        }
    }    

    // Refresh the select FM windows.
    refreshFMWindow();

    // Refresh the inspect windows
    if (!pUI->getDetectObjs().isEmpty()) {
        auto pVision = getModule<IVision>(VISION_MODEL);
        pVision->refreshAllDeviceWindows();
    }
}

void FiducialMarkWidget::on_btnRemoveFM_clicked() {
    auto index = ui.listFiducialMarkWidget->currentRow();
    if (index >= 0) {
        auto alignmentId = m_vecAlignmentDB[index].Id;
        QString msg = "Are you sure to delete the fiducial mark " + formatAlignmentName(m_vecAlignmentDB[index], index + 1);
        int result = QMessageBox::warning(this, "Remove Fiducial Mark", msg, QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Cancel);
        if (result != QMessageBox::StandardButton::Ok)
            return;

        Engine::DeleteAlignment(alignmentId);
        refreshFMWindow();
        if (index < m_vecFMBigImagePos.size())
            m_vecFMBigImagePos.erase(m_vecFMBigImagePos.begin() + index);
    }
}

void FiducialMarkWidget::on_comboBoxChooseImage_indexChanged(int index)
{
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto vecCombinedBigImage = m_pDataCtrl->getCombinedBigImages();
    if (index >= 0 && index < vecCombinedBigImage.size() && !vecCombinedBigImage[index].empty())
        pUI->setImage(vecCombinedBigImage[index]);
}