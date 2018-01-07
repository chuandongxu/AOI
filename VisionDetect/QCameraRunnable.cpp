#include "QCameraRunnable.h"
#include "../Common/SystemData.h"

#include "VisionDetectRunView.h"

#include <time.h>
#include <qthread.h>
#include <qthreadpool.h>

QCameraRunnable::QCameraRunnable(VisionDetectRunView* pSetting)
	: m_pSetting(pSetting), QRunnable()
{
	m_exit = false;
	m_bRunning = false;
	m_bCapturEnable = false;
	m_bCapturing = false;
}

QCameraRunnable::~QCameraRunnable()
{
}

void QCameraRunnable::quit()
{
	m_exit = true;
}

bool QCameraRunnable::isRunning()
{
	return m_bRunning;
}

void QCameraRunnable::startCapture()
{
	m_bCapturEnable = true;
}

void QCameraRunnable::stopCapture()
{
	m_bCapturEnable = false;
}

bool QCameraRunnable::isCapturing()
{
	return m_bCapturing;
}

void QCameraRunnable::run()
{
	System->setTrackInfo(QString(QStringLiteral("视频采集启动成功...")));
	
	m_bRunning = true;
	while (!isExit())
	{
		QThread::msleep(200);

		if (!waitStartBtn())continue;
		if (isExit())break;

		if (!captureImages())continue;
		if (isExit())break;

		if (!waitCheckDone())continue;
		if (isExit())break;
	}
	m_bRunning = false;
	
	System->setTrackInfo(QString(QStringLiteral("视频采集已停止...")));
}

bool QCameraRunnable::waitStartBtn()
{
	while (1)
	{
		if (m_bCapturEnable)
		{
			m_bCapturing = true;
			return true;
		}

		if (isExit()) return false;
		QThread::msleep(50);
	}

	return true;
}

bool QCameraRunnable::captureImages()
{
	if (m_pSetting)
	{
		m_pSetting->guideDisplayImages();
	}
	
	return true;
}

bool QCameraRunnable::waitCheckDone()
{
	m_bCapturing = false;
	return true;
}

bool QCameraRunnable::isExit()
{
	return m_exit;
}
