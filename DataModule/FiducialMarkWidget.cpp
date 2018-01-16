#include <QMessageBox>
#include "VisionAPI.h"
#include "FiducialMarkWidget.h"
#include "../include/VisionUI.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"

using namespace AOI::Vision;
using namespace NFG::AOI;

FiducialMarkWidget::FiducialMarkWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

FiducialMarkWidget::~FiducialMarkWidget()
{
}

void FiducialMarkWidget::on_btnAddFiducialMark_clicked()
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
    pUI->getSelectDeviceWindow( rrectCadWindow, rrectImageWindow );
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

    PR_SRCH_FIDUCIAL_MARK_CMD stCmd;
    PR_SRCH_FIDUCIAL_MARK_RPY stRpy;
    stCmd.enType = PR_FIDUCIAL_MARK_TYPE::SQUARE;
    stCmd.fSize = rrectImageWindow.size.width;
    stCmd.fSize = 10;
    stCmd.rectSrchWindow = cv::Rect ( rrectImageWindow.center.x - rrectImageWindow.size.width, rrectImageWindow.center.y - rrectImageWindow.size.height,
        rrectImageWindow.size.width * 2, rrectImageWindow.size.height * 2 );
    stCmd.matInputImg = pUI->getImage();

    PR_SrchFiducialMark ( &stCmd, &stRpy );
    if ( stRpy.enStatus != VisionStatus::OK ) {
        PR_GET_ERROR_INFO_RPY stErrStrRpy;
        PR_GetErrorInfo(stRpy.enStatus, &stErrStrRpy);    
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), stErrStrRpy.achErrorStr, QStringLiteral("Quit"));
        return;
    }

    m_vecFMCadWindow.push_back ( rrectCadWindow );
    m_vecFMImageWindow.push_back ( rrectImageWindow );
    ui.lineEditFiducialMark->setText( QString::number ( m_vecFMCadWindow.size() ) );
    pUI->setSelectedFM ( m_vecFMCadWindow );
}

void FiducialMarkWidget::on_btnDoAlignment_clicked()
{
    if ( m_vecFMCadWindow.size() != 3 ) {
        QMessageBox::critical(nullptr, QStringLiteral("Fiducial Mark"), QStringLiteral("Please select 3 fiducial mark to do alignment."), QStringLiteral("Quit"));
        return;
    }
    VectorOfPoint2f vecPointCad, vecPointImage;
    for ( const auto &rotatedRect : m_vecFMCadWindow )
        vecPointCad.push_back ( rotatedRect.center );
    for ( const auto &rotatedRect : m_vecFMImageWindow )
        vecPointImage.push_back ( rotatedRect.center );

    auto matAffine = cv::getAffineTransform ( vecPointCad, vecPointImage );
    matAffine.convertTo ( matAffine, CV_32FC1 );

    Engine::DeviceVector vecDevice;
    auto result = Engine::GetAllDevices ( vecDevice );
    if ( Engine::OK != result ) {
        String errorType, errorMessage;
        Engine::GetErrorDetail ( errorType, errorMessage );
        errorMessage = "Failed to get devices from project, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    double dResolutionX, dResolutionY;
    bool bBoardRotated = false;
    dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    auto matImage = pUI->getImage();

    QVector<cv::RotatedRect> vecDeviceWindows;
    for ( const auto &device : vecDevice ) {
        auto x = device.x / dResolutionX;
        auto y = device.y / dResolutionY;
        if ( bBoardRotated )
            x = matImage.cols - x;
        else
            y = matImage.rows - y; //In cad, up is positive, but in image, down is positive.
        auto width  = device.width  / dResolutionX;
        auto height = device.height / dResolutionY;
        
        std::vector<float> vecSrcPos;
        vecSrcPos.push_back ( x );
        vecSrcPos.push_back ( y );
        vecSrcPos.push_back ( 1 );
        cv::Mat matSrcPos ( vecSrcPos );
        cv::Mat matDestPos = matAffine * matSrcPos;
        x = matDestPos.at<float>(0);
        y = matDestPos.at<float>(1);

        cv::RotatedRect deviceWindow ( cv::Point2f(x, y), cv::Size2f(width, height), device.angle );
        vecDeviceWindows.push_back ( deviceWindow );
    }
    pUI->setDeviceWindows ( vecDeviceWindows );
}