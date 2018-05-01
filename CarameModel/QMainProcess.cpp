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
	m_bCaptureDone = false;
	m_bCaptureLocker = false;
}

QMainProcess::~QMainProcess()
{
}

bool QMainProcess::startCapturing()
{
	if (m_pCameraCtrl->getCameraCount() <= 0) return false;

	//m_pCameraCtrl->getCamera(0)->startGrabing(getImageBufferNum());

	int nWaitTime = 30 * 60 * 100;
	while (m_bCaptureDone && (nWaitTime-- > 0) && !m_pCameraCtrl->getCamera(0)->isStopped())
	{
		QThread::msleep(10);
	}

	if (nWaitTime <= 0)
	{	
		System->setTrackInfo("wait get image timeout error!");
		return false;
	}

	m_bufferImages.clear();
	if (!m_pCameraCtrl->getCamera(0)->startGrabing(m_nCaptureNum))
	{
		System->setTrackInfo("startGrabing error!");
		return false;
	}

	nWaitTime = 2 * 1000;
	while (!m_pCameraCtrl->getCamera(0)->isGrabing() && (nWaitTime-- > 0))
	{
		QThread::msleep(1);
	}

	if (nWaitTime > 0)
	{
		return true;
	}

	return false;
}

bool QMainProcess::getImages(QVector<cv::Mat>& imageMats)
{
	bool bRet = true;

	if (m_pCameraCtrl->getCamera(0)->captureImageByFrameTrig(m_bufferImages))
	{
		saveImages(m_bufferImages);
	}
	else
	{
		bRet = false;
		System->setTrackInfo("Capture Images Stopped or Time Out.");
	}

	QAutoLocker loacker(&m_mutex);

	m_bCaptureDone = true;

	int nCaptureNum = m_bufferImages.size();
	if (!m_pCameraCtrl->getCamera(0)->isStopped() && (m_nCaptureNum != nCaptureNum))
	{
		System->setTrackInfo(QString("Camera Capture error, Image Num: %1").arg(nCaptureNum));

		QString capturePath = System->getParam("camera_cap_image_path").toString();
		QDateTime dtm = QDateTime::currentDateTime();
		QString fileDir = capturePath + "/" + dtm.toString("MMddhhmmss") + "/";
		QDir dir; dir.mkdir(fileDir);

		for (size_t i = 0; i < m_bufferImages.size(); ++i) {
			QString name = QString("%1").arg(i + 1, 2, 10, QChar('0')) + QStringLiteral(".bmp");
			cv::imwrite((fileDir + name).toStdString().c_str(), m_bufferImages[i]);
		}
		bRet = false;
	}

	for (int i = 0; i < nCaptureNum; i++)
	{
		imageMats.push_back(m_bufferImages[i]);
	}

	m_pCameraCtrl->getCamera(0)->stopGrabing();
	m_pCameraCtrl->getCamera(0)->clearGrabing();

	return bRet;
}

bool QMainProcess::getLastImages(QVector<cv::Mat>& imageMats)
{
	bool bRet = true;

	int nWaitTime = 10 * 100;
	while (!m_bCaptureDone && (nWaitTime-- > 0) && !m_pCameraCtrl->getCamera(0)->isStopped())
	{
		QThread::msleep(10);
	}

	QAutoLocker loacker(&m_mutex);

	if (nWaitTime > 0)
	{
		int nCaptureNum = m_bufferImages.size();
		for (int i = 0; i < nCaptureNum; i++)
		{
			imageMats.push_back(m_bufferImages[i]);
		}

		if (m_nCaptureNum != nCaptureNum) bRet = false;
	}
	else
	{
		bRet = false;
	}

	m_bCaptureDone = false;

	return bRet;
}

bool QMainProcess::stopCapturing()
{
	m_pCameraCtrl->getCamera(0)->stopGrabing();
	m_pCameraCtrl->getCamera(0)->clearGrabing();

	m_bCaptureDone = false;

	return true;
}

bool QMainProcess::isStartCapturing()
{
	return m_pCameraCtrl->getCamera(0)->isGrabing();
}

bool QMainProcess::lockCameraCapture()
{
	QAutoLocker loacker(&m_mutex);

	if (m_bCaptureLocker)
	{
		return false;
	}
	else
	{
		m_bCaptureLocker = true;
		return true;
	}
}

void QMainProcess::unlockCameraCapture()
{
	QAutoLocker loacker(&m_mutex);

	m_bCaptureLocker = false;
}

bool QMainProcess::isCameraCaptureAvaiable()
{
	QAutoLocker loacker(&m_mutex);

	return !m_bCaptureLocker;
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
	int nDlpMode = System->getParam("sys_run_dlp_mode").toInt();
	bool bMotionCardTrigger = (1 == nDlpMode);

	const int nLightCaptureNum = 6; // image num triggered by lighting IO 

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