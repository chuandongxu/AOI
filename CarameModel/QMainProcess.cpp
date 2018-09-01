#include "QMainProcess.h"

#include <QFileDialog>

#include "CameraCtrl.h"
#include "caramemodel_global.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IData.h"
#include "../include/IVision.h"
#include "../Common/ThreadPrioc.h"
#include "../Common/eos.h"

#include <QDebug>
#include <QMath.h>
#include <QMessageBox>
#include <qdatetime.h>
#include <QApplication>
#include <qthread.h>

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define ToInt(value)                (static_cast<int>(value))

QMainProcess::QMainProcess(CameraCtrl* pCameraCtrl, QObject *parent)
    : m_pCameraCtrl(pCameraCtrl), QObject(parent)
{
    m_emCaptureMode = ICamera::TRIGGER_ALL;
    m_nCaptureNum = DLP_SEQ_PATTERN_IMG_NUM;
    m_bHWTrigger = true;
}

QMainProcess::~QMainProcess()
{
}

bool QMainProcess::startCapturing()
{
    if (System->isRunOffline()) return true;

    if (m_pCameraCtrl->getCameraCount() <= 0) return false;
    
    m_imageMats.clear();
    if (!m_pCameraCtrl->getCamera(0)->startGrabing(m_nCaptureNum))
    {
        System->setTrackInfo("startGrabing error!");
        return false;
    }

    int nWaitTime = 2 * 1000;
    while (!m_pCameraCtrl->getCamera(0)->isGrabing() && (nWaitTime-- > 0))
    {
        QThread::msleep(1);
    }

    if (nWaitTime <= 0)
    {
        return false;
    }

    return true;
}

bool QMainProcess::getImages(QVector<cv::Mat>& imageMats)
{
    if (m_pCameraCtrl->getCamera(0)->captureImageByFrameTrig(m_imageMats))
    {
        bufferImages();
        saveImages(m_imageMats);
    }
    else
    {
        if (!isStopped())
        {
            System->setTrackInfo("Capture Images Time Out.");            
        }        
        return false;
    }

    m_waitMutex.lock();
    m_waitCon.wakeAll();
    m_waitMutex.unlock();

    int nCaptureNum = m_bufferImages.size();
    if (!m_pCameraCtrl->getCamera(0)->isStopped() && (m_nCaptureNum != nCaptureNum))
    {
        System->setTrackInfo(QString("Camera Capture error, Image Num: %1").arg(nCaptureNum));

        QString capturePath = System->getParam("camera_cap_image_path").toString();
        QDateTime dtm = QDateTime::currentDateTime();
        QString fileDir = capturePath + "/" + dtm.toString("MMddhhmmss") + "/";
        QDir dir; dir.mkdir(fileDir);

        for (size_t i = 0; i < m_bufferImages.size(); ++ i) {
            QString name = QString("%1").arg(i + 1, 2, 10, QChar('0')) + QStringLiteral(".bmp");
            cv::imwrite((fileDir + name).toStdString().c_str(), m_bufferImages[i]);
        }
        
        return false;
    }

    imageMats = m_bufferImages;

    m_pCameraCtrl->getCamera(0)->stopGrabing();
    m_pCameraCtrl->getCamera(0)->clearGrabing();

    return true;
}

bool QMainProcess::getLastImages(QVector<cv::Mat>& imageMats)
{
    bool bRet = true;

    m_waitMutex.lock();
    bool bWaitDone = m_waitCon.wait(&m_waitMutex, 10 * 1000);
    if (bWaitDone)
    {
        imageMats = m_bufferImages;
        if (m_nCaptureNum != m_bufferImages.size()) bRet = false;
    }
    else
    {
        QString msg = QString("Wait for image time out, wait for capture count ") + QString::number(m_nCaptureNum);
        System->setTrackInfo(msg);
        bRet = false;
    }    
    m_waitMutex.unlock();

    return bRet;
}

