#include <QMessageBox>
#include "VisionAPI.h"
#include "FiducialMarkWidget.h"
#include "../include/IVisionUI.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../Common/SystemData.h"
#include "DataUtils.h"
#include "SetFiducialMarkDialog.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/video.hpp"

using namespace NFG::AOI;

FiducialMarkWidget::FiducialMarkWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

FiducialMarkWidget::~FiducialMarkWidget()
{
}

void FiducialMarkWidget::showEvent(QShowEvent *event)
{
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    auto matImage = pUI->getImage();
    m_nBigImageWidth  = matImage.cols / dCombinedImageScale;
    m_nBigImageHeight = matImage.rows / dCombinedImageScale;

    m_vecFMBigImagePos.clear();
    refreshFMWindow();
}

void FiducialMarkWidget::on_btnSelectFiducialMark_clicked()
{
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SET_FIDUCIAL_MARK);
}

static std::vector<cv::Point> getCornerOfRotatedRect (const cv::RotatedRect &rotatedRect)
{
    std::vector<cv::Point> vecPoint;
    cv::Point2f arrPt[4];
    rotatedRect.points(arrPt);
    for (int i = 0; i < 4; ++i)
        vecPoint.push_back(arrPt[i]);
    return vecPoint;
}

void FiducialMarkWidget::on_btnConfirmFiducialMark_clicked()
{
    auto nCountOfImgPerFrame = System->getParam("scan_image_OneFrameImageCount").toInt();
	auto nCountOfFrameX = System->getParam("scan_image_FrameCountX").toInt();
    auto nCountOfFrameY = System->getParam("scan_image_FrameCountY").toInt();
	auto dOverlapUmX = System->getParam("scan_image_OverlapX").toDouble();
    auto dOverlapUmY = System->getParam("scan_image_OverlapY").toDouble();
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();

    int nOverlapX = static_cast<int> ( dOverlapUmX / dResolutionX + 0.5 );
    int nOverlapY = static_cast<int> ( dOverlapUmY / dResolutionY + 0.5 );

    auto nCountOfImgPerRow = System->getParam("scan_image_RowImageCount").toInt();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();
    auto nScanDirection = System->getParam("scan_image_Direction").toInt();
    auto strImageFolder = System->getParam("scan_image_Folder").toString();

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    cv::RotatedRect rrectCadWindow, rrectImageWindow;
    pUI->getSelectDeviceWindow ( rrectCadWindow, rrectImageWindow );
    rrectCadWindow.center.x /= dCombinedImageScale;
    rrectCadWindow.center.y /= dCombinedImageScale;
    rrectCadWindow.size.width  /= dCombinedImageScale;
    rrectCadWindow.size.height /= dCombinedImageScale;

    bool bFound = false;
    for ( const auto &rotatedRect : m_vecFMCadWindow ) {
        auto contour = getCornerOfRotatedRect ( rotatedRect );
        contour.push_back ( contour.front() );
        auto distance = cv::pointPolygonTest ( contour, rrectCadWindow.center, false );
        if ( distance >= 0 ) {
            bFound = true;
            break;
        }
    }

    if ( bFound ) {
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), QStringLiteral("This ficucial mark already added"), QStringLiteral("Quit"));
        return;
    }

    auto matImage = pUI->getImage();
    int nBigImgWidth  = matImage.cols / dCombinedImageScale;
    int nBigImgHeight = matImage.rows / dCombinedImageScale;
    auto matFirstImg = cv::imread ( strImageFolder.toStdString() + "/F1-1-1.bmp" );
    if ( matFirstImg.empty() ) {
        QString strMsg = QString ( "Failed to read frame image from " ) + strImageFolder;
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), strMsg, QStringLiteral("Quit"));
        return;
    }

    int nSelectPtX = rrectImageWindow.center.x / dCombinedImageScale;
    int nSelectPtY = rrectImageWindow.center.y / dCombinedImageScale;

    int nFrameX, nFrameY, nPtInFrameX, nPtInFrameY;
    DataUtils::getFrameFromCombinedImage ( nBigImgWidth, nBigImgHeight, matFirstImg.cols, matFirstImg.rows, 
        nOverlapX, nOverlapY, nSelectPtX, nSelectPtY, nFrameX, nFrameY, nPtInFrameX, nPtInFrameY,
        static_cast<Vision::PR_SCAN_IMAGE_DIR> ( nScanDirection ) );

    char arrCharFileName[100];
    int nImageIndex = nFrameX * nCountOfImgPerFrame + nFrameY * nCountOfImgPerRow + 1;
    _snprintf ( arrCharFileName, sizeof(arrCharFileName), "/F%d-%d-1.bmp", nFrameY + 1, nImageIndex );

    auto strFrameImagePath = strImageFolder + arrCharFileName;
    auto matFrameImg = cv::imread ( strFrameImagePath.toStdString(), cv::IMREAD_GRAYSCALE );
    if ( matFrameImg.empty() ) {
        QString strMsg = QString ( "Failed to read frame image " ) + strFrameImagePath;
        QMessageBox::critical ( nullptr, QStringLiteral("Fiducial Mark"), strMsg, QStringLiteral("Quit") );
        return;
    }
    cv::cvtColor ( matFrameImg, matFrameImg, CV_BayerGR2BGR );

    SetFiducialMarkDialog dialogSetFM;
    dialogSetFM.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialogSetFM.show();
    dialogSetFM.raise();
    dialogSetFM.activateWindow();
    int iReturn = dialogSetFM.exec();

    if (iReturn != QDialog::Accepted) {
        return;
    }

    bool bUseStandardShape = dialogSetFM.getUseStandardShape();
    if ( bUseStandardShape ) {
        Vision::PR_SRCH_FIDUCIAL_MARK_CMD stCmd;
        Vision::PR_SRCH_FIDUCIAL_MARK_RPY stRpy;
        stCmd.matInputImg = matFrameImg;
        stCmd.enType = dialogSetFM.getFiducialMarkShape();
        stCmd.fSize = dialogSetFM.getFiducialMarkSize() * MM_TO_UM / System->getSysParam("CAM_RESOLUTION_X").toDouble();
        stCmd.fMargin = stCmd.fSize / 2.f;
        stCmd.rectSrchWindow = cv::Rect ( nPtInFrameX - stCmd.fSize * 2, nPtInFrameY - stCmd.fSize * 2,
            stCmd.fSize * 4, stCmd.fSize * 4 );
        if ( stCmd.rectSrchWindow.x < 0 ) stCmd.rectSrchWindow.x = 0;
        if ( stCmd.rectSrchWindow.y < 0 ) stCmd.rectSrchWindow.y = 0;
        if ( ( stCmd.rectSrchWindow.x + stCmd.rectSrchWindow.width ) > stCmd.matInputImg.cols )
            stCmd.rectSrchWindow.width  = stCmd.matInputImg.cols - stCmd.rectSrchWindow.x;
        if ( ( stCmd.rectSrchWindow.y + stCmd.rectSrchWindow.height ) > stCmd.matInputImg.rows )
            stCmd.rectSrchWindow.height = stCmd.matInputImg.rows - stCmd.rectSrchWindow.y;

        PR_SrchFiducialMark ( &stCmd, &stRpy );
        if ( stRpy.enStatus != Vision::VisionStatus::OK ) {
            Vision::PR_GET_ERROR_INFO_RPY stErrStrRpy;
            Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrStrRpy);    
            QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), stErrStrRpy.achErrorStr, QStringLiteral("Quit"));
            return;
        }
        QString qstrMsg;
        qstrMsg.sprintf ( "Success to search fiducial mark, match score %f, pos in original image [%f, %f].", stRpy.fMatchScore, stRpy.ptPos.x, stRpy.ptPos.y );
        System->setTrackInfo( qstrMsg );

        int nPosInCombineImageX, nPosInCombineImageY;
        DataUtils::getCombinedImagePosFromFramePos ( nBigImgWidth, nBigImgHeight, matFirstImg.cols, matFirstImg.rows,
            nOverlapX, nOverlapY, nFrameX, nFrameY, stRpy.ptPos.x, stRpy.ptPos.y, nPosInCombineImageX, nPosInCombineImageY,
            static_cast<Vision::PR_SCAN_IMAGE_DIR>(nScanDirection) );

        m_vecFMBigImagePos.push_back ( cv::Point2f ( nPosInCombineImageX, nPosInCombineImageY ) );

        Engine::Alignment alignment;
        if ( bBoardRotated ) {
            alignment.tmplPosX = ( nBigImgWidth - rrectCadWindow.center.x )  * dResolutionX;
            alignment.tmplPosY = rrectCadWindow.center.y * dResolutionY;
        }else {
            alignment.tmplPosX = rrectCadWindow.center.x * dResolutionX;
            alignment.tmplPosY = ( nBigImgHeight - rrectCadWindow.center.y ) * dResolutionY;
        }

        alignment.tmplWidth  = dialogSetFM.getFiducialMarkSize() * MM_TO_UM;
        alignment.tmplHeight = dialogSetFM.getFiducialMarkSize() * MM_TO_UM;
        alignment.srchWinWidth  = 4.f * alignment.tmplWidth;
        alignment.srchWinHeight = 4.f * alignment.tmplHeight;
        alignment.isFM = bUseStandardShape;
        alignment.fmShape = static_cast<int> ( dialogSetFM.getFiducialMarkShape() );
        alignment.isFMDark = dialogSetFM.getIsFiducialMarkDark();
        alignment.vecLight.push_back(1);
        auto result = Engine::CreateAlignment ( alignment );
        if (Engine::OK != result) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            errorMessage = "Failed to create alignment, error message " + errorMessage;
            QMessageBox::critical(nullptr, QStringLiteral("Set Fiducial Mark"), errorMessage.c_str(), QStringLiteral("Quit"));
            return;
        }

        m_vecFMCadWindow.push_back ( rrectCadWindow );
        ui.lineEditFiducialMark->setText( QString::number ( m_vecFMCadWindow.size() ) );

        auto vecTmpVecFMCadWindow ( m_vecFMCadWindow );
        for ( auto &rrFMCadWindow : vecTmpVecFMCadWindow ) {
            rrFMCadWindow.center.x *= dCombinedImageScale;
            rrFMCadWindow.center.y *= dCombinedImageScale;
            rrFMCadWindow.size.width  *= dCombinedImageScale;
            rrFMCadWindow.size.height *= dCombinedImageScale;
        }
        pUI->setSelectedFM ( vecTmpVecFMCadWindow );
    }
}

