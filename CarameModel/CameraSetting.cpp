#include "CameraSetting.h"
#include <qfiledialog.h>
#include "../include/workflowDefine.h"
#include "../Common/eos.h"
#include "../Common/SystemData.h"
#include "../Common/ThreadPrioc.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/ICamera.h"
#include "../include/IVisionUI.h"
#include "../include/IMotion.h"
#include "../include/IDlp.h"
#include "../include/ILight.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMath.h>
#include <QMessageBox>
#include <QThread>

#include <opencv2/highgui/highgui.hpp>
#include "opencv2/calib3d.hpp"

#include "VisionAPI.h"

#include "../CurveEditor/IGraphicEditor.h"

#include "MatHeader.h"

using namespace AOI;

CameraSetting::CameraSetting(CameraCtrl* pCameraCtrl, QWidget *parent)
    : m_pCameraCtrl(pCameraCtrl), QWidget(parent)
{
    ui.setupUi(this);

    m_lastIndexMode = -1;
    m_lastIndexNum = -1;

    m_pGraphicsEditor = new IGraphicEditor();
    m_pGraphicsEditor->setViewPos(100, 0);
    m_pGraphicsEditor->setScale(1.0, 100);

    //相机设置
    QString capturePath = System->getParam("camera_cap_image_path").toString();
    if (capturePath.isEmpty())capturePath = QApplication::applicationDirPath() + "/capture";
    ui.lineEdit->setText(capturePath);

    QString imgDLP1Path = System->getParam("camera_cap_image_dlp1_path").toString();
    if (imgDLP1Path.isEmpty())imgDLP1Path = QApplication::applicationDirPath() + "/capture";
    ui.lineEdit_imgDLP1->setText(imgDLP1Path);

    QString imgDLP2Path = System->getParam("camera_cap_image_dlp2_path").toString();
    if (imgDLP2Path.isEmpty())imgDLP2Path = QApplication::applicationDirPath() + "/capture";
    ui.lineEdit_imgDLP2->setText(imgDLP2Path);

    bool bCaptureImage = System->getParam("camera_cap_image_enable").toBool();
    ui.checkBoxRecordAllImage->setChecked(bCaptureImage);

    bool bCaptureLightImage = System->getParam("camera_cap_image_light").toBool();
    ui.checkBoxCaptureLight->setChecked(bCaptureLightImage);

    bool bCaptureImageAsMatlab = System->getParam("camera_cap_image_matlab").toBool();
    ui.checkBoxCaptureAsMatlab->setChecked(bCaptureImageAsMatlab);

    QStringList ls;
    ls << QStringLiteral("Baslar相机采集") << QStringLiteral("Dalsa采集卡采集") << QStringLiteral("手动导入图片");
    ui.comboBox_captureMode->addItems(ls);
    int nCaptureMode = System->getParam("camera_capture_mode").toInt();
    ui.comboBox_captureMode->setCurrentIndex(nCaptureMode);
    connect(ui.comboBox_captureMode, SIGNAL(currentIndexChanged(int)), SLOT(onCaptureModeIndexChanged(int)));
    ui.groupBox_imgDLP->setEnabled(2 == nCaptureMode);

    ls.clear();
    ls << QStringLiteral("全部图像(DLP + Light)采集") << QStringLiteral("全部图像(12帧×DLP Num)采集") << QStringLiteral("标准图像(12帧)采集") << QStringLiteral("单帧采集") << QStringLiteral("灯光图像(6帧)采集");
    ui.comboBox_captureNumMode->addItems(ls);
    ui.comboBox_captureNumMode->setCurrentIndex(0);
    connect(ui.comboBox_captureNumMode, SIGNAL(currentIndexChanged(int)), SLOT(onCaptureNumModeIndexChanged(int)));

    updateUI();

    connect(ui.checkBoxRecordAllImage, SIGNAL(stateChanged(int)), SLOT(onStateChangeCapture(int)));
    connect(ui.checkBoxCaptureLight, SIGNAL(stateChanged(int)), SLOT(onStateChangeCaptureLight(int)));
    connect(ui.checkBoxCaptureAsMatlab, SIGNAL(stateChanged(int)), SLOT(onStateChangeCaptureAsMatlab(int)));

    bool bShowCross = System->getParam("camera_capture_cross_enable").toBool();
    int nShowCrossWidth = System->getParam("camera_capture_cross_width").toInt();
    ui.checkBox_crossEnable->setChecked(bShowCross);
    ui.lineEdit_crossGapWidth->setText(QString("%1").arg(nShowCrossWidth));
    connect(ui.checkBox_crossEnable, SIGNAL(stateChanged(int)), SLOT(onStateChangeCrossEnable(int)));

    connect(ui.pushButton, SIGNAL(clicked()), SLOT(onSelCapturePath()));
    connect(ui.pushButton_2, SIGNAL(clicked()), SLOT(onSaveCapturePath()));
    connect(ui.pushButton_imgDLP1Select, SIGNAL(clicked()), SLOT(onSelImgDLP1Path()));
    connect(ui.pushButton_imgDLP1Save, SIGNAL(clicked()), SLOT(onSaveImgDLP1Path()));
    connect(ui.pushButton_imgDLP2Select, SIGNAL(clicked()), SLOT(onSelImgDLP2Path()));
    connect(ui.pushButton_imgDLP2Save, SIGNAL(clicked()), SLOT(onSaveImgDLP2Path()));


    //采集操作
    ls.clear();
    ls << QStringLiteral("第1个DLP") << QStringLiteral("第2个DLP") << QStringLiteral("第3个DLP") << QStringLiteral("第4个DLP");
    ui.comboBox_selectDLP->addItems(ls);
    ls.clear();
    ls << QStringLiteral("第一圈红") << QStringLiteral("第一圈白") << QStringLiteral("第二圈绿") << QStringLiteral("第三圈蓝") << QStringLiteral("第四圈白") << QStringLiteral("第四圈蓝");
    ui.comboBox_selectLight->addItems(ls);
    connect(ui.comboBox_selectLight, SIGNAL(currentIndexChanged(int)), SLOT(onSelectLightIndexChanged(int)));
    connect(ui.pushButton_startSetupDLP, SIGNAL(clicked()), SLOT(onStartSetupDLP()));
    connect(ui.pushButton_endSetupDLP, SIGNAL(clicked()), SLOT(onEndSetupDLP()));
    connect(ui.pushButton_captureDLP, SIGNAL(clicked()), SLOT(onCaptureDLP()));
    connect(ui.pushButton_captureLight, SIGNAL(clicked()), SLOT(onCaptureLight()));

    //相机标定
    ui.lineEdit_2->setText(QString("%1").arg(15));
    ui.lineEdit_3->setText(QString("%1").arg(15));
    ui.lineEdit_4->setText(QString("%1").arg(2.0));
    ui.lineEdit_7->setText(QString("%1").arg(90));
    ui.lineEdit_8->setText(QString("%1").arg(10));

    connect(ui.pushButton_3, SIGNAL(clicked()), SLOT(onCalibration()));

    double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    double dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    ui.lineEdit_5->setText(QString("%1").arg(dResolutionX));
    ui.lineEdit_6->setText(QString("%1").arg(dResolutionY));

    //输入限制
    inputIntRangePos = new QIntValidator(0, 1000, this);
    inputIntRangeAll = new QIntValidator(-1000, 1000, this);
    inputDoubleRangePos = new QDoubleValidator(0, 1000, 10, this);
    inputDoubleRangeAll = new QDoubleValidator(-1000, 1000, 10, this);

    ui.lineEdit_2->setValidator(inputIntRangePos);
    ui.lineEdit_3->setValidator(inputIntRangePos);
    ui.lineEdit_4->setValidator(inputIntRangePos);
    ui.lineEdit_7->setValidator(inputDoubleRangePos);
    ui.lineEdit_8->setValidator(inputDoubleRangePos);
    ui.lineEdit_crossGapWidth->setValidator(inputIntRangePos);

    QString user;
    int level = 0;
    System->getUser(user, level);
    if (USER_LEVEL_TECH > level)
    {
        ui.checkBoxCaptureAsMatlab->setEnabled(false);
    }
}

