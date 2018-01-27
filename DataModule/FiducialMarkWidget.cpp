#include <QMessageBox>
#include "VisionAPI.h"
#include "FiducialMarkWidget.h"
#include "../include/VisionUI.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "DataUtils.h"
#include "DataVariables.h"
#include "SetFiducialMarkDialog.h"

using namespace AOI;
using namespace NFG::AOI;

FiducialMarkWidget::FiducialMarkWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

FiducialMarkWidget::~FiducialMarkWidget()
{
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
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    cv::RotatedRect rrectCadWindow, rrectImageWindow;
    pUI->getSelectDeviceWindow ( rrectCadWindow, rrectImageWindow );
    rrectCadWindow.center.x /= DataVariablesInstance->getCombinedImgScale();
    rrectCadWindow.center.y /= DataVariablesInstance->getCombinedImgScale();
    rrectCadWindow.size.width  /= DataVariablesInstance->getCombinedImgScale();
    rrectCadWindow.size.height /= DataVariablesInstance->getCombinedImgScale();

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
    int nBigImgWidth  = matImage.cols / DataVariablesInstance->getCombinedImgScale();
    int nBigImgHeight = matImage.rows / DataVariablesInstance->getCombinedImgScale();
    auto matFirstImg = cv::imread ( DataVariablesInstance->getFrameImageFolder() + "/F1-1-1.bmp" );
    if ( matFirstImg.empty() ) {
        QString strMsg = QString ( "Failed to read frame image from " ) + DataVariablesInstance->getFrameImageFolder().c_str();
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), strMsg, QStringLiteral("Quit"));
        return;
    }

    int nSelectPtX = rrectImageWindow.center.x / DataVariablesInstance->getCombinedImgScale();
    int nSelectPtY = rrectImageWindow.center.y / DataVariablesInstance->getCombinedImgScale();

    int nFrameX, nFrameY, nPtInFrameX, nPtInFrameY;
    DataUtils::getFrameFromCombinedImage ( nBigImgWidth, nBigImgHeight, matFirstImg.cols, matFirstImg.rows, 
        DataVariablesInstance->getOverlapX(), DataVariablesInstance->getOverlapY(), 
        nSelectPtX, nSelectPtY, nFrameX, nFrameY, nPtInFrameX, nPtInFrameY );

    char arrCharFileName[100];
    int nImageIndex = nFrameX * DataVariablesInstance->getCountOfImgPerFrame() + nFrameY * DataVariablesInstance->getCountOfImgPerRow() + 1;
    _snprintf ( arrCharFileName, sizeof(arrCharFileName), "/F%d-%d-1.bmp", nFrameY + 1, nImageIndex );

    auto strFrameImagePath = DataVariablesInstance->getFrameImageFolder() + arrCharFileName;
    auto matFrameImg = cv::imread ( strFrameImagePath, cv::IMREAD_GRAYSCALE );
    if ( matFrameImg.empty() ) {
        QString strMsg = QString ( "Failed to read frame image " ) + strFrameImagePath.c_str();
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
    if ( iReturn != QDialog::Accepted ) {
        return;
    }

    double dResolutionX, dResolutionY;
    bool bBoardRotated = false;
    dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();

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
            stCmd.rectSrchWindow.width = stCmd.matInputImg.cols - stCmd.rectSrchWindow.x;
        if ( ( stCmd.rectSrchWindow.y + stCmd.rectSrchWindow.height ) > stCmd.matInputImg.rows )
            stCmd.rectSrchWindow.height = stCmd.matInputImg.rows - stCmd.rectSrchWindow.y;

        PR_SrchFiducialMark ( &stCmd, &stRpy );
        if ( stRpy.enStatus != Vision::VisionStatus::OK ) {
            Vision::PR_GET_ERROR_INFO_RPY stErrStrRpy;
            Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrStrRpy);    
            QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), stErrStrRpy.achErrorStr, QStringLiteral("Quit"));
            return;
        }

        int nPosInCombineImageX, nPosInCombineImageY;
        DataUtils::getCombinedImagePosFromFramePos ( nBigImgWidth, nBigImgHeight, matFirstImg.cols, matFirstImg.rows,
            DataVariablesInstance->getOverlapX(), DataVariablesInstance->getOverlapY(),
            nFrameX, nFrameY, stRpy.ptPos.x, stRpy.ptPos.y, nPosInCombineImageX, nPosInCombineImageY );

        rrectImageWindow.center.x = nPosInCombineImageX;
        rrectImageWindow.center.y = nPosInCombineImageY;
        m_vecFMImageWindow.push_back ( rrectImageWindow );

        m_vecFMCadWindow.push_back ( rrectCadWindow );        
        ui.lineEditFiducialMark->setText( QString::number ( m_vecFMCadWindow.size() ) );

        auto vecTmpVecFMCadWindow ( m_vecFMCadWindow );
        for ( auto &rrFMCadWindow : vecTmpVecFMCadWindow ) {
            rrFMCadWindow.center.x *= DataVariablesInstance->getCombinedImgScale();
            rrFMCadWindow.center.y *= DataVariablesInstance->getCombinedImgScale();
            rrFMCadWindow.size.width  *= DataVariablesInstance->getCombinedImgScale();
            rrFMCadWindow.size.height *= DataVariablesInstance->getCombinedImgScale();
        }
        pUI->setSelectedFM ( vecTmpVecFMCadWindow );

        Engine::Alignment alignment;
        if ( bBoardRotated ) {
            alignment.tmplPosX =  ( nBigImgWidth - nPosInCombineImageX ) * dResolutionX;
            alignment.tmplPosY = nPosInCombineImageY * dResolutionY;
        }else {
            alignment.tmplPosX = nPosInCombineImageX * dResolutionX;
            alignment.tmplPosY = ( nBigImgHeight - nPosInCombineImageY ) * dResolutionY;
        }

        alignment.tmplWidth  = dialogSetFM.getFiducialMarkSize() * MM_TO_UM;
        alignment.tmplHeight = dialogSetFM.getFiducialMarkSize() * MM_TO_UM;
        alignment.srchWinWidth = 4.f * alignment.tmplWidth;
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
    }
}