//Search all the fiducial mark.
int FiducialMarkWidget::srchFiducialMark()
{
    auto nCountOfImgPerFrame = System->getParam("scan_image_OneFrameImageCount").toInt();
    auto nCountOfImgPerRow = System->getParam("scan_image_RowImageCount").toInt();

    auto dOverlapUmX = System->getParam("scan_image_OverlapX").toDouble();
    auto dOverlapUmY = System->getParam("scan_image_OverlapY").toDouble();
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    int nOverlapX = static_cast<int> ( dOverlapUmX / dResolutionX + 0.5 );
    int nOverlapY = static_cast<int> ( dOverlapUmY / dResolutionY + 0.5 );

    auto nScanDirection = System->getParam("scan_image_Direction").toInt();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();
    auto strImageFolder = System->getParam("scan_image_Folder").toString();

    Engine::AlignmentVector vecAlignmentDB;
    auto result = Engine::GetAllAlignments ( vecAlignmentDB );
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get alignment from data base, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Do alignment"), errorMessage.c_str(), QStringLiteral("Quit"));
        return NOK;
    }

    if ( vecAlignmentDB.size() != m_vecFMCadWindow.size() ) {
        QString strMsg;
        strMsg.sprintf("Software error, the DB alignment count %d not match with CAD alignment count %d.",
            vecAlignmentDB.size(), m_vecFMCadWindow.size() );
        QMessageBox::critical(nullptr, QStringLiteral("Do alignment"), strMsg, QStringLiteral("Quit"));
        return NOK;
    }

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    auto matImage = pUI->getImage();
    int nBigImgWidth  = matImage.cols / dCombinedImageScale;
    int nBigImgHeight = matImage.rows / dCombinedImageScale;
    auto matFirstImg = cv::imread( strImageFolder.toStdString() + "/F1-1-1.bmp");
    if (matFirstImg.empty()) {
        QString strMsg = QString("Failed to read frame image from ") + strImageFolder;
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), strMsg, QStringLiteral("Quit"));
        return NOK;
    }

    //Research for the fiducial mark again to do alignment. There is one problem here, the fiducial shape is not remembered.
    for ( size_t i = 0; i < m_vecFMCadWindow.size(); ++ i ) {
        auto rrectCadWindow = m_vecFMCadWindow[i];
        int nSelectPtX = rrectCadWindow.center.x;
        int nSelectPtY = rrectCadWindow.center.y;

        int nFrameX, nFrameY, nPtInFrameX, nPtInFrameY;
        DataUtils::getFrameFromCombinedImage ( nBigImgWidth, nBigImgHeight, matFirstImg.cols, matFirstImg.rows,
            nOverlapX, nOverlapY, nSelectPtX, nSelectPtY, nFrameX, nFrameY, nPtInFrameX, nPtInFrameY,
            static_cast<Vision::PR_SCAN_IMAGE_DIR> ( nScanDirection ) );

        char arrCharFileName[100];
        int nImageIndex = nFrameX * nCountOfImgPerFrame + nFrameY * nCountOfImgPerRow + 1;
        _snprintf(arrCharFileName, sizeof(arrCharFileName), "/F%d-%d-1.bmp", nFrameY + 1, nImageIndex);

        auto strFrameImagePath = strImageFolder + arrCharFileName;
        auto matFrameImg = cv::imread ( strFrameImagePath.toStdString(), cv::IMREAD_GRAYSCALE);
        if (matFrameImg.empty()) {
            QString strMsg = QString("Failed to read frame image ") + strFrameImagePath;
            QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), strMsg, QStringLiteral("Quit"));
            return NOK;
        }
        cv::cvtColor(matFrameImg, matFrameImg, CV_BayerGR2BGR);

        Vision::PR_SRCH_FIDUCIAL_MARK_CMD stCmd;
        Vision::PR_SRCH_FIDUCIAL_MARK_RPY stRpy;
        stCmd.matInputImg = matFrameImg;
        stCmd.enType = static_cast<Vision::PR_FIDUCIAL_MARK_TYPE> ( vecAlignmentDB[i].fmShape );
        stCmd.fSize = vecAlignmentDB[i].tmplWidth / System->getSysParam("CAM_RESOLUTION_X").toDouble();
        stCmd.fMargin = stCmd.fSize / 2.f;
        int nSrchWinWidth  = vecAlignmentDB[i].srchWinWidth  / System->getSysParam("CAM_RESOLUTION_X").toDouble();
        int nSrchWinHeight = vecAlignmentDB[i].srchWinHeight / System->getSysParam("CAM_RESOLUTION_X").toDouble();
        stCmd.rectSrchWindow = cv::Rect ( nPtInFrameX - nSrchWinWidth / 2, nPtInFrameY - nSrchWinHeight / 2,
            nSrchWinWidth, nSrchWinHeight );
        if ( stCmd.rectSrchWindow.x < 0 ) stCmd.rectSrchWindow.x = 0;
        if ( stCmd.rectSrchWindow.y < 0 ) stCmd.rectSrchWindow.y = 0;
        if ( ( stCmd.rectSrchWindow.x + stCmd.rectSrchWindow.width ) > stCmd.matInputImg.cols )
            stCmd.rectSrchWindow.width = stCmd.matInputImg.cols - stCmd.rectSrchWindow.x;
        if ( ( stCmd.rectSrchWindow.y + stCmd.rectSrchWindow.height ) > stCmd.matInputImg.rows )
            stCmd.rectSrchWindow.height = stCmd.matInputImg.rows - stCmd.rectSrchWindow.y;

        PR_SrchFiducialMark ( &stCmd, &stRpy );
        if ( stRpy.enStatus != Vision::VisionStatus::OK ) {
            Vision::PR_GET_ERROR_INFO_RPY stErrStrRpy;
            Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrStrRpy);    
            QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), stErrStrRpy.achErrorStr, QStringLiteral("Quit"));
            return NOK;
        }
        QString qstrMsg;
        qstrMsg.sprintf ( "Success to search fiducial mark, match score %f, pos in original image [%f, %f].", stRpy.fMatchScore, stRpy.ptPos.x, stRpy.ptPos.y );
        System->setTrackInfo( qstrMsg );

        int nPosInCombineImageX, nPosInCombineImageY;
        DataUtils::getCombinedImagePosFromFramePos ( nBigImgWidth, nBigImgHeight, matFirstImg.cols, matFirstImg.rows,
            nOverlapX, nOverlapY, nFrameX, nFrameY, stRpy.ptPos.x, stRpy.ptPos.y, nPosInCombineImageX, nPosInCombineImageY,
            static_cast<Vision::PR_SCAN_IMAGE_DIR> ( nScanDirection ) );
        m_vecFMBigImagePos.push_back ( cv::Point ( nPosInCombineImageX, nPosInCombineImageY ) );
    }
    return OK;
}