CameraSetting::~CameraSetting()
{
    if (inputIntRangePos)
    {
        delete inputIntRangePos;
        inputIntRangePos = NULL;
    }

    if (inputIntRangeAll)
    {
        delete inputIntRangeAll;
        inputIntRangeAll = NULL;
    }

    if (inputDoubleRangePos)
    {
        delete inputDoubleRangePos;
        inputDoubleRangePos = NULL;
    }

    if (inputDoubleRangeAll)
    {
        delete inputDoubleRangeAll;
        inputDoubleRangeAll = NULL;
    }

    if (m_pGraphicsEditor)
    {
        delete m_pGraphicsEditor;
        m_pGraphicsEditor = NULL;
    }
}

QWidget* CameraSetting::getCaliTab()
{
    return ui.tab_3;
}

void CameraSetting::onStateChangeCapture(int iState)
{
    int data = 0;
    if (Qt::Checked == iState) data = 1;

    System->setParam("camera_cap_image_enable", (bool)data);
}

void CameraSetting::onStateChangeCaptureLight(int iState)
{
    int data = 0;
    if (Qt::Checked == iState) data = 1;

    System->setParam("camera_cap_image_light", (bool)data);
}

void CameraSetting::onStateChangeCaptureAsMatlab(int iState)
{
    int data = 0;
    if (Qt::Checked == iState) data = 1;

    System->setParam("camera_cap_image_matlab", (bool)data);
}

