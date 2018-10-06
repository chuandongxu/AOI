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
#include "DataUtils.h"

using namespace NFG::AOI;
using namespace AOI;

ScanImageWidget::ScanImageWidget(DataCtrl *pDataCtrl, QWidget *parent)
    : m_pDataCtrl(pDataCtrl),
      QWidget(parent)
{
    ui.setupUi(this);
    
    ui.lineEditFrameImageFolder->setText(System->getParam("scan_image_Folder").toString());
    
    ui.btnScanImage->setEnabled(false);
    ui.comboBoxDisplayImage->setEnabled(false);

    connect(ui.comboBoxDisplayImage, SIGNAL(currentIndexChanged(int)), SLOT(on_comboBoxDisplayImage_indexChanged(int)));
}

ScanImageWidget::~ScanImageWidget() {
}

void ScanImageWidget::showEvent(QShowEvent *event) {
    Int32 nCountOfFrameX = 0, nCountOfFrameY = 0, nCountOfImgPerFrame = 0, nCountOfImgPerRow = 0, nScanDirection = 0;
    Engine::GetParameter("ScanImageFrameCountX", nCountOfFrameX, 0);
    Engine::GetParameter("ScanImageFrameCountY", nCountOfFrameY, 0);
    Engine::GetParameter("ScanImageOneFrameImageCount", nCountOfImgPerFrame, 0);
    Engine::GetParameter("ScanImageRowImageCount", nCountOfImgPerRow, 0);
    Engine::GetParameter("ScanImageDirection", nScanDirection, 0);

    float dOverlapUmX = 0.f, dOverlapUmY = 0.f, dCombinedImageScale = 1.f;
    Engine::GetParameter("ScanImageOverlapX", dOverlapUmX, 0.f);
    Engine::GetParameter("ScanImageOverlapY", dOverlapUmY, 0.f);
    Engine::GetParameter("ScanImageZoomFactor", dCombinedImageScale, 1.f);

    ui.lineEditOneFrameImageCountScan->setText(QString("%1").arg(nCountOfImgPerFrame));
    ui.lineEditFrameCountXScan->setText(QString("%1").arg(nCountOfFrameX));
    ui.lineEditFrameCountYScan->setText(QString("%1").arg(nCountOfFrameY));
    ui.lineEditOverlapXScan->setText(QString("%1").arg(dOverlapUmX));
    ui.lineEditOverlapYScan->setText(QString("%1").arg(dOverlapUmY));
    ui.lineEditRowImageCountScan->setText(QString("%1").arg(nCountOfImgPerRow));
    ui.lineEditCombinedImageZoomFactorScan->setText(QString("%1").arg(dCombinedImageScale));
    ui.comboBoxScanDirection->setCurrentIndex(nScanDirection);
}

void ScanImageWidget::on_btnCombineImageParamsSave_clicked() {
    Engine::SetParameter("ScanImageOneFrameImageCount", ui.lineEditOneFrameImageCountScan->text().toInt());
    Engine::SetParameter("ScanImageFrameCountX", ui.lineEditFrameCountXScan->text().toInt());
    Engine::SetParameter("ScanImageFrameCountY", ui.lineEditFrameCountYScan->text().toInt());
    Engine::SetParameter("ScanImageOverlapX", ui.lineEditOverlapXScan->text().toFloat());
    Engine::SetParameter("ScanImageOverlapY", ui.lineEditOverlapYScan->text().toFloat());
    Engine::SetParameter("ScanImageRowImageCount", ui.lineEditRowImageCountScan->text().toInt());
    Engine::SetParameter("ScanImageZoomFactor", ui.lineEditCombinedImageZoomFactorScan->text().toFloat());
    Engine::SetParameter("ScanImageDirection", ui.comboBoxScanDirection->currentIndex());
}