void FiducialMarkWidget::on_btnDoAlignment_clicked()
{
    if ( m_vecFMCadWindow.size() < 2 ) {
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), QStringLiteral("Please select at least 2 fiducial mark to do alignment."), QStringLiteral("Quit"));
        return;
    }

    double dResolutionX, dResolutionY;
    bool bBoardRotated = false;
    dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    auto matImage = pUI->getImage();
    cv::Size sizeOriginalCombineImage;
    sizeOriginalCombineImage.width  = matImage.size().width  / DataVariablesInstance->getCombinedImgScale();
    sizeOriginalCombineImage.height = matImage.size().height / DataVariablesInstance->getCombinedImgScale();

    float fRotationInRadian, Tx, Ty, fScale;
    cv::Mat matTransform;
    if ( m_vecFMCadWindow.size() >= 3 ) {
        Vision::VectorOfPoint2f vecPointCad, vecPointImage;
        for ( const auto &rotatedRect : m_vecFMCadWindow )
            vecPointCad.push_back ( rotatedRect.center );
        for ( const auto &rotatedRect : m_vecFMImageWindow )
            vecPointImage.push_back ( rotatedRect.center );

        if ( m_vecFMCadWindow.size() == 3 )
            matTransform = cv::getAffineTransform ( vecPointCad, vecPointImage );
        else
            matTransform = cv::estimateRigidTransform ( vecPointCad, vecPointImage, true );
        
        matTransform.convertTo ( matTransform, CV_32FC1 );

        m_fCadOffsetX = matTransform.at<float>( 0, 2 ) * dResolutionX;
        m_fCadOffsetY = matTransform.at<float>( 1, 2 ) * dResolutionY;
    }else {
        DataUtils::alignWithTwoPoints ( m_vecFMCadWindow[0].center,
            m_vecFMCadWindow[1].center,
            m_vecFMImageWindow[0].center,
            m_vecFMImageWindow[1].center,
            fRotationInRadian, Tx, Ty, fScale );

        m_fCadOffsetX = Tx * dResolutionX;
        m_fCadOffsetY = Ty * dResolutionY;

        //cv::Point2f ptCtr( m_vecFMCadWindow[0].center.x / 2.f + m_vecFMCadWindow[0].center.x / 2.f,  m_vecFMCadWindow[1].center.y / 2.f + m_vecFMCadWindow[1].center.y / 2.f );
        cv::Point2f ptCtr( sizeOriginalCombineImage.width / 2.f, sizeOriginalCombineImage.height / 2.f );
        double fDegree = fRotationInRadian * 180. / CV_PI;
        matTransform = cv::getRotationMatrix2D ( ptCtr, fDegree, fScale );
        matTransform.at<double>(0, 2) += Tx;
        matTransform.at<double>(1, 2) += Ty;
        matTransform.convertTo ( matTransform, CV_32FC1 );
    }

    Engine::BoardVector vecBoard;
    auto result = Engine::GetAllBoards ( vecBoard );
    if ( Engine::OK != result ) {
        std::string errorType, errorMessage;
        Engine::GetErrorDetail ( errorType, errorMessage );
        errorMessage = "Failed to get board from project, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }
    QVector<cv::RotatedRect> vecDeviceWindows;

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
                x = sizeOriginalCombineImage.width  - x;
            else
                y = sizeOriginalCombineImage.height - y; //In cad, up is positive, but in image, down is positive.        
        
            std::vector<float> vecSrcPos;
            vecSrcPos.push_back ( x );
            vecSrcPos.push_back ( y );
            vecSrcPos.push_back ( 1 );
            cv::Mat matSrcPos ( vecSrcPos );
            cv::Mat matDestPos = matTransform * matSrcPos;
            x = matDestPos.at<float>(0) * DataVariablesInstance->getCombinedImgScale();
            y = matDestPos.at<float>(1) * DataVariablesInstance->getCombinedImgScale();

            auto width  = device.width  / dResolutionX * DataVariablesInstance->getCombinedImgScale();
            auto height = device.height / dResolutionY * DataVariablesInstance->getCombinedImgScale();
            cv::RotatedRect deviceWindow ( cv::Point2f(x, y), cv::Size2f(width, height), device.angle );
            vecDeviceWindows.push_back ( deviceWindow );
        }
    }
    pUI->setDeviceWindows ( vecDeviceWindows );

    //Update the offset to board.
    for ( auto &board : vecBoard ) {
        if ( bBoardRotated ) {
            board.x -= m_fCadOffsetX;
            board.y += m_fCadOffsetY;
        }else {
            board.x += m_fCadOffsetX;
            board.y -= m_fCadOffsetY;
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
}