int FiducialMarkWidget::refreshFMWindow()
{
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    Engine::AlignmentVector vecAlignment;
    auto result = Engine::GetAllAlignments ( vecAlignment );
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
    for ( const auto &alignment : vecAlignment ) {
        auto x = alignment.tmplPosX / dResolutionX;
        auto y = alignment.tmplPosY / dResolutionY;
        if (bBoardRotated)
            x = m_nBigImageWidth  - x;
        else
            y = m_nBigImageHeight - y; //In cad, up is positive, but in image, down is positive.
        auto width  = alignment.tmplWidth  / dResolutionX;
        auto height = alignment.tmplHeight / dResolutionY;
        cv::RotatedRect alignmentWindow ( cv::Point2f(x, y), cv::Size2f(width, height), 0 );
        m_vecFMCadWindow.push_back ( alignmentWindow );
    }
    ui.lineEditFiducialMark->setText( QString::number ( m_vecFMCadWindow.size() ) );

    auto vecTmpVecFMCadWindow(m_vecFMCadWindow);
    for (auto &rrFMCadWindow : vecTmpVecFMCadWindow) {
        rrFMCadWindow.center.x *= dCombinedImageScale;
        rrFMCadWindow.center.y *= dCombinedImageScale;
        rrFMCadWindow.size.width  *= dCombinedImageScale;
        rrFMCadWindow.size.height *= dCombinedImageScale;
    }
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setSelectedFM(vecTmpVecFMCadWindow);
    return OK;
}

