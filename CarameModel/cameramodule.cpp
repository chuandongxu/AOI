#include "cameramodule.h"
#include "CameraCtrl.h"
#include "../Common/SystemData.h"
#include "qlist.h"
#include "QMainProcess.h"
#include "CameraSetting.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

CameraModule::CameraModule(int id, const QString &name)
	:QModuleBase(id, name)
{
	initial();
}

void CameraModule::setErrorMap()
{
	System->addErrorMap(ERROR_GRABIMAGE, MSG_ERROR_GRABIMAGE);
}


CameraModule::~CameraModule()
{
	unInit();
}

void CameraModule::initial(int nWindow)
{
	// 相机设备初始化
	m_pCameraCtrl = NULL;
	m_pCameraCtrl = new CameraCtrl();
	m_pCameraCtrl->initial();

	m_pMainProcess = QSharedPointer<QMainProcess>(new QMainProcess(m_pCameraCtrl));

	m_pSetting = new CameraSetting(m_pCameraCtrl);

	// 错误码
	setErrorMap();

	System->setParam("camera_hw_tri_enable", true);
}

void CameraModule::unInit()
{
	if (m_pCameraCtrl)
	{
		m_pCameraCtrl->unInit();
		delete m_pCameraCtrl;
		m_pCameraCtrl = NULL;
	}
}

QWidget* CameraModule::getCalibrationView()
{
	return m_pSetting->getCaliTab();
}

void CameraModule::openCamera()
{
	return;
}

void CameraModule::closeCamera()
{
	return;
}

int CameraModule::getCameraNum()
{
	if (m_pCameraCtrl)
	{
		return m_pCameraCtrl->getCameraCount();
	}

	return 0;
}

bool CameraModule::startUpCapture()
{
	if (m_pMainProcess)
	{
		return m_pMainProcess->startUpCapture();
	}

	return false;
}

bool CameraModule::endUpCapture()
{	
	if (m_pMainProcess)
	{
		return m_pMainProcess->endUpCapture();
	}
	return false;
}

bool CameraModule::selectCaptureMode(TRIGGER emCaptureMode)
{
	if (m_pMainProcess)
	{
		return m_pMainProcess->selectCaptureMode(emCaptureMode);
	}
	return false;
}

const QVector<cv::Mat>& CameraModule::getImageBuffer()
{
	return m_pMainProcess->getImageBuffer();
}

const cv::Mat& CameraModule::getImageItemBuffer(int nIndex)
{
	return m_pMainProcess->getImageItemBuffer(nIndex);
}

int CameraModule::getImageBufferNum()
{	
	return m_pMainProcess->getImageBufferNum();
}

int CameraModule::getImageBufferCaptureNum()
{
	return m_pMainProcess->getImageBufferCaptureNum();
}

bool CameraModule::startCapturing()
{
	return m_pMainProcess->startCapturing();
}

void CameraModule::clearImageBuffer()
{
	m_pMainProcess->clearImageBuffer();
}

bool CameraModule::isCaptureImageBufferDone()
{
	return m_pMainProcess->isCaptureImageBufferDone();
}

bool CameraModule::lockCameraCapture()
{
	return m_pMainProcess->lockCameraCapture();
}

void CameraModule::unlockCameraCapture()
{
	m_pMainProcess->unlockCameraCapture();
}

bool CameraModule::isCameraCaptureAvaiable()
{
	return m_pMainProcess->isCameraCaptureAvaiable();
}

bool CameraModule::getCameraScreenSize(int& nWidth, int& nHeight)
{
	CameraCtrl * ctrlTmp = (CameraCtrl*)m_pCameraCtrl;
	if (ctrlTmp)
	{
		CameraDevice * tmpDevice = ctrlTmp->getCamera(0);
		if (tmpDevice)
		{
			return tmpDevice->getCameraScreenSize(nWidth, nHeight);
		}
	}

	nWidth = 0;
	nHeight = 0;
	return false;
}

bool CameraModule::grabCamImage(int nCamera, cv::Mat& image, bool bSync)
{
	CameraCtrl * ctrlTmp = (CameraCtrl*)m_pCameraCtrl;
	if (ctrlTmp)
	{
		CameraDevice * tmpDevice = ctrlTmp->getCamera(nCamera);
		if (tmpDevice)
		{
			return tmpDevice->captureImage(image);
		}
	}

	return false;
}

void CameraModule::setExposureTime(int nCamera, double exposureTime)
{
	/*****/
	CameraCtrl * ctrlTmp = (CameraCtrl*)m_pCameraCtrl;
	if (ctrlTmp)
	{
		CameraDevice * tmpDevice = ctrlTmp->getCamera(nCamera);
		if (tmpDevice)
		{
			tmpDevice->setExposureTime(exposureTime);
		}
	}

	/*****/

}

void CameraModule::addSettingWiddget(QTabWidget *tabWidget)
{
	if (tabWidget)
	{
		tabWidget->addTab(m_pSetting/*new CameraSetting(m_pCameraCtrl)*/, QStringLiteral("相机设定"));
	}

	//QString user;
	//int level = 0;
	//System->getUser(user, level);
	//if (USER_LEVEL_MANAGER > level)
	//{
	//	//tabWidget->setEnabled(false);
	//}		
}


void CameraModule::showSettingWidget()
{


}

QMOUDLE_INSTANCE(CameraModule)