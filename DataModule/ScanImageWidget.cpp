#include "ScanImageWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include "../include/VisionUI.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "opencv.hpp"
#include "DataStoreAPI.h"
#include "SystemData.h"
#include "DataVariables.h"

using namespace NFG::AOI;
using namespace AOI;

ScanImageWidget::ScanImageWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

ScanImageWidget::~ScanImageWidget()
{
}

void ScanImageWidget::on_btnSelectFrameImages_clicked() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilter(tr("Image Files (*.png *.jpg *.bmp)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    }else
        return;
    ui.lineEditFrameImageFolder->setText(fileNames[0]);
}

cv::Mat ScanImageWidget::combineImage(const QString &strInputFolder)
{
    Vision::PR_COMBINE_IMG_CMD stCmd;
    Vision::PR_COMBINE_IMG_RPY stRpy;
    stCmd.nCountOfImgPerFrame = ui.lineEditOneFrameImageCount->text().toInt();
    stCmd.nCountOfFrameX = ui.lineEditFrameCountX->text().toInt();
    stCmd.nCountOfFrameY = ui.lineEditFrameCountY->text().toInt();
    stCmd.nOverlapX = ui.lineEditOverlapX->text().toInt();
    stCmd.nOverlapY = ui.lineEditOverlapY->text().toInt();
    stCmd.nCountOfImgPerRow = ui.lineEditRowImageCount->text().toInt();

    const std::string strFolder = strInputFolder.toStdString() + "/";

    for (int nRow = 0; nRow < stCmd.nCountOfFrameY; ++ nRow)
    //Column start from right to left.
    for (int nCol = 0; nCol < stCmd.nCountOfFrameX; ++ nCol)
    for (int imgNo = 1; imgNo <= stCmd.nCountOfImgPerFrame; ++ imgNo)
    {
        char arrCharFileName[100];
        int nImageIndex = nCol * stCmd.nCountOfImgPerFrame + nRow * stCmd.nCountOfImgPerRow + imgNo;
        _snprintf(arrCharFileName, sizeof(arrCharFileName), "F%d-%d-1.bmp", nRow + 1, nImageIndex);
        std::string strImagePath = strFolder + std::string ( arrCharFileName );
        cv::Mat mat = cv::imread(strImagePath, cv::IMREAD_GRAYSCALE);
        if ( mat.empty() )
            continue;
        cv::Mat matColor;
        cv::cvtColor(mat, matColor, CV_BayerGR2BGR);
        stCmd.vecInputImages.push_back( matColor );
    }

    Vision::PR_CombineImg ( &stCmd, &stRpy );
    if ( stRpy.enStatus != Vision::VisionStatus::OK ) {
        Vision::PR_GET_ERROR_INFO_RPY stErrorInfo;
        Vision::PR_GetErrorInfo ( stRpy.enStatus, &stErrorInfo );
        QString strErrormsg = QString ( "Failed to combine images, error message: ") + stErrorInfo.achErrorStr;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), strErrormsg, QStringLiteral("Quit"));
        return cv::Mat();
    }

    double dScale = ui.lineEditCombinedImageZoomFactor->text().toFloat();
    cv::Mat matResize;
    cv::resize ( stRpy.vecResultImages[0], matResize, cv::Size(), dScale, dScale );
    return matResize;
}

void ScanImageWidget::on_btnCombineLoadImage_clicked() {
    auto strFolder = ui.lineEditFrameImageFolder->text();
    auto matImage = combineImage ( strFolder );
    if ( matImage.empty() )
        return;

    updateImageDeviceWindows ( matImage );
}

void ScanImageWidget::on_btnSelectCombinedImage_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilter(tr("Image Files (*.png *.jpg *.bmp)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    }else
        return;

    std::string strFrameImageFolder = ui.lineEditFrameImageFolder->text().toStdString();
    DataVariablesInstance->setFrameImageFolder ( strFrameImageFolder );
    int nCountOfImgPerFrame = ui.lineEditOneFrameImageCount->text().toInt();
    DataVariablesInstance->setCountOfImgPerFrame ( nCountOfImgPerFrame );
    int nCountOfFrameX = ui.lineEditFrameCountX->text().toInt();
    DataVariablesInstance->setFrameCountX ( nCountOfFrameX );
    int nCountOfFrameY = ui.lineEditFrameCountY->text().toInt();
    DataVariablesInstance->setFrameCountY ( nCountOfFrameY );
    int nOverlapX = ui.lineEditOverlapX->text().toInt();
    DataVariablesInstance->setOverlapX ( nOverlapX );
    int nOverlapY = ui.lineEditOverlapY->text().toInt();
    DataVariablesInstance->setOverlapY ( nOverlapY );
    int nCountOfImgPerRow = ui.lineEditRowImageCount->text().toInt();
    DataVariablesInstance->setCountOfImgPerRow ( nCountOfImgPerRow );
    float fCombinedImgScale = ui.lineEditCombinedImageZoomFactor->text().toFloat();
    DataVariablesInstance->setCombinedImgScale ( fCombinedImgScale );

    auto matImage = cv::imread ( fileNames[0].toStdString() );
    updateImageDeviceWindows ( matImage );
}

void ScanImageWidget::updateImageDeviceWindows(const cv::Mat &matImage)
{
    bool bBoardRotated = ui.checkBoxBoardRotated->isChecked();
    double dResolutionX = ui.lineEditResolutionX->text().toDouble();
    double dResolutionY = ui.lineEditResolutionY->text().toDouble();
    float fCombinedImgScale = ui.lineEditCombinedImageZoomFactor->text().toFloat();
    QVector<cv::RotatedRect> vecDeviceWindows;

    Engine::BoardVector vecBoard;
    auto result = Engine::GetAllBoards ( vecBoard );
    if ( Engine::OK != result ) {
        std::string errorType, errorMessage;
        Engine::GetErrorDetail ( errorType, errorMessage );
        errorMessage = "Failed to get board from project, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    for ( const auto &board : vecBoard ) {
        Engine::DeviceVector vecDevice;
        result = Engine::GetBoardDevice ( board.Id, vecDevice );
        if ( Engine::OK != result ) {
            std::string errorType, errorMessage;
            Engine::GetErrorDetail ( errorType, errorMessage );
            errorMessage = "Failed to get devices from project, error message " + errorMessage;
            QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
            return;
        }

        for ( const auto &device : vecDevice ) {
            if ( device.isBottom )
                continue;

            auto x = ( device.x + board.x ) / dResolutionX * fCombinedImgScale;
            auto y = ( device.y + board.y ) / dResolutionY * fCombinedImgScale;
            if ( bBoardRotated )
                x = matImage.cols - x;
            else
                y = matImage.rows - y; //In cad, up is positive, but in image, down is positive.
            auto width  = device.width  / dResolutionX * fCombinedImgScale;
            auto height = device.height / dResolutionY * fCombinedImgScale;
            cv::RotatedRect deviceWindow ( cv::Point2f(x, y), cv::Size2f(width, height), device.angle );
            vecDeviceWindows.push_back ( deviceWindow );
        }
    }

    System->setSysParam("CAM_RESOLUTION_X", dResolutionX );
	System->setSysParam("CAM_RESOLUTION_Y", dResolutionY );
    System->setSysParam("BOARD_ROTATED", bBoardRotated );

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SET_FIDUCIAL_MARK);
    pUI->setImage ( matImage, true );
    pUI->setDeviceWindows ( vecDeviceWindows );
}