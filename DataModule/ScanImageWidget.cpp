#include "ScanImageWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include "../include/IVisionUI.h"
#include "../include/IMotion.h"
#include "../include/ICamera.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "opencv.hpp"
#include "DataStoreAPI.h"
#include "SystemData.h"

using namespace NFG::AOI;
using namespace AOI;

ScanImageWidget::ScanImageWidget(DataCtrl *pDataCtrl, QWidget *parent)
    : m_pDataCtrl(pDataCtrl),
      QWidget(parent)
{
    ui.setupUi(this);

    ui.lineEditOneFrameImageCountScan->setText(QString("%1").arg(System->getParam("scan_image_OneFrameImageCount").toInt()));
    ui.lineEditFrameCountXScan->setText(QString("%1").arg(System->getParam("scan_image_FrameCountX").toInt()));
    ui.lineEditFrameCountYScan->setText(QString("%1").arg(System->getParam("scan_image_FrameCountY").toInt()));
    ui.lineEditOverlapXScan->setText(QString("%1").arg(System->getParam("scan_image_OverlapX").toDouble()));
    ui.lineEditOverlapYScan->setText(QString("%1").arg(System->getParam("scan_image_OverlapY").toDouble()));
    ui.lineEditRowImageCountScan->setText(QString("%1").arg(System->getParam("scan_image_RowImageCount").toInt()));
    ui.lineEditCombinedImageZoomFactorScan->setText(QString("%1").arg(System->getParam("scan_image_ZoomFactor").toDouble()));
    ui.lineEditFrameImageFolder->setText(System->getParam("scan_image_Folder").toString());
    ui.comboBoxScanDirection->setCurrentIndex(System->getParam("scan_image_Direction").toInt());
    ui.btnScanImage->setEnabled(false);
    ui.comboBoxDisplayImage->setEnabled(false);

    connect(ui.comboBoxDisplayImage, SIGNAL(currentIndexChanged(int)), SLOT(on_comboBoxDisplayImage_indexChanged(int)));
}

ScanImageWidget::~ScanImageWidget() {
}

void ScanImageWidget::on_btnCombineImageParamsSave_clicked() {
    System->setParam("scan_image_OneFrameImageCount", ui.lineEditOneFrameImageCountScan->text().toInt());
    System->setParam("scan_image_FrameCountX", ui.lineEditFrameCountXScan->text().toInt());
    System->setParam("scan_image_FrameCountY", ui.lineEditFrameCountYScan->text().toInt());
    System->setParam("scan_image_OverlapX", ui.lineEditOverlapXScan->text().toDouble());
    System->setParam("scan_image_OverlapY", ui.lineEditOverlapYScan->text().toDouble());
    System->setParam("scan_image_RowImageCount", ui.lineEditRowImageCountScan->text().toInt());
    System->setParam("scan_image_ZoomFactor", ui.lineEditCombinedImageZoomFactorScan->text().toDouble());
    System->setParam("scan_image_Direction", ui.comboBoxScanDirection->currentIndex());
}