void ScanImageWidget::on_btnPrepareScanImage_clicked() {
    float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f;
    if (Engine::GetBigBoardCoords(left, top, right, bottom) != 0) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
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
    float overlapX = 0.f, overlapY = 0.f;

    int result = DataUtils::assignFrames(left, top, right, bottom, fovWidth, fovHeight, m_vecVecFrameCtr, overlapX, overlapY);
    Int32 frameCountX = m_vecVecFrameCtr[0].size();
    Int32 frameCountY = m_vecVecFrameCtr.size();

    ui.lineEditFrameCountXScan->setText(QString::number(frameCountX));
    ui.lineEditFrameCountYScan->setText(QString::number(frameCountY));
    ui.lineEditOverlapXScan->setText(QString::number(overlapX));
    ui.lineEditOverlapYScan->setText(QString::number(overlapY));

    Engine::SetParameter("ScanImageFrameCountX", frameCountX);
    Engine::SetParameter("ScanImageFrameCountY", frameCountY);
    Engine::SetParameter("ScanImageOverlapX", overlapX);
    Engine::SetParameter("ScanImageOverlapY", overlapY);

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
        updateImageDeviceWindows(m_pDataCtrl->getCombinedBigImages()[PROCESSED_IMAGE_SEQUENCE::SOLDER_LIGHT]);
        ui.comboBoxDisplayImage->setEnabled(true);

        auto pUI = getModule<IVisionUI>(UI_MODEL);
        pUI->setHeightData(m_pScanImageThread->getCombinedBigHeight());
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

cv::Mat ScanImageWidget::_combineImage(const QString &strInputFolder) {
    Int32 nCountOfFrameX = 0, nCountOfFrameY = 0, nCountOfImgPerFrame = 0, nCountOfImgPerRow = 0, nScanDirection = 0;
    Engine::GetParameter("ScanImageFrameCountX", nCountOfFrameX, 0);
    Engine::GetParameter("ScanImageFrameCountY", nCountOfFrameY, 0);
    Engine::GetParameter("ScanImageOneFrameImageCount", nCountOfImgPerFrame, 0);
    Engine::GetParameter("ScanImageRowImageCount", nCountOfImgPerRow, 0);
    Engine::GetParameter("ScanImageDirection", nScanDirection, 0);

    float fOverlapUmX = 0.f, fOverlapUmY = 0.f;
    Engine::GetParameter("ScanImageOverlapX", fOverlapUmX, 0.f);
    Engine::GetParameter("ScanImageOverlapY", fOverlapUmY, 0.f);

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);

    int nOverlapX = static_cast<int> (fOverlapUmX / dResolutionX + 0.5f);
    int nOverlapY = static_cast<int> (fOverlapUmY / dResolutionY + 0.5f);
    
    float dCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", dCombinedImageScale, 1.f);    

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
    Engine::SetParameter("BOARD_ROTATED", bBoardRotated);

    auto matImage = _combineImage(strFolder);
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

    Engine::SetParameter("BOARD_ROTATED", bBoardRotated);

    auto matImage = cv::imread(fileNames[0].toStdString());
    updateImageDeviceWindows(matImage);
}

void ScanImageWidget::on_btnSaveScanImage_clicked() {
    auto vecCombinedBigImage = m_pDataCtrl->getCombinedBigImages();
    if (vecCombinedBigImage.empty()) {
        System->showMessage(QString(QStringLiteral("扫图")), QString(QStringLiteral("请先扫图!")));
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Open Directory"),
        "/home",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    auto strFolder = dir.toStdString();
    for (size_t i = 0; i < vecCombinedBigImage.size(); ++ i) {
        auto strImage = strFolder + "/" + std::to_string(i) + ".png";
        cv::imwrite(strImage, vecCombinedBigImage[i]);
    }

    auto strHeightFile = strFolder + "/height.yml";
    cv::FileStorage fs(strHeightFile, cv::FileStorage::WRITE);
    if (!fs.isOpened()) {
        System->showMessage(QString(QStringLiteral("保存图片")), QString(QStringLiteral("打开文件 %1 失败!")).arg(strHeightFile.c_str()));
        return;
    }

    cv::write(fs, "height", m_pDataCtrl->getCombinedBigHeight());
    fs.release();
}

void ScanImageWidget::on_btnOpenScanImage_clicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Open Directory"),
        "/home",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    auto strFolder = dir.toStdString();
    AOI::Vision::VectorOfMat vecCombinedImage;
    for (size_t i = 0; i < PROCESSED_IMAGE_SEQUENCE::TOTAL_COUNT; ++ i) {
        auto strImage = strFolder + "/" + std::to_string(i) + ".png";
        auto matImage = cv::imread(strImage);
        if (matImage.empty()) {
            System->showMessage(QString(QStringLiteral("加载图片")), QString(QStringLiteral("读取图片 %1 失败!")).arg(strImage.c_str()));
            return;
        }
        vecCombinedImage.push_back(matImage);
    }

    auto strHeightFile = strFolder + "/height.yml";
    cv::FileStorage fs(strHeightFile, cv::FileStorage::READ);
    if (!fs.isOpened()) {
        System->showMessage(QString(QStringLiteral("打开高度文件")), QString(QStringLiteral("打开高度文件 %1 失败!")).arg(strHeightFile.c_str()));
        return;
    }

    cv::Mat matHeight;
    cv::FileNode fileNode = fs["height"];
    cv::read(fileNode, matHeight);
    if (matHeight.empty()) {
        System->showMessage(QString(QStringLiteral("打开高度文件")), QString(QStringLiteral("读取高度文件 %1 失败!")).arg(strHeightFile.c_str()));
        return;
    }
    fs.release();

    m_pDataCtrl->setCombinedBigResult(vecCombinedImage, matHeight);
    updateImageDeviceWindows(m_pDataCtrl->getCombinedBigImages()[PROCESSED_IMAGE_SEQUENCE::SOLDER_LIGHT]);
    ui.comboBoxDisplayImage->setEnabled(true);

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setHeightData(matHeight);
}

void ScanImageWidget::updateImageDeviceWindows(const cv::Mat &matImage) {    
    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    float fCombinedImgScale = System->getParam("ScanImageZoomFactor").toDouble();

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