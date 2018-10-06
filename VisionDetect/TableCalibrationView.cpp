#include "TableCalibrationView.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/ICamera.h"
#include "../include/IMotion.h"
#include "../include/ILight.h"
#include "../include/IVisionUI.h"

#include <QMessageBox>
#include <qthread.h>

#include "../lib/VisionLibrary/include/VisionAPI.h"
#define ToInt(value)                (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace AOI;

TableCalibrationView::TableCalibrationView(VisionCtrl* pCtrl, QWidget *parent)
    : m_pCtrl(pCtrl), QWidget(parent)
{
    ui.setupUi(this);

    m_bGuideCali = false;
    ui.pushButton_stopCali->setEnabled(false);

    initUI();
}

TableCalibrationView::~TableCalibrationView()
{
}

void TableCalibrationView::initUI()
{
    double dCaliReadyPositionX = System->getParam("table_cali_ready_pos_x").toDouble();
    double dCaliReadyPositionY = System->getParam("table_cali_ready_pos_y").toDouble();
    double dCaliReadyPositionZ = System->getParam("table_cali_ready_pos_z").toDouble();
    ui.lineEdit_readyPosX->setText(QString("%1").arg(dCaliReadyPositionX));
    ui.lineEdit_readyPosY->setText(QString("%1").arg(dCaliReadyPositionY));
    ui.lineEdit_readyPosZ->setText(QString("%1").arg(dCaliReadyPositionZ));

    connect(ui.pushButton_Joystick, SIGNAL(clicked()), SLOT(onReadyPosJoystick()));
    connect(ui.pushButton_MoveToReady, SIGNAL(clicked()), SLOT(onMoveToReady()));

    connect(ui.pushButton_startCali, SIGNAL(clicked()), SLOT(onStart()));
    connect(ui.pushButton_stopCali, SIGNAL(clicked()), SLOT(onEnd()));

    double dCaliMoveDist = System->getParam("table_cali_move_dist").toDouble();
    ui.lineEdit_moveDist->setText(QString("%1").arg(dCaliMoveDist));

    connect(ui.pushButton_runTest, SIGNAL(clicked()), SLOT(onRunTest()));
    connect(ui.pushButton_SaveCali, SIGNAL(clicked()), SLOT(onSaveCali()));
}

void TableCalibrationView::onReadyPosJoystick()
{
    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
    if (pMotion)
    {
        pMotion->setJoystickXMotor(AXIS_MOTOR_X, 1.2, ui.lineEdit_readyPosX);
        pMotion->setJoystickYMotor(AXIS_MOTOR_Y, 1.2, ui.lineEdit_readyPosY);

        pMotion->startJoystick();
    }
}

void TableCalibrationView::onMoveToReady()
{
    double dCaliReadyPositionX = ui.lineEdit_readyPosX->text().toDouble();
    double dCaliReadyPositionY = ui.lineEdit_readyPosY->text().toDouble();
    double dCaliReadyPositionZ = ui.lineEdit_readyPosZ->text().toDouble();

    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
    if (pMotion)
    {
        std::vector<int> axis;
        std::vector<double> pos;
        std::vector<int> profs;

        axis.push_back(AXIS_MOTOR_X); axis.push_back(AXIS_MOTOR_Y); axis.push_back(AXIS_MOTOR_Z);
        pos.push_back(dCaliReadyPositionX); pos.push_back(dCaliReadyPositionY); pos.push_back(dCaliReadyPositionZ);
        profs.push_back(1); axis.push_back(1); axis.push_back(1);

        pMotion->moveToGroup(axis, pos, profs, true);
    }
}

void TableCalibrationView::onSaveCali()
{
    double dCaliReadyPositionX = ui.lineEdit_readyPosX->text().toDouble();
    double dCaliReadyPositionY = ui.lineEdit_readyPosY->text().toDouble();
    double dCaliReadyPositionZ = ui.lineEdit_readyPosZ->text().toDouble();

    System->setParam("table_cali_ready_pos_x", dCaliReadyPositionX);
    System->setParam("table_cali_ready_pos_y", dCaliReadyPositionY);
    System->setParam("table_cali_ready_pos_z", dCaliReadyPositionZ);

    double dCaliMoveDist = ui.lineEdit_moveDist->text().toDouble();
    System->setParam("table_cali_move_dist", dCaliMoveDist);
}

void TableCalibrationView::onStart()
{
    ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
    if (!pCam) return;

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    if (!pUI) return;

    if (m_bGuideCali)
    {
        QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("目前已经在运行！"));
        return;
    }

    pCam->selectCaptureMode(ICamera::TRIGGER_ONE);
    if (pCam->getCameraNum() > 0)
    {
        if (!pUI->startUpCapture())
        {
            QSystem::closeMessage();
            QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("相机初始化问题。"));
            return;
        }
    }
    else
    {
        QSystem::closeMessage();
        QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("请检查相机是否连接。"));
        return;
    }

    m_bGuideCali = true;
    ui.pushButton_startCali->setEnabled(false);
    ui.pushButton_stopCali->setEnabled(true);
}

void TableCalibrationView::onEnd()
{
    ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
    if (!pCam) return;

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    if (!pUI) return;

    if (!m_bGuideCali) return;
    
    pUI->endUpCapture();

    QSystem::closeMessage();

    m_bGuideCali = false;
    ui.pushButton_startCali->setEnabled(true);
    ui.pushButton_stopCali->setEnabled(false);
}

