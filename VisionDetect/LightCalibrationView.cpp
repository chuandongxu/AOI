#include "LightCalibrationView.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/ICamera.h"
#include "../include/IMotion.h"
#include "../include/IVisionUI.h"
#include "../include/ILight.h"

#include <QMessageBox>
#include <qthread.h>
#include <qdebug.h>

#include "../lib/VisionLibrary/include/VisionAPI.h"
#define ToInt(value)                (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace AOI;

LightCalibrationView::LightCalibrationView(VisionCtrl* pCtrl, QWidget *parent)
    : m_pCtrl(pCtrl), QWidget(parent)
{
    ui.setupUi(this);

    m_bGuideCali = false;
    ui.pushButton_stopCali->setEnabled(false);
    ui.pushButton_captureLight->setEnabled(false);
    ui.pushButton_DetectGrayScale->setEnabled(false);
    ui.pushButton_AutoCali->setEnabled(false);

    initUI();
}

LightCalibrationView::~LightCalibrationView()
{
}

void LightCalibrationView::initUI()
{
    double dLightCaliReadyPositionX = System->getParam("light_cali_ready_pos_x").toDouble();
    double dLightCaliReadyPositionY = System->getParam("light_cali_ready_pos_y").toDouble();
    double dLightCaliReadyPositionZ = System->getParam("light_cali_ready_pos_z").toDouble();
    ui.lineEdit_readyPosX->setText(QString("%1").arg(dLightCaliReadyPositionX));
    ui.lineEdit_readyPosY->setText(QString("%1").arg(dLightCaliReadyPositionY));
    ui.lineEdit_readyPosZ->setText(QString("%1").arg(dLightCaliReadyPositionZ));

    connect(ui.pushButton_Joystick, SIGNAL(clicked()), SLOT(onReadyPosJoystick()));
    connect(ui.pushButton_MoveToReady, SIGNAL(clicked()), SLOT(onMoveToReady()));

    connect(ui.pushButton_startCali, SIGNAL(clicked()), SLOT(onStart()));
    connect(ui.pushButton_stopCali, SIGNAL(clicked()), SLOT(onEnd()));
    connect(ui.pushButton_captureLight, SIGNAL(clicked()), SLOT(onCaptureLight()));

    connect(ui.comboBox_selectLight, SIGNAL(currentIndexChanged(int)), SLOT(onSelectLightIndexChanged(int)));
    QStringList ls;
    ls << QStringLiteral("第一圈红") << QStringLiteral("第一圈白") << QStringLiteral("第二圈绿") << QStringLiteral("第三圈蓝") << QStringLiteral("第四圈白") << QStringLiteral("第四圈蓝");
    ui.comboBox_selectLight->addItems(ls);
    ui.comboBox_selectLight->setCurrentIndex(0);

    connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), SLOT(onSliderChanged(int)));

    int nDetectGrayScaleRow = System->getParam("detect_gray_scale_row").toInt();
    int nDetectGrayScaleCol = System->getParam("detect_gray_scale_col").toInt();
    int nGrayLevel = System->getParam("auto_detect_gray_level").toInt();
    ui.lineEdit_DetectGraySacleRow->setText(QString("%1").arg(nDetectGrayScaleRow));
    ui.lineEdit_DetectGrayScaleCol->setText(QString("%1").arg(nDetectGrayScaleCol));
    ui.lineEdit_GrayValue->setText(QString("%1").arg(nGrayLevel));
    connect(ui.pushButton_DetectGrayScale, SIGNAL(clicked()), SLOT(onDetectGrayScale()));

    connect(ui.pushButton_SaveCali, SIGNAL(clicked()), SLOT(onSaveCali()));

    connect(ui.pushButton_AutoCali, SIGNAL(clicked()), SLOT(onAutoCali()));
}

void LightCalibrationView::onReadyPosJoystick()
{
    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
    if (pMotion)
    {
        pMotion->setJoystickXMotor(AXIS_MOTOR_X, 1.2, ui.lineEdit_readyPosX);
        pMotion->setJoystickYMotor(AXIS_MOTOR_Y, 1.2, ui.lineEdit_readyPosY);

        pMotion->startJoystick();
    }
}