void CameraSetting::onStateChangeCrossEnable(int iState)
{
    int data = 0;
    if (Qt::Checked == iState) data = 1;

    int nCrossGap = ui.lineEdit_crossGapWidth->text().toInt();

    System->setParam("camera_capture_cross_enable", (bool)data);
    System->setParam("camera_capture_cross_width", nCrossGap);
}

void CameraSetting::onCaptureModeIndexChanged(int iIndex)
{
    int nCaptureMode = ui.comboBox_captureMode->currentIndex();

    ui.groupBox_imgDLP->setEnabled(2 == nCaptureMode);

    System->setParam("camera_capture_mode", nCaptureMode);

    QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("重启软件，相机采集模式才可以应用！"));
}

void CameraSetting::onCaptureNumModeIndexChanged(int iIndex)
{
    int nCaptureNumMode = ui.comboBox_captureNumMode->currentIndex();

    if (nCaptureNumMode == 2) // disable 12 images captured
    {
        ui.comboBox_captureNumMode->setCurrentIndex(m_lastIndexNum);
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("目前不支持12副图像采集，请选择其他模式！"));
        return;
    }
    else
    {
        m_lastIndexNum = nCaptureNumMode;
    }
    

    ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
    if (pCam)
    {        
        pCam->selectCaptureMode((ICamera::TRIGGER)nCaptureNumMode, true);
    }

    updateUI();
}

void CameraSetting::updateUI()
{
    int nCaptureNumMode = ui.comboBox_captureNumMode->currentIndex();

    ui.comboBox_selectDLP->setEnabled(false);
    ui.pushButton_captureDLP->setEnabled(false);
    ui.comboBox_selectLight->setEnabled(false);
    ui.pushButton_captureLight->setEnabled(false);

    switch (nCaptureNumMode)
    {
    case 0:
        ui.pushButton_captureDLP->setEnabled(true);
        break;
    case 1:
        ui.pushButton_captureDLP->setEnabled(true);
        break;
    case 2:
        ui.comboBox_selectDLP->setEnabled(true);
        ui.pushButton_captureDLP->setEnabled(true);
        break;
    case 3:
        ui.comboBox_selectLight->setEnabled(true);
        ui.pushButton_captureLight->setEnabled(true);
        break;
    case 4:
        ui.pushButton_captureLight->setEnabled(true);
        break;
    default:
        break;
    }
}

void CameraSetting::onSelCapturePath()
{
    QString str = QFileDialog::getExistingDirectory();
    if (!str.isEmpty())ui.lineEdit->setText(str);
}

void CameraSetting::onSaveCapturePath()
{
    QString str = ui.lineEdit->text();
    if (!str.isEmpty())System->setParam("camera_cap_image_path", str);
}

void CameraSetting::onSelImgDLP1Path()
{
    QString str = QFileDialog::getExistingDirectory();
    if (!str.isEmpty())ui.lineEdit_imgDLP1->setText(str);
}