void ScanImageWidget::on_btnPrepareScanImage_clicked() {
    float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f;
    if (Engine::GetBigBoardCoords(left, top, right, bottom) != 0) {
        String errorType, errorMessage;
        Engine::GetErrorDetail ( errorType, errorMessage );
        System->showMessage(QStringLiteral("设置电路板"), QStringLiteral("Error at GetBigBoardCoords, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
        return;
    }

    if (right <= left || top < bottom) {
        System->showMessage(QStringLiteral("设置电路板"), QStringLiteral("电路板参数不正确."));
        return;
    }

    m_vecVecFrameCtr.clear();
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto pCamera = getModule<ICamera>(CAMERA_MODEL);
    int nImageWidth = 0, nImageHeight = 0;
    pCamera->getCameraScreenSize(nImageWidth, nImageHeight);
    float fovWidth  = nImageWidth  * dResolutionX;
    float fovHeight = nImageHeight * dResolutionY;

    int frameCountX = static_cast<int>((right - left) / fovWidth ) + 1;
    int frameCountY = static_cast<int>((top - bottom) / fovHeight) + 1;
    float overlapX = 0.f, overlapY = 0.f;
    if (frameCountX > 1)
        overlapX = (frameCountX * fovWidth  - (right - left)) / (frameCountX - 1);
    else
        overlapX = 0.f;
    if (frameCountY > 1)
        overlapY = (frameCountY * fovHeight - (top - bottom)) / (frameCountY - 1);
    else
        overlapY = 0.f;

    for (int row = 0; row < frameCountY; ++ row) {
        Vision::VectorOfPoint2f vecFrameCtr;
        for (int col = 0; col < frameCountX; ++ col)
        {
            float frameCtrX = 0.f, frameCtrY = 0.f;
            if (frameCountX > 1)
                frameCtrX = left + (col * (fovWidth  - overlapX) + fovWidth  / 2.f);
            else
                frameCtrX = (right + left) / 2.f;

            if (frameCountY > 1)
                frameCtrY = top  - (row * (fovHeight - overlapY) + fovHeight / 2.f);
            else
                frameCtrY = (top + bottom) / 2.f;
            vecFrameCtr.emplace_back(frameCtrX, frameCtrY);
        }
        m_vecVecFrameCtr.push_back(vecFrameCtr);
    }

    ui.lineEditFrameCountXScan->setText(QString::number(frameCountX));
    ui.lineEditFrameCountYScan->setText(QString::number(frameCountY));
    ui.lineEditOverlapXScan->setText(QString::number(overlapX));
    ui.lineEditOverlapYScan->setText(QString::number(overlapY));

    ui.btnScanImage->setEnabled(true);
}

void ScanImageWidget::on_btnScanImage_clicked() {
    System->showMessage(QStringLiteral("扫图"), QStringLiteral("正在扫图中，请等候..."), 1);

    float fOverlapX = ui.lineEditOverlapXScan->text().toFloat();
    float fOverlapY = ui.lineEditOverlapYScan->text().toFloat();
    
    Vision::PR_SCAN_IMAGE_DIR enScanDir = static_cast<Vision::PR_SCAN_IMAGE_DIR>(ui.comboBoxScanDirection->currentIndex());

    m_pScanImageThread = new ScanImageThread(m_vecVecFrameCtr, fOverlapX, fOverlapY, enScanDir);
    connect(m_pScanImageThread, &ScanImageThread::finished, this, &ScanImageWidget::on_scanImage_done);
    m_pScanImageThread->start();
}

void ScanImageWidget::on_scanImage_done()
{
    if (m_pScanImageThread->isGood()) {
        m_pDataCtrl->setCombinedBigResult(m_pScanImageThread->getCombinedBigImages(), m_pScanImageThread->getCombinedBigHeight());
        auto pUI = getModule<IVisionUI>(UI_MODEL);
        pUI->setImage(m_pDataCtrl->getCombinedBigImages()[PROCESSED_IMAGE_SEQUENCE::WHITE_LIGHT]);
        ui.comboBoxDisplayImage->setEnabled(true);
    }

    System->closeMessage();

    delete m_pScanImageThread;
    m_pScanImageThread = NULL;
}

void ScanImageWidget::on_comboBoxDisplayImage_indexChanged(int index)
{
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto vecCombinedBigImage = m_pDataCtrl->getCombinedBigImages();
    if (index >= 0 && index <= vecCombinedBigImage.size() && !vecCombinedBigImage[index].empty())
        pUI->setImage(vecCombinedBigImage[index]);
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
    }
    else
        return;
    ui.lineEditFrameImageFolder->setText(fileNames[0]);
}

cv::Mat ScanImageWidget::combineImage(const QString &strInputFolder) {
    auto nCountOfImgPerFrame = System->getParam("scan_image_OneFrameImageCount").toInt();
    auto nCountOfFrameX = System->getParam("scan_image_FrameCountX").toInt();
    auto nCountOfFrameY = System->getParam("scan_image_FrameCountY").toInt();
    auto dOverlapUmX = System->getParam("scan_image_OverlapX").toDouble();
    auto dOverlapUmY = System->getParam("scan_image_OverlapY").toDouble();
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();

    int nOverlapX = static_cast<int> (dOverlapUmX / dResolutionX + 0.5);
    int nOverlapY = static_cast<int> (dOverlapUmY / dResolutionY + 0.5);

    auto nCountOfImgPerRow = System->getParam("scan_image_RowImageCount").toInt();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();
    auto nScanDirection = System->getParam("scan_image_Direction").toInt();

    Vision::PR_COMBINE_IMG_CMD stCmd;
    Vision::PR_COMBINE_IMG_RPY stRpy;
    stCmd.nCountOfImgPerFrame = nCountOfImgPerFrame;
    stCmd.nCountOfFrameX = nCountOfFrameX;
    stCmd.nCountOfFrameY = nCountOfFrameY;
    stCmd.nOverlapX = nOverlapX;
    stCmd.nOverlapY = nOverlapX;
    stCmd.nCountOfImgPerRow = nCountOfImgPerRow;
    stCmd.enScanDir = static_cast<Vision::PR_SCAN_IMAGE_DIR> (nScanDirection);

    const std::string strFolder = strInputFolder.toStdString() + "/";

    for (int nRow = 0; nRow < stCmd.nCountOfFrameY; ++ nRow)
        //Column start from right to left.
    for (int nCol = 0; nCol < stCmd.nCountOfFrameX; ++ nCol)
    for (int imgNo = 1; imgNo <= stCmd.nCountOfImgPerFrame; ++ imgNo) {
        char arrCharFileName[100];
        int nImageIndex = nCol * stCmd.nCountOfImgPerFrame + nRow * stCmd.nCountOfImgPerRow + imgNo;
        _snprintf(arrCharFileName, sizeof(arrCharFileName), "F%d-%d-1.bmp", nRow + 1, nImageIndex);
        std::string strImagePath = strFolder + std::string(arrCharFileName);
        cv::Mat mat = cv::imread(strImagePath, cv::IMREAD_GRAYSCALE);
        if (mat.empty())
            continue;
        cv::Mat matColor;
        cv::cvtColor(mat, matColor, CV_BayerGR2BGR);
        stCmd.vecInputImages.push_back(matColor);
    }

    Vision::PR_CombineImg(&stCmd, &stRpy);
    if (stRpy.enStatus != Vision::VisionStatus::OK) {
        Vision::PR_GET_ERROR_INFO_RPY stErrorInfo;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrorInfo);
        QString strErrormsg = QString("Failed to combine images, error message: ") + stErrorInfo.achErrorStr;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), strErrormsg, QStringLiteral("Quit"));
        return cv::Mat();
    }

    cv::Mat matResize;
    cv::resize(stRpy.vecResultImages[0], matResize, cv::Size(), dCombinedImageScale, dCombinedImageScale);

    int imgNo = 1;
    for (const auto &mat : stRpy.vecResultImages) {
        char arrChFileName[100];
        _snprintf(arrChFileName, sizeof(arrChFileName), "CombineResult_%d.bmp", imgNo);
        std::string strResultFile = strFolder + arrChFileName;
        cv::Mat matResize;
        cv::resize(mat, matResize, cv::Size(), dCombinedImageScale, dCombinedImageScale);
        cv::imwrite(strResultFile, matResize);
        ++ imgNo;
    }

    return matResize;
}