void LightCalibrationView::onMoveToReady()
{
    double dLightCaliReadyPositionX = ui.lineEdit_readyPosX->text().toDouble();
    double dLightCaliReadyPositionY = ui.lineEdit_readyPosY->text().toDouble();
    //double dLightCaliReadyPositionZ = ui.lineEdit_readyPosZ->text().toDouble();

    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
    if (pMotion)
    {
        std::vector<int> axis;
        std::vector<double> pos;
        std::vector<int> profs;

        axis.push_back(AXIS_MOTOR_X); axis.push_back(AXIS_MOTOR_Y); //axis.push_back(AXIS_MOTOR_Z);
        pos.push_back(dLightCaliReadyPositionX); pos.push_back(dLightCaliReadyPositionY); //pos.push_back(dLightCaliReadyPositionZ);
        profs.push_back(0); profs.push_back(0); //profs.push_back(1);

        pMotion->moveToGroup(axis, pos, profs, true);
    }
}

void LightCalibrationView::onSaveCali()
{
    double dLightCaliReadyPositionX = ui.lineEdit_readyPosX->text().toDouble();
    double dLightCaliReadyPositionY = ui.lineEdit_readyPosY->text().toDouble();
    double dLightCaliReadyPositionZ = ui.lineEdit_readyPosZ->text().toDouble();

    System->setParam("light_cali_ready_pos_x", dLightCaliReadyPositionX);
    System->setParam("light_cali_ready_pos_y", dLightCaliReadyPositionY);
    System->setParam("light_cali_ready_pos_z", dLightCaliReadyPositionZ);    

    int nGrayLevel = ui.lineEdit_GrayValue->text().toInt();
    System->setParam("auto_detect_gray_level", nGrayLevel);

    ILight* pLight = getModule<ILight>(LIGHT_MODEL);
    if (pLight)
    {
        for (int i = 0; i < 6; i++)
        {
            pLight->saveLuminance(i);
        }       
    }
}

void LightCalibrationView::onStart()
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

    onMoveToReady();

    QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("标定正在启动中..."), 0);
    QApplication::processEvents();

    pCam->selectCaptureMode(ICamera::TRIGGER_ONE);
    if (pCam->getCameraNum() > 0)
    {
        if (!pUI->startUpCapture(false))
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
    ui.pushButton_captureLight->setEnabled(true);
    ui.pushButton_DetectGrayScale->setEnabled(true);
    ui.pushButton_AutoCali->setEnabled(true);

    ui.comboBox_selectLight->setCurrentIndex(0);

    QApplication::processEvents();
    QSystem::closeMessage();
}

void LightCalibrationView::onEnd()
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

    ui.pushButton_captureLight->setEnabled(false);
    ui.pushButton_DetectGrayScale->setEnabled(false);
    ui.pushButton_AutoCali->setEnabled(false);
}

void LightCalibrationView::onCaptureLight()
{
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    if (!pUI) return;

    cv::Mat matImg;
    if (guideReadImage(matImg, ui.comboBox_selectLight->currentIndex()))
    {
        m_matImage = matImg;

        int nWidth = matImg.cols;
        int nHeight = matImg.rows;

        double dScale = 0.2;

        m_selectROI.x = nWidth / 2 + nWidth / 2 * dScale;
        m_selectROI.y = 0 + nHeight * dScale;
        m_selectROI.width = nWidth / 2 - nWidth / 2 * dScale * 2;
        m_selectROI.height = nHeight - nHeight * dScale * 2;

        cv::rectangle(matImg, m_selectROI, cv::Scalar(0, 255, 0), 5, 8, 0);
        pUI->setImage(matImg, true);
    }
}