void CameraSetting::onSaveImgDLP1Path()
{
    QString str = ui.lineEdit_imgDLP1->text();
    if (!str.isEmpty())System->setParam("camera_cap_image_dlp1_path", str);
}

void CameraSetting::onSelImgDLP2Path()
{
    QString str = QFileDialog::getExistingDirectory();
    if (!str.isEmpty())ui.lineEdit_imgDLP2->setText(str);
}

void CameraSetting::onSaveImgDLP2Path()
{
    QString str = ui.lineEdit_imgDLP2->text();
    if (!str.isEmpty())System->setParam("camera_cap_image_dlp2_path", str);
}

void CameraSetting::onCalibration()
{
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    if (!pUI) return;

    int nRow = ui.lineEdit_2->text().toInt();
    int nCol = ui.lineEdit_3->text().toInt();
    double dDist = ui.lineEdit_4->text().toDouble();
    double dScore = ui.lineEdit_7->text().toDouble();
    double dPixelOffsDiff = ui.lineEdit_8->text().toDouble();

    cv::Mat matImage = pUI->getImage();
    if (!matImage.empty())
    {
        Vision::PR_CALIBRATE_CAMERA_CMD  stCmd;
        stCmd.matInputImg = matImage;
        stCmd.szBoardPattern = cv::Size(nCol, nRow);
        stCmd.fPatternDist = dDist;
        stCmd.fMinTmplMatchScore = dScore;

        pUI->clearImage();

        Vision::PR_CALIBRATE_CAMERA_RPY stRpy;
        Vision::VisionStatus retStatus = Vision::PR_CalibrateCamera(&stCmd, &stRpy);
        if (retStatus == Vision::VisionStatus::OK)
        {
            double dResolutionX = stRpy.dResolutionX;
            double dResolutionY = stRpy.dResolutionY;

            ui.lineEdit_5->setText(QString("%1").arg(dResolutionX));
            ui.lineEdit_6->setText(QString("%1").arg(dResolutionY));

            System->setSysParam("CAM_RESOLUTION_X", dResolutionX);
            System->setSysParam("CAM_RESOLUTION_Y", dResolutionY);

            std::vector<aaAaa::aaSpline> splines;

            cv::Mat rvecIni;
            cv::Mat matRotionIni(stRpy.matInitialExtrinsicMatrix, cv::Rect(0, 0, 3, 3));
            cv::Rodrigues(matRotionIni, rvecIni);

            cv::Mat tvecIni(stRpy.matInitialExtrinsicMatrix, cv::Rect(3, 0, 1, 3));
            std::vector<cv::Point2f> vecProjectPointsIni;
            cv::projectPoints(stRpy.vecObjectPoints, rvecIni, tvecIni, stRpy.matInitialIntrinsicMatrix, stRpy.matDistCoeffs, vecProjectPointsIni);

            aaAaa::aaSpline splineXIni, splineYIni;
            for (int i = 0; i < stRpy.vecImagePoints.size(); i++)
            {
                cv::Point2f ptImage = stRpy.vecImagePoints[i];
                cv::Point2f ptProject = vecProjectPointsIni[i];

                splineXIni.addKnots(aaAaa::aaPoint(i, (ptImage.x - ptProject.x)));
                splineYIni.addKnots(aaAaa::aaPoint(i, (ptImage.y - ptProject.y)));
            }
            splineXIni.name = "Initial X";
            splines.push_back(splineXIni);
            splineYIni.name = "Initial Y";
            //spline.setLimit(0, 255, 255, 0);
            splines.push_back(splineYIni);

            QString runPath = QCoreApplication::applicationDirPath() + "/output/";

            QString fileName = runPath + "matInitialIntrinsicMatrix" + ".txt";
            printfMatToFile<double>(stRpy.matInitialIntrinsicMatrix, fileName);

            fileName = runPath + "matInitialExtrinsicMatrix" + ".txt";
            printfMatToFile<double>(stRpy.matInitialExtrinsicMatrix, fileName);

            fileName = runPath + "matIntrinsicMatrix" + ".txt";
            printfMatToFile<double>(stRpy.matIntrinsicMatrix, fileName);

            fileName = runPath + "matExtrinsicMatrix" + ".txt";
            printfMatToFile<double>(stRpy.matExtrinsicMatrix, fileName);

            fileName = runPath + "matDistCoeffs" + ".txt";
            printfMatToFile<double>(stRpy.matDistCoeffs, fileName);

            fileName = runPath + "vecObjectPoints" + ".txt";
            printf3fVectorToFile(stRpy.vecObjectPoints, fileName);

            fileName = runPath + "vecImagePoints" + ".txt";
            printf2fVectorToFile(stRpy.vecImagePoints, fileName);

            cv::Mat rvec;
            cv::Mat matRotion(stRpy.matExtrinsicMatrix, cv::Rect(0, 0, 3, 3));
            cv::Rodrigues(matRotion, rvec);

            cv::Mat tvec(stRpy.matExtrinsicMatrix, cv::Rect(3, 0, 1, 3));
            std::vector<cv::Point2f> vecProjectPoints;
            cv::projectPoints(stRpy.vecObjectPoints, rvec, tvec, stRpy.matIntrinsicMatrix, stRpy.matDistCoeffs, vecProjectPoints);

            aaAaa::aaSpline splineX, splineY;
            for (int i = 0; i < stRpy.vecImagePoints.size(); i++)
            {
                cv::Point2f ptImage = stRpy.vecImagePoints[i];
                cv::Point2f ptProject = vecProjectPoints[i];

                splineX.addKnots(aaAaa::aaPoint(i, (ptImage.x - ptProject.x)));
                splineY.addKnots(aaAaa::aaPoint(i, (ptImage.y - ptProject.y)));

                if (qAbs(ptImage.x - ptProject.x) > dPixelOffsDiff || qAbs(ptImage.y - ptProject.y) > dPixelOffsDiff)
                {
                    int nRectSize = convertToPixel(dDist * 1000 / 2);
                    cv::Rect select;
                    select.x = ptImage.x - nRectSize / 2;
                    select.y = ptImage.y - nRectSize / 2;
                    select.width = nRectSize;
                    select.height = nRectSize;
                    cv::rectangle(stRpy.matCornerPointsImg, select, cv::Scalar(0, 255, 255), 3, 8, 0);
                }
            }
            splineX.name = "Corrected X";
            splines.push_back(splineX);
            splineY.name = "Corrected Y";
            //spline.setLimit(0, 255, 255, 0);
            splines.push_back(splineY);

            m_pGraphicsEditor->setSplines(splines);
            m_pGraphicsEditor->show();

            pUI->setImage(stRpy.matCornerPointsImg);
        }
        else
        {
            ui.lineEdit_5->setText(QString("%1").arg(0.0));
            ui.lineEdit_6->setText(QString("%1").arg(0.0));
            pUI->addImageText(QString("Error at Calibration, error code = %1").arg((int)retStatus));
        }
    }
    else
    {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
    }

}