bool QMainProcess::stopCapturing()
{
    if (System->isRunOffline())
        return true;

    m_pCameraCtrl->getCamera(0)->stopGrabing();
    m_pCameraCtrl->getCamera(0)->clearGrabing();

    m_waitMutex.lock();
    m_waitCon.wakeAll();
    m_waitMutex.unlock();

    return true;
}

bool QMainProcess::isStartCapturing()
{
    return m_pCameraCtrl->getCamera(0)->isGrabing();
}

bool QMainProcess::isStopped()
{
    return m_pCameraCtrl->getCamera(0)->isStopped();
}

bool QMainProcess::startUpCapture(bool bHWTrigger)
{
    if (m_pCameraCtrl->getCameraCount() <= 0)
    {
        return false;
    }

    CameraDevice* pDev = m_pCameraCtrl->getCamera(0);
    if (pDev)
    {
        pDev->setHardwareTrigger(bHWTrigger);
    }

    selectCaptureMode(m_emCaptureMode, false);

    m_bHWTrigger = bHWTrigger;

    return true;
}

bool QMainProcess::isHWTrigger()
{
    return m_bHWTrigger;
}

bool QMainProcess::endUpCapture()
{
    if (m_pCameraCtrl->getCameraCount() <= 0)
    {
        return false;
    }

    stopCapturing();

    return true;
}

bool QMainProcess::selectCaptureMode(ICamera::TRIGGER emCaptureMode, bool reStartUp)
{
    const int nLightCaptureNum = 6; // image num triggered by lighting IO

    bool bMotionCardTrigger = System->isHardwareTrigger();
    if (bMotionCardTrigger)
    {
        int nDlpNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
        m_nCaptureNum = DLP_SEQ_PATTERN_IMG_NUM * nDlpNum;
        switch (emCaptureMode)
        {
        case  ICamera::TRIGGER_ALL:
            m_nCaptureNum = DLP_SEQ_PATTERN_IMG_NUM * nDlpNum + nLightCaptureNum;
            break;
        case  ICamera::TRIGGER_DLP_ALL:
            m_nCaptureNum = DLP_SEQ_PATTERN_IMG_NUM * nDlpNum;
            break;
        case  ICamera::TRIGGER_DLP:
            m_nCaptureNum = DLP_SEQ_PATTERN_IMG_NUM * 1;
            break;
        case  ICamera::TRIGGER_ONE:
            m_nCaptureNum = 1;
            break;
        case  ICamera::TRIGGER_LIGHT:
            m_nCaptureNum = nLightCaptureNum;
            break;
        default:
            break;
        }
    }
    else
    {
        m_nCaptureNum = DLP_SEQ_PATTERN_IMG_NUM;
    }

    if (reStartUp && (m_emCaptureMode != emCaptureMode))
    {
        stopCapturing();
        
        int nWaitTime = 5 * 100;
        while(!isStartCapturing() && (nWaitTime-- > 0))
        {
            QThread::msleep(10);
        }        
    }
    m_emCaptureMode = emCaptureMode;

    return true;
}

void QMainProcess::saveImages(QVector<cv::Mat>& images)
{
    if (!isHWTrigger()) return;

    //showImageToScreen(buffers.last());
    bool bCaptureImage = System->getParam("camera_cap_image_enable").toBool();
    //bool bCaptureSWImage = System->getParam("camera_cap_image_sw_enable").toBool();
    if (bCaptureImage)
    {
        QString capturePath = System->getParam("camera_cap_image_path").toString();

        QDateTime dtm = QDateTime::currentDateTime();
        QString fileDir = capturePath + "/" + dtm.toString("MMddhhmmss") + "/";
        QDir dir; dir.mkdir(fileDir);

        for (int i = 0; i < images.size(); i++)
        {
            cv::Mat image = images.at(i);

            QString name = QString("%1").arg(i + 1, 2, 10, QChar('0')) + QStringLiteral(".bmp");
            m_pCameraCtrl->getCamera(0)->saveImage(image, name, fileDir);
        }
    }
}

void QMainProcess::bufferImages()
{
    m_bufferImages.clear();
    m_bufferImages = m_imageMats;    
}