bool LightCalibrationView::guideReadImage(cv::Mat& matImg, int nSelectLight)
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
        if (!pLight->triggerCapturing(ILight::TRIGGER(ILight::TRIGGER_ONE_CH1 + nSelectLight), true, true))
        {
            System->setTrackInfo(QString("triggerCapturing error!"));
        }
    }
    else
    {
        QVector<int> nPorts;
        switch (nSelectLight)
        {
        case 0:
            nPorts.push_back(DO_LIGHT1_CH1);
            break;
        case 1:
            nPorts.push_back(DO_LIGHT1_CH2);
            break;
        case 2:
            nPorts.push_back(DO_LIGHT1_CH3);
            break;
        case 3:
            nPorts.push_back(DO_LIGHT1_CH4);
            break;
        case 4:
            nPorts.push_back(DO_LIGHT2_CH1);
            break;
        case 5:
            nPorts.push_back(DO_LIGHT2_CH2);
            break;
        default:
            break;
        }
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

int LightCalibrationView::getLightLum(int nIndex)
{
    ILight* pLight = getModule<ILight>(LIGHT_MODEL);
    if (pLight)
    {
        int nLum = pLight->getChLuminace(nIndex);
        return nLum;
    }

    return 0;
}

void LightCalibrationView::setLightLum(int nIndex, int lum)
{
    ILight* pLight = getModule<ILight>(LIGHT_MODEL);
    if (pLight)
    {
        pLight->setLuminance(nIndex, lum);
    }
}

void LightCalibrationView::onSelectLightIndexChanged(int iIndex)
{
    int nLightIndex = ui.comboBox_selectLight->currentIndex();

    ui.horizontalSlider->setValue(getLightLum(nLightIndex));
}

void LightCalibrationView::onSliderChanged(int lum)
{
    QString str = QString::number(lum);
    ui.lineEdit_6->setText(str);

    int nLightIndex = ui.comboBox_selectLight->currentIndex();    
    setLightLum(nLightIndex, lum);    
}

void LightCalibrationView::onDetectGrayScale()
{
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    if (!pUI) return;

    if (m_matImage.empty())
    {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
        return;
    }

    int nDetectGrayScaleRow = ui.lineEdit_DetectGraySacleRow->text().toInt();
    int nDetectGrayScaleCol = ui.lineEdit_DetectGrayScaleCol->text().toInt();

    Vision::PR_GRID_AVG_GRAY_SCALE_CMD  stCmd;
    stCmd.nGridRow = nDetectGrayScaleRow;
    stCmd.nGridCol = nDetectGrayScaleCol;
    
    stCmd.vecInputImgs.push_back(m_matImage);

    Vision::PR_GRID_AVG_GRAY_SCALE_RPY stRpy;
    Vision::VisionStatus retStatus = Vision::PR_GridAvgGrayScale(&stCmd, &stRpy);
    if (retStatus == Vision::VisionStatus::OK)
    {
        pUI->displayImage(stRpy.matResultImg);
    }
    else
    {
        pUI->addImageText(QString("Error at Cal Gray Sacle, error code = %1").arg((int)retStatus));
    }
}

void LightCalibrationView::onAutoCali()
{
    QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("标定中..."), 0);
    QApplication::processEvents();

    int _MaxLum = 1000;
    int _GrayLevel = ui.lineEdit_GrayValue->text().toInt();
    int _MaxCount = 10;

    for (int i = 0; i < 6; i++)
    {
        int nMinLum = 0;
        int nMaxLum = _MaxLum;

        ui.comboBox_selectLight->setCurrentIndex(i);

        int nCurrLum = 0;
        int nCount = _MaxCount;
        while (nCount-- > 0)
        {
            nCurrLum = (nMaxLum + nMinLum) / 2;

            //setLightLum(i, nCurrLum);
            ui.horizontalSlider->setValue(nCurrLum);           
            onCaptureLight();

            cv::Mat matImg = m_matImage(m_selectROI);
            int gray = grayLevel(matImg);
            qDebug() << "Auto Cali, index: " << i << " gray: " << gray;

            if (gray >= _GrayLevel)
            {
                nMaxLum = nCurrLum;
            }
            else
            {
                nMinLum = nCurrLum;              
            }
            QThread::msleep(100);
        }

        QApplication::processEvents();
        QThread::msleep(1000);       
    }

    QSystem::closeMessage();
}

int LightCalibrationView::grayLevel(cv::Mat matImg)
{
    if (matImg.type() == CV_8UC3)
    {
        cvtColor(matImg, matImg, CV_RGB2GRAY);
    }  

    int m = matImg.rows;
    int n = matImg.cols;

    if (m == 0 || n == 0) return 0;

    long lGrayValue = 0;
    for (int y = 0; y < m; y++)
    {
        for (int x = 0; x < n; x++)
        {
            uchar& gray = matImg.at<uchar>(y, x);
            lGrayValue += gray;         
        }
    }
    return lGrayValue / (m*n);
}