void CameraSetting::onStartSetupDLP()
{
    IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
    if (pDlp)
    {
        int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
        for (int i = 0; i < nStationNum; i++)
        {
            if (pDlp->isConnected(i))
            {
                if (!pDlp->startUpCapture(i)) continue;
            }
        }
    }
}

void CameraSetting::onEndSetupDLP()
{
    IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
    if (pDlp)
    {
        int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
        for (int i = 0; i < nStationNum; i++)
        {
            if (pDlp->isConnected(i))
            {
                if (!pDlp->endUpCapture(i)) continue;
            }
        }
    }
}

void CameraSetting::onCaptureDLP()
{
    QAutoLocker loacker(&m_mutex);

    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion) return;

    ILight* pLight = getModule<ILight>(LIGHT_MODEL);
    if (!pLight) return;

    int nCaptureNumMode = ui.comboBox_captureNumMode->currentIndex();
    int nSelectDLP = ui.comboBox_selectDLP->currentIndex();

    bool bClearSteup = (m_lastIndexMode != nCaptureNumMode);
    m_lastIndexMode = nCaptureNumMode;

    switch (nCaptureNumMode)
    {
    case 0:
    {
        bool bTriggerBoard = System->isTriggerBoard();
        if (bTriggerBoard)
        {
            if (!pLight->triggerCapturing(ILight::TRIGGER_ALL, true, bClearSteup))
            {
                System->setTrackInfo(QString("triggerCapturing error!"));
            }
        }
        else
        {
            if (!pMotion->triggerCapturing(IMotion::TRIGGER_ALL, true, bClearSteup))
            {
                System->setTrackInfo(QString("triggerCapturing error!"));
            }
        }
    }
    break;
    case 1:
    {
        bool bTriggerBoard = System->isTriggerBoard();
        if (bTriggerBoard)
        {
            if (!pLight->triggerCapturing(ILight::TRIGGER_DLP, true, bClearSteup))
            {
                System->setTrackInfo(QString("triggerCapturing error!"));
            }
        }
        else
        {
            if (!pMotion->triggerCapturing(IMotion::TRIGGER_DLP, true, bClearSteup))
            {
                System->setTrackInfo(QString("triggerCapturing error!"));
            }
        }
    }
    break;
    case 2:
    {
        bool bTriggerBoard = System->isTriggerBoard();
        if (bTriggerBoard)
        {
            if (!pLight->triggerCapturing(ILight::TRIGGER(ILight::TRIGGER_DLP1 + nSelectDLP), true, bClearSteup))
            {
                System->setTrackInfo(QString("triggerCapturing error!"));
            }
        }
        else
        {
            if (!pMotion->triggerCapturing(IMotion::TRIGGER(IMotion::TRIGGER_DLP1 + nSelectDLP), true, bClearSteup))
            {
                System->setTrackInfo(QString("triggerCapturing error!"));
            }
        }
    }
    break;
    default:
        break;
    }

    ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
    if (pCam)
    {
        QVector<cv::Mat> matImgs;
        if (!pCam->getLastImages(matImgs))
        {
            System->setTrackInfo(QString("getImages error"));
            return;
        }
    }
}

void CameraSetting::onSelectLightIndexChanged(int iIndex)
{
    int nIndex = ui.comboBox_selectLight->currentIndex();
    m_lastIndexMode = -1;
}

void CameraSetting::onCaptureLight()
{
    QAutoLocker loacker(&m_mutex);

    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion) return;

    ILight* pLight = getModule<ILight>(LIGHT_MODEL);
    if (!pLight) return;

    int nCaptureNumMode = ui.comboBox_captureNumMode->currentIndex();
    int nSelectLight = ui.comboBox_selectLight->currentIndex();

    bool bClearSteup = (m_lastIndexMode != nCaptureNumMode);
    m_lastIndexMode = nCaptureNumMode;

    switch (nCaptureNumMode)
    {
    case 3:
    {

        bool bTriggerBoard = System->isTriggerBoard();
        if (bTriggerBoard)
        {          
            if (!pLight->triggerCapturing(ILight::TRIGGER(ILight::TRIGGER_ONE_CH1 + nSelectLight), true, bClearSteup))
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
    }
    break;
    case 4:
    {
        bool bTriggerBoard = System->isTriggerBoard();
        if (bTriggerBoard)
        {
            if (!pLight->triggerCapturing(ILight::TRIGGER_LIGHT, true, bClearSteup))
            {
                System->setTrackInfo(QString("triggerCapturing error!"));
            }
        }
        else
        {
            if (!pMotion->triggerCapturing(IMotion::TRIGGER_LIGHT, true, bClearSteup))
            {
                System->setTrackInfo(QString("triggerCapturing error!"));
            }
        }
    }
    break;
    default:
        break;
    }

    ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
    if (pCam)
    {
        QVector<cv::Mat> matImgs;
        if (!pCam->getLastImages(matImgs))
        {
            System->setTrackInfo(QString("getImages error"));
            return;
        }
    }
}

double CameraSetting::convertToPixel(double umValue)
{
    double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    double dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    double dResolution = (dResolutionX + dResolutionY) / 2;

    if (qAbs(dResolution) > 0.0001)
    {
        return (umValue / dResolution);
    }
    return 0.0;
}

double CameraSetting::convertToUm(double pixel)
{
    double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    double dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    double dResolution = (dResolutionX + dResolutionY) / 2;

    return pixel*dResolution;
}
