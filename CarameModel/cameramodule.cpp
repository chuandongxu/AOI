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

bool CameraModule::captureAllImages(QVector<cv::Mat>& imageMats)
{
	IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return false;

	imageMats.clear();

	if (!startCapturing())
	{
		System->setTrackInfo(QString("startCapturing error."));	
		return false;
	}
	
	if (!pMotion->triggerCapturing(IMotion::TRIGGER_ALL, true))
	{
		System->setTrackInfo(QString("triggerCapturing error."));	
		return false;
	}	

	int nWaitTime = 100;
	while (! isCaptureImageBufferDone() && nWaitTime-- > 0)
		QThread::msleep(10);

	if (nWaitTime <= 0)
	{
		System->setTrackInfo(QString("CaptureImageBufferDone error."));
		return false;
	}

	int nCaptureNum = getImageBufferCaptureNum();
	for (int i = 0; i < nCaptureNum; ++ i)
	{
		cv::Mat matImage = getImageItemBuffer(i);
		imageMats.push_back(matImage);
	}

	if (nCaptureNum != getImageBufferNum())
	{	
		System->setTrackInfo(QString("System captureAllImages error, Image Num: %1").arg(nCaptureNum));

        QString capturePath = System->getParam("camera_cap_image_path").toString();
        QDateTime dtm = QDateTime::currentDateTime();
        QString fileDir = capturePath + "/" + dtm.toString("MMddhhmmss") + "/";
        QDir dir; dir.mkdir(fileDir);

        for (size_t i = 0; i < imageMats.size(); ++ i) {
            QString name = QString("%1").arg(i + 1, 2, 10, QChar('0')) + QStringLiteral(".bmp");
            cv::imwrite((fileDir + name).toStdString().c_str(), imageMats[i]);
        }
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