#include <QDialog>

#include "DLPTableCalibrationView.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IMotion.h"
#include "../include/ICamera.h"
#include "../include/constants.h"

#include "../DataModule/DataUtils.h"
#include "ScanImageThread.h"
#include "DLPTableCalibrationRstWidget.h"

DLPTableCalibrationView::DLPTableCalibrationView(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    m_pDisplayWidget = std::make_shared<DLPTableCalibrationRstWidget>();
}

DLPTableCalibrationView::~DLPTableCalibrationView()
{
}

void DLPTableCalibrationView::showEvent(QShowEvent *event) 
{
    ui.btnBtmRight->setEnabled(false);
    _displayResult();
}

void DLPTableCalibrationView::on_btnTopLeft_clicked() 
{
    _showJoyStick();

    auto nReturn = System->showInteractMessage(QStringLiteral("���ò�������"), QStringLiteral("���ƶ�XY Tableֱ��������Ķ�׼�������Ͻ�"));
    if (nReturn != QDialog::Accepted)
        return;

    ui.btnBtmRight->setEnabled(true);

    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    pMotion->getCurrentPos(AXIS_MOTOR_X, &m_dLeftX);
    pMotion->getCurrentPos(AXIS_MOTOR_Y, &m_dTopY);

    m_dLeftX *= MM_TO_UM;
    m_dTopY *= MM_TO_UM;

    _saveResult();
    _displayResult();
}

void DLPTableCalibrationView::on_btnBtmRight_clicked() 
{
    _showJoyStick();

    auto nReturn = System->showInteractMessage(QStringLiteral("���ò�������"), QStringLiteral("���ƶ�XY Tableֱ��������Ķ�׼�������½�"));
    if (nReturn != QDialog::Accepted)
        return;

    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    pMotion->getCurrentPos(AXIS_MOTOR_X, &m_dRightX);
    pMotion->getCurrentPos(AXIS_MOTOR_Y, &m_dBottomY);

    m_dRightX *= MM_TO_UM;
    m_dBottomY *= MM_TO_UM;

    if (m_dRightX <= m_dLeftX) {
        System->showMessage(QStringLiteral("���ò�������"), QStringLiteral("�����ұ߽����� (%1) С����߽����� (%2). ����������.").arg(m_dRightX).arg(m_dLeftX));
        return;
    }

    if (m_dTopY < m_dBottomY) {
        System->showMessage(QStringLiteral("���ò�������"), QStringLiteral("�����ϱ߽����� (%1) С���±߽����� (%2). ����������.").arg(m_dTopY).arg(m_dBottomY));
        return;
    }

    _saveResult();
    _displayResult();
}

void DLPTableCalibrationView::on_btnPrepareScanImage_clicked() 
{
    _displayResult();

    if (m_dRightX <= m_dLeftX || m_dTopY < m_dBottomY) {
        System->showMessage(QStringLiteral("���ü������"), QStringLiteral("��������������ȷ."));
        return;
    }

    m_vecVecFrameCtr.clear();
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto pCamera = getModule<ICamera>(CAMERA_MODEL);
    int nImageWidth = 0, nImageHeight = 0;
    pCamera->getCameraScreenSize(nImageWidth, nImageHeight);
    float fovWidth = nImageWidth  * dResolutionX;
    float fovHeight = nImageHeight * dResolutionY;
    float overlapX = 0.f, overlapY = 0.f;

    int result = DataUtils::assignFrames(m_dLeftX, m_dTopY, m_dRightX, m_dBottomY, fovWidth, fovHeight, m_vecVecFrameCtr, overlapX, overlapY);
    auto frameCountX = m_vecVecFrameCtr[0].size();
    auto frameCountY = m_vecVecFrameCtr.size();

    ui.lineEditFrameCountXScan->setText(QString::number(frameCountX));
    ui.lineEditFrameCountYScan->setText(QString::number(frameCountY));

    m_nFrameCountX = frameCountX;
    m_nFrameCountY = frameCountY;

    //ui.btnScanImage->setEnabled(true);
}

void DLPTableCalibrationView::on_btnScanImage_clicked() 
{
    System->showMessage(QStringLiteral("ɨ����"), QStringLiteral("����ɨ�����У���Ⱥ�..."), 1);   

    m_pScanImageThread = new ScanImageThread(m_vecVecFrameCtr, m_dLeftX*UM_TO_MM, m_dTopY*UM_TO_MM, m_dRightX*UM_TO_MM, m_dBottomY*UM_TO_MM);
    connect(m_pScanImageThread, &ScanImageThread::finished, this, &DLPTableCalibrationView::on_scanImage_done);
    m_pScanImageThread->start();
}

void DLPTableCalibrationView::on_scanImage_done()
{
    if (m_pScanImageThread->isGood()) 
    {
        Vision::VectorOfVectorOfFloat frameChartData = m_pScanImageThread->getFrameChartData();

        m_pDisplayWidget->setFrameChartData(frameChartData);
        m_pDisplayWidget->show();
    }

    System->closeMessage();

    delete m_pScanImageThread;
    m_pScanImageThread = NULL;
}

void DLPTableCalibrationView::_saveResult()
{
    System->setParam("dlp_table_cali_pos_left", m_dLeftX);
    System->setParam("dlp_table_cali_pos_top", m_dTopY);
    System->setParam("dlp_table_cali_pos_right", m_dRightX);
    System->setParam("dlp_table_cali_pos_bottom", m_dBottomY);
}

void DLPTableCalibrationView::_displayResult() 
{
    float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f;

    left = System->getParam("dlp_table_cali_pos_left").toDouble();
    top = System->getParam("dlp_table_cali_pos_top").toDouble();
    right = System->getParam("dlp_table_cali_pos_right").toDouble();
    bottom = System->getParam("dlp_table_cali_pos_bottom").toDouble();

    m_dLeftX = left;
    m_dTopY = top;
    m_dRightX = right;
    m_dBottomY = bottom;

    QString strBoardLeftTop, strBoardRightBtm;
    strBoardLeftTop.sprintf("%.2f, %.2f", left, top);
    ui.lineEditBoardLeftTop->setText(strBoardLeftTop);
    strBoardRightBtm.sprintf("%.2f, %.2f", right, bottom);
    ui.lineEditBoardRightBtm->setText(strBoardRightBtm);
    ui.lineEditBoardWidth->setText(QString::number(right - left));
    ui.lineEditBoardHeight->setText(QString::number(top - bottom));
}

void DLPTableCalibrationView::_showJoyStick() 
{
    IMotion *pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion)
        return;

    pMotion->setJoystickXMotor(AXIS_MOTOR_X, 0.2, NULL);
    pMotion->setJoystickYMotor(AXIS_MOTOR_Y, 0.2, NULL);

    pMotion->startJoystick();
}