void FiducialMarkWidget::on_btnDoAlignment_clicked()
{
    if ( m_vecFMCadWindow.size() < 2 ) {
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), QStringLiteral("Please select at least 2 fiducial mark to do alignment."), QStringLiteral("Quit"));
        return;
    }

    if ( m_vecFMBigImagePos.size() < m_vecFMCadWindow.size() ) {
        m_vecFMBigImagePos.clear();
        if ( srchFiducialMark() != OK )
            return;
    } 

    double dResolutionX, dResolutionY;
    bool bBoardRotated = false;
    dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    float fRotationInRadian, Tx, Ty, fScale;
    cv::Mat matTransform;
    if ( m_vecFMCadWindow.size() >= 3 ) {
        Vision::VectorOfPoint2f vecPointCad;
        for ( const auto &rotatedRect : m_vecFMCadWindow )
            vecPointCad.push_back ( rotatedRect.center );

        if ( m_vecFMCadWindow.size() == 3 )
            matTransform = cv::getAffineTransform ( vecPointCad, m_vecFMBigImagePos );
        else
            matTransform = cv::estimateRigidTransform ( vecPointCad, m_vecFMBigImagePos, true );
        
        matTransform.convertTo ( matTransform, CV_32FC1 );

        m_fCadOffsetX = matTransform.at<float>( 0, 2 ) * dResolutionX;
        m_fCadOffsetY = matTransform.at<float>( 1, 2 ) * dResolutionY;
    }else {
        DataUtils::alignWithTwoPoints ( m_vecFMCadWindow[0].center,
            m_vecFMCadWindow[1].center,
            m_vecFMBigImagePos[0],
            m_vecFMBigImagePos[1],
            fRotationInRadian, Tx, Ty, fScale );

        m_fCadOffsetX = Tx * dResolutionX;
        m_fCadOffsetY = Ty * dResolutionY;

        cv::Point2f ptCtr( m_vecFMCadWindow[0].center.x / 2.f + m_vecFMCadWindow[1].center.x / 2.f,  m_vecFMCadWindow[0].center.y / 2.f + m_vecFMCadWindow[1].center.y / 2.f );
        //cv::Point2f ptCtr( m_nBigImageWidth / 2.f, m_nBigImageHeight / 2.f );
        double fDegree = fRotationInRadian * 180. / CV_PI;
        matTransform = cv::getRotationMatrix2D ( ptCtr, fDegree, fScale );
        matTransform.at<double>(0, 2) += Tx;
        matTransform.at<double>(1, 2) += Ty;
        matTransform.convertTo ( matTransform, CV_32FC1 );
    }

    auto vecVecTransform = DataUtils::matToVector<float> ( matTransform );

    Engine::BoardVector vecBoard;
    auto result = Engine::GetAllBoards ( vecBoard );
    if ( Engine::OK != result ) {
        String errorType, errorMessage;
        Engine::GetErrorDetail ( errorType, errorMessage );
        errorMessage = "Failed to get board from project, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    VisionViewDeviceVector vecVisionViewDevices;
    for ( const auto &board : vecBoard ) {
        Engine::DeviceVector vecDevice;
        auto result = Engine::GetBoardDevice ( board.Id, vecDevice );
        if ( Engine::OK != result ) {
            String errorType, errorMessage;
            Engine::GetErrorDetail ( errorType, errorMessage );
            errorMessage = "Failed to get devices from project, error message " + errorMessage;
            QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
            return;
        }

        for ( const auto &device : vecDevice ) {
            if ( device.isBottom )
                continue;

            auto x = ( device.x + board.x ) / dResolutionX;
            auto y = ( device.y + board.y ) / dResolutionY;
            if ( bBoardRotated )
                x = m_nBigImageWidth  - x;
            else
                y = m_nBigImageHeight - y; //In cad, up is positive, but in image, down is positive.        
        
            std::vector<float> vecSrcPos;
            vecSrcPos.push_back ( x );
            vecSrcPos.push_back ( y );
            vecSrcPos.push_back ( 1 );
            cv::Mat matSrcPos ( vecSrcPos );
            cv::Mat matDestPos = matTransform * matSrcPos;
            x = matDestPos.at<float>(0) * dCombinedImageScale;
            y = matDestPos.at<float>(1) * dCombinedImageScale;

            auto width  = device.width  / dResolutionX * dCombinedImageScale;
            auto height = device.height / dResolutionY * dCombinedImageScale;
            cv::RotatedRect deviceWindow ( cv::Point2f(x, y), cv::Size2f(width, height), device.angle );
            vecVisionViewDevices.emplace_back ( device.Id, device.name, deviceWindow );
        }
    }
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setDeviceWindows ( vecVisionViewDevices );

    //Update the offset to board.
    for ( auto &board : vecBoard ) {
        if ( bBoardRotated ) {
            board.x += -m_fCadOffsetX;
            board.y +=  m_fCadOffsetY;
        }else {
            board.x +=  m_fCadOffsetX;
            board.y += -m_fCadOffsetY;
        }

        result = Engine::UpdateBoard ( board );
        if (Engine::OK != result) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            errorMessage = "Failed to update board, error message " + errorMessage;
            QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
            return;
        }
    }

    Engine::AlignmentVector vecAlignment;
    Engine::GetAllAlignments ( vecAlignment );
    for ( auto &alignment : vecAlignment ) {
        if ( bBoardRotated ) {
            alignment.tmplPosX += -m_fCadOffsetX;
            alignment.tmplPosY +=  m_fCadOffsetY;
        }else {
            alignment.tmplPosX +=  m_fCadOffsetX;
            alignment.tmplPosY += -m_fCadOffsetY;
        }
        Engine::UpdateAlignment ( alignment );
    }

    //Refresh the select FM window.
    refreshFMWindow();
}