#include <QList>
#include <QThread>
#include <QDir>
#include <QDateTime>

#include "cameramodule.h"
#include "CameraCtrl.h"
#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IMotion.h"
#include "../include/ILight.h"

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

bool CameraModule::startUpCapture(bool bHWTrigger)
{
	if (m_pMainProcess)
	{
		return m_pMainProcess->startUpCapture(bHWTrigger);
	}

	return false;
}

bool CameraModule::isHWTrigger()
{
	if (m_pMainProcess)
	{
		return m_pMainProcess->isHWTrigger();
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

bool CameraModule::selectCaptureMode(TRIGGER emCaptureMode, bool reStartUp)
{
	if (m_pMainProcess)
	{
		return m_pMainProcess->selectCaptureMode(emCaptureMode, reStartUp);
	}
	return false;
}

bool CameraModule::startCapturing()
{
	return m_pMainProcess->startCapturing();
}

bool CameraModule::getImages(QVector<cv::Mat>& imageMats)
{
	return m_pMainProcess->getImages(imageMats);
}

bool CameraModule::getLastImages(QVector<cv::Mat>& imageMats)
{
	return m_pMainProcess->getLastImages(imageMats);
}

bool CameraModule::stopCapturing()
{
	return m_pMainProcess->stopCapturing();
}

bool CameraModule::captureAllImages(QVector<cv::Mat>& imageMats)
{
	IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return false;

    ILight* pLight = getModule<ILight>(LIGHT_MODEL);
    if (!pLight) return false;

	imageMats.clear();

	//if (!startCapturing())
	//{
	//	System->setTrackInfo(QString("startCapturing error."));
	//	return false;
	//}
	
    bool bTriggerBoard = System->isTriggerBoard();
    if (bTriggerBoard)
    {
        if (!pLight->triggerCapturing(ILight::TRIGGER_ALL, true, true))
        {
            System->setTrackInfo(QString("triggerCapturing error!"));
        }
    }
    else
    {
        if (!pMotion->triggerCapturing(IMotion::TRIGGER_ALL, true))
        {
            System->setTrackInfo(QString("triggerCapturing error."));
            return false;
        }
    }

	if (!getLastImages(imageMats))
	{
		System->setTrackInfo(QString("getLastImages error."));
		return false;
	}

	return true;
}

bool CameraModule::getCameraScreenSize(int& nWidth, int& nHeight)
{
    if (System->isRunOffline()) {
        nWidth  = 2040;
        nHeight = 2048;
        return true;
    }

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