void ScanImageWidget::on_btnCombineLoadImage_clicked() {
    auto strFolder = ui.lineEditFrameImageFolder->text();
    bool bBoardRotated = ui.checkBoxBoardRotated->isChecked();
    double dResolutionX = ui.lineEditResolutionX->text().toDouble();
    double dResolutionY = ui.lineEditResolutionY->text().toDouble();
    System->setParam("scan_image_Folder", strFolder);
    System->setSysParam("CAM_RESOLUTION_X", dResolutionX);
    System->setSysParam("CAM_RESOLUTION_Y", dResolutionY);
    System->setSysParam("BOARD_ROTATED", bBoardRotated);

    auto matImage = combineImage(strFolder);
    if (matImage.empty())
        return;

    updateImageDeviceWindows(matImage);
}

void ScanImageWidget::on_btnSelectCombinedImage_clicked() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilter(tr("Image Files (*.png *.jpg *.bmp)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    }
    else
        return;

    auto strFolder = ui.lineEditFrameImageFolder->text();
    bool bBoardRotated = ui.checkBoxBoardRotated->isChecked();
    double dResolutionX = ui.lineEditResolutionX->text().toDouble();
    double dResolutionY = ui.lineEditResolutionY->text().toDouble();
    System->setParam("scan_image_Folder", strFolder);
    System->setSysParam("CAM_RESOLUTION_X", dResolutionX);
    System->setSysParam("CAM_RESOLUTION_Y", dResolutionY);
    System->setSysParam("BOARD_ROTATED", bBoardRotated);

    auto matImage = cv::imread(fileNames[0].toStdString());
    updateImageDeviceWindows(matImage);
}

void ScanImageWidget::updateImageDeviceWindows(const cv::Mat &matImage) {
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    float fCombinedImgScale = System->getParam("scan_image_ZoomFactor").toDouble();

    VisionViewDeviceVector vecVisionViewDevices;
    Engine::BoardVector vecBoard;
    auto result = Engine::GetAllBoards(vecBoard);
    if (Engine::OK != result) {
        std::string errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get board from project, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    for (const auto &board : vecBoard) {
        Engine::DeviceVector vecDevice;
        result = Engine::GetBoardDevice(board.Id, vecDevice);
        if (Engine::OK != result) {
            std::string errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            errorMessage = "Failed to get devices from project, error message " + errorMessage;
            QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
            return;
        }

        for (const auto &device : vecDevice) {
            if (device.isBottom)
                continue;

            auto x = (device.x + board.x) / dResolutionX * fCombinedImgScale;
            auto y = (device.y + board.y) / dResolutionY * fCombinedImgScale;
            if (bBoardRotated)
                x = matImage.cols - x;
            else
                y = matImage.rows - y; //In cad, up is positive, but in image, down is positive.
            auto width = device.width / dResolutionX * fCombinedImgScale;
            auto height = device.height / dResolutionY * fCombinedImgScale;
            cv::RotatedRect deviceWindow(cv::Point2f(x, y), cv::Size2f(width, height), device.angle);
            vecVisionViewDevices.emplace_back(device.Id, device.name, deviceWindow);
        }
    }

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SET_FIDUCIAL_MARK);
    pUI->setImage(matImage, true);
    pUI->setDeviceWindows(vecVisionViewDevices);
}

bool ScanImageWidget::_moveToCapturePos(float fPosX, float fPosY)
{
    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return false;

    if (! pMotion->moveToGroup(std::vector<int>({AXIS_MOTOR_X, AXIS_MOTOR_Y}), std::vector<double>({fPosX, fPosY}), std::vector<int>({0, 0}), true)) {
        System->setTrackInfo(QString("move to position error."));
		return false;
    }
    return true;
}