void TableCalibrationView::onRunTest()
{
    // Move to Ready
    double dCaliReadyPositionX = ui.lineEdit_readyPosX->text().toDouble();
    double dCaliReadyPositionY = ui.lineEdit_readyPosY->text().toDouble();
    double dCaliReadyPositionZ = ui.lineEdit_readyPosZ->text().toDouble();

    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
    if (pMotion)
    {
        std::vector<int> axis;
        std::vector<double> pos;
        std::vector<int> profs;

        axis.push_back(AXIS_MOTOR_X); axis.push_back(AXIS_MOTOR_Y); axis.push_back(AXIS_MOTOR_Z);
        pos.push_back(dCaliReadyPositionX); pos.push_back(dCaliReadyPositionY); pos.push_back(dCaliReadyPositionZ);
        profs.push_back(1); axis.push_back(1); axis.push_back(1);

        pMotion->moveToGroup(axis, pos, profs, true);
    }

    QThread::msleep(500);

    cv::Mat matImg;
    if (!guideReadImage(matImg))
    {
        QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("相机抓图问题。"));
        return;
    }
    m_matImages[0] = matImg;

    // Move to another position
    double dCaliMoveDist = ui.lineEdit_moveDist->text().toDouble();
    dCaliReadyPositionX += dCaliMoveDist;
    if (pMotion)
    {
        std::vector<int> axis;
        std::vector<double> pos;
        std::vector<int> profs;

        axis.push_back(AXIS_MOTOR_X); axis.push_back(AXIS_MOTOR_Y); axis.push_back(AXIS_MOTOR_Z);
        pos.push_back(dCaliReadyPositionX); pos.push_back(dCaliReadyPositionY); pos.push_back(dCaliReadyPositionZ);
        profs.push_back(1); axis.push_back(1); axis.push_back(1);

        pMotion->moveToGroup(axis, pos, profs, true);
    }

    if (!guideReadImage(matImg))
    {
        QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("相机抓图问题。"));
        return;
    }
    m_matImages[1] = matImg;

    // Combine Image
    auto nCountOfImgPerFrame = ui.lineEditOneFrameImageCountScan->text().toInt();
    auto nCountOfFrameX = ui.lineEditFrameCountXScan->text().toInt();
    auto nCountOfFrameY = ui.lineEditFrameCountYScan->text().toInt();
    auto dOverlapUmX = ui.lineEditOverlapXScan->text().toDouble();
    auto dOverlapUmY = ui.lineEditOverlapYScan->text().toDouble();
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    int nOverlapX = static_cast<int> (dOverlapUmX / dResolutionX + 0.5);
    int nOverlapY = static_cast<int> (dOverlapUmY / dResolutionY + 0.5);

    auto nCountOfImgPerRow = ui.lineEditRowImageCountScan->text().toInt();
    auto dCombinedImageScale = ui.lineEditCombinedImageZoomFactorScan->text().toDouble();
    auto nScanDirection = ui.comboBoxScanDirection->currentIndex();

    Vision::PR_COMBINE_IMG_CMD stCmd;
    Vision::PR_COMBINE_IMG_RPY stRpy;
    stCmd.nCountOfImgPerFrame = nCountOfImgPerFrame;
    stCmd.nCountOfFrameX = nCountOfFrameX;
    stCmd.nCountOfFrameY = nCountOfFrameY;
    stCmd.nOverlapX = nOverlapX;
    stCmd.nOverlapY = nOverlapX;
    stCmd.nCountOfImgPerRow = nCountOfImgPerRow;
    stCmd.enScanDir = static_cast<Vision::PR_SCAN_IMAGE_DIR> (nScanDirection);    

    for (int i = 0; i < _TestImageNum; ++i)
    {
        stCmd.vecInputImages.push_back(m_matImages[i]);
    }

    Vision::PR_CombineImg(&stCmd, &stRpy);
    if (stRpy.enStatus != Vision::VisionStatus::OK) {
        Vision::PR_GET_ERROR_INFO_RPY stErrorInfo;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrorInfo);
        QString strErrormsg = QString("Failed to combine images, error message: ") + stErrorInfo.achErrorStr;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), strErrormsg, QStringLiteral("Quit"));
        return;
    }

    cv::Mat matResize;
    cv::resize(stRpy.vecResultImages[0], matResize, cv::Size(), dCombinedImageScale, dCombinedImageScale);
    
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    if (pUI)
    {
        pUI->displayImage(matResize);
    }
}

bool TableCalibrationView::guideReadImage(cv::Mat& matImg)
{
    ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
    if (!pCam) return false;

    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion) return false;

    ILight* pLight = getModule<ILight>(LIGHT_MODEL);
    if (!pLight) return false;

    if (!pCam->selectCaptureMode(ICamera::TRIGGER_ONE))// 1 image
    {
        System->setTrackInfo(QString("selectCaptureMode error"));
        return false;
    }

    //if (!pCam->startCapturing())
    //{
    //    System->setTrackInfo(QString("startCapturing error"));
    //    return false;
    //}

    bool bTriggerBoard = System->isTriggerBoard();
    if (bTriggerBoard)
    {
        if (!pLight->triggerCapturing(ILight::TRIGGER(ILight::TRIGGER_ONE_CH1), true, true))
        {
            System->setTrackInfo(QString("triggerCapturing error!"));
        }
    }
    else
    {
        QVector<int> nPorts;

        nPorts.push_back(DO_LIGHT1_CH1);
        nPorts.push_back(DO_CAMERA_TRIGGER2);

        pMotion->setDOs(nPorts, 1);
        QThread::msleep(10);
        pMotion->setDOs(nPorts, 0);
    }

    QVector<cv::Mat> matImgs;
    if (!pCam->getLastImages(matImgs))
    {
        System->setTrackInfo(QString("getImages error"));
        return false;
    }

    if (matImgs.size() > 0)
        matImg = matImgs[0];

    return true;
}
