#include "QMainProcess.h"

#include "CameraCtrl.h"
#include "QMainCameraOnLive.h"
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

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define ToInt(value)                (static_cast<int>(value))

QMainProcess::QMainProcess(CameraCtrl* pCameraCtrl, QObject *parent)
	: m_pCameraCtrl(pCameraCtrl), QObject(parent)
{
	m_nCaptureNum = DLP_SEQ_PATTERN_IMG_NUM;
	m_bCaptureDone = false;
	m_bCaptureLocker = false;

	m_pCameraOnLive = NULL;
}

QMainProcess::~QMainProcess()
{
}

void QMainProcess::pushImageBuffer(cv::Mat& matImage)
{
	m_bufferImages.push_back(matImage);
}

void QMainProcess::setImageBuffer(QVector<cv::Mat>& matImages)
{
	for (int i = 0; i < matImages.size(); i++)
	{
		m_bufferImages.push_back(matImages[i]);
	}
}

const QVector<cv::Mat>& QMainProcess::getImageBuffer()
{
	return m_bufferImages;
}

const cv::Mat& QMainProcess::getImageItemBuffer(int nIndex)
{
	return (m_bufferImages[nIndex]);
}

int	QMainProcess::getImageBufferNum()
{
	return m_nCaptureNum;
}

int QMainProcess::getImageBufferCaptureNum()
{
	return m_bufferImages.size();
}

void QMainProcess::clearImageBuffer()
{
	m_bufferImages.clear();
	m_bCaptureDone = false;
}

bool QMainProcess::startCapturing()
{
	if (m_pCameraCtrl->getCameraCount() <= 0) return false;

	//m_pCameraCtrl->getCamera(0)->startGrabing(getImageBufferNum());	

	clearImageBuffer();

	int nWaitTime = 2 * 1000;
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

void QMainProcess::setCaptureImageBufferDone()
{
	m_bCaptureDone = true;
}

bool QMainProcess::isCaptureImageBufferDone()
{
	return m_bCaptureDone;
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

bool QMainProcess::startUpCapture()
{
	if (m_pCameraCtrl->getCameraCount() <= 0)
	{
		return false;
	}

	if (!m_pCameraOnLive)
	{
		int nCaptureNumMode = System->getParam("camera_capture_num_mode").toInt();
		selectCaptureMode(static_cast<ICamera::TRIGGER>(nCaptureNumMode));	

		//System->setParam("camera_cap_image_sw_enable", true);

		m_pCameraOnLive = new MainCameraOnLive(this, m_pCameraCtrl->getCamera(0));
		m_pCameraOnLive->start();
	}

	return true;
}

bool QMainProcess::endUpCapture()
{
	if (m_pCameraCtrl->getCameraCount() <= 0)
	{
		return false;
	}

	if (m_pCameraOnLive)
	{
		m_pCameraCtrl->getCamera(0)->stopGrabing();

		m_pCameraOnLive->setQuitFlag();
		while (m_pCameraOnLive->isRuning())
		{
			QThread::msleep(10);
			QApplication::processEvents();
		}
		QThread::msleep(200);
		delete m_pCameraOnLive;
		m_pCameraOnLive = NULL;
	}

	return true;
}

bool QMainProcess::selectCaptureMode(ICamera::TRIGGER emCaptureMode)
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

	return true;
}