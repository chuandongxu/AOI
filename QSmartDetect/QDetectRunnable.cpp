#include "QDetectRunnable.h"
#include "qsmartdetect_global.h"
#include <time.h>
#include <qthread.h>
#include <qthreadpool.h>
#include <QFileDialog>

#include "../Common/ModuleMgr.h"
#include "../Common/SystemData.h"
#include "../include/IdDefine.h"
#include "../include/IData.h"
#include "../include/IDlp.h"
#include "../include/ICamera.h"
#include "../include/IVision.h"
#include "../include/IMotion.h"
#include "../Common/eos.h"

#include "CryptLib.h"

#include "../DataModule/QDetectObj.h"
#include "../DataModule/QProfileObj.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define POS_ZHOME   "zHome%0"
#define PROFILE_X   "xMoveProfile%0"
const QString TimeFormatString = "MMdd";

QCheckerRunable::QCheckerRunable(QCheckerParamMap * paramMap, Q3DStructData* dataParam)
	:m_paramMap(paramMap), m_dataParam(dataParam), m_exit(false)
{
	//SetThreadAffinityMask(GetCurrentThread(), 0x01);	
	m_bRunning = false;
}

QCheckerRunable::~QCheckerRunable()
{

}

void QCheckerRunable::quit()
{
	m_exit = true;
}

void QCheckerRunable::imgStop()
{
}

bool QCheckerRunable::isRunning()
{
	return m_bRunning;
}

bool QCheckerRunable::startUpSetup()
{
	int iStation = getStationID();

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (pVision)
	{
		pVision->loadCmdData(iStation);
	}

	int nCaptureMode = System->getParam("camera_capture_mode").toInt();
	bool isImageFloder = (2 == nCaptureMode);
	if (isImageFloder) return true;

	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return false;
	
	return pDlp->isConnected(iStation - 1);
}

bool QCheckerRunable::endUpSetup()
{
	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return false;

	int iStation = getStationID();

	return true;
}

void QCheckerRunable::run()
{
	int iStation = getStationID();

	if (!startUpSetup())
	{
		System->setTrackInfo(QString(QStringLiteral("工位%0启动失败！")).arg(iStation));
		return;
	}
	System->setTrackInfo(QString(QStringLiteral("工位%0启动成功")).arg(iStation));
	m_bRunning = true;

	int nDlpMode = System->getParam("sys_run_dlp_mode").toInt();
	bool bMotionCardTrigger = (1 == nDlpMode);

	double dtime_start = 0, dtime_movePos = 0;
	while (!isExit())
	{
		QThread::msleep(1);

		if (!waitStartBtn())continue;
		if (isExit())break;

		if (!bMotionCardTrigger)
		{
			dtime_start = double(clock());
			if (!captureImages())continue;
			if (isExit())break;
			dtime_movePos = double(clock());
			System->setTrackInfo(QStringLiteral("Station%1:采集图像数据: %2 ms").arg(iStation).arg(dtime_movePos - dtime_start), true);
		}

		dtime_start = double(clock());
		if (!generateGrayImage(bMotionCardTrigger))continue;
		if (isExit())break;
		dtime_movePos = double(clock());
		System->setTrackInfo(QStringLiteral("Station%1:计算3D灰度图: %2 ms").arg(iStation).arg(dtime_movePos - dtime_start), true);

		dtime_start = double(clock());
		if (!calculate3DHeight(bMotionCardTrigger))continue;
		if (isExit())break;
		dtime_movePos = double(clock());
		System->setTrackInfo(QStringLiteral("Station%1:计算3D高度数据: %2 ms").arg(iStation).arg(dtime_movePos - dtime_start), true);

		if (!waitCheckDone())continue;
		if (isExit())break;
	}

	endUpSetup();

	m_bRunning = false;
	System->setTrackInfo(QString(QStringLiteral("工位%0已停止")).arg(getStationID()));
}

bool QCheckerRunable::waitStartBtn()
{
	while (1)
	{
		if (m_dataParam->_bStartCapturing)
		{
			return true;
		}

		if (isExit())return false;
		QThread::msleep(10);
	}

	return true;
}

bool QCheckerRunable::captureImages()
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return false;

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	m_imageMats.clear();

	int nWaitTime = 5 * 100;
	while (!pCam->isCameraCaptureAvaiable() && nWaitTime-- > 0 && !isExit())
	{
		QThread::msleep(10);
	}
	if (nWaitTime <= 0) return false;

	int iStation = getStationID();

	if (!pCam->lockCameraCapture(iStation))
	{
		return false;
	}

	//QEos::Notify(EVENT_CHECK_STATE, iStation, STATION_STATE_CAPTURING, 0);

	int nCaptureMode = System->getParam("camera_capture_mode").toInt();
	bool isImageFloder = (2 == nCaptureMode);
	if (isImageFloder)
	{
		System->setParam("dlp_image_capture_index", iStation - 1);
	}

	if (!pCam->startCapturing())
	{
		System->setTrackInfo(QString("startCapturing error,station %1").arg(iStation));
		pCam->unlockCameraCapture();
		return false;
	}

	System->setTrackInfo(QStringLiteral("Station%1:开始采集数据...").arg(iStation));

	if (!isImageFloder)
	{
		if (!pDlp->isConnected(iStation - 1) || !pDlp->trigger(iStation - 1))
		{
			System->setTrackInfo(QString("DLP not connection error,station %1").arg(iStation));
			pCam->unlockCameraCapture();
			return false;
		}
	}
	
	nWaitTime = 10 * 100;
	while (!pCam->isCaptureImageBufferDone() && nWaitTime-- > 0 && !isExit())
	{
		QThread::msleep(10);
	}

	if (nWaitTime <= 0)
	{
		System->setTrackInfo(QString("CaptureImageBufferDone error,station %1").arg(iStation));
		pCam->unlockCameraCapture();
		return false;
	}

	bool bCaptureImageAsMatlab = System->getParam("camera_cap_image_matlab").toBool();
	
	int nCaptureNum = pCam->getImageBufferCaptureNum();
	for (int i = 0; i < nCaptureNum; i++)
	{
		int nIndex = i;

		if (bCaptureImageAsMatlab && !isImageFloder)
		{
			if (5 == nIndex)
			{
				nIndex += 1;
			}
			else if (6 == nIndex)
			{
				nIndex -= 1;
			}
		}

		cv::Mat matImage = pCam->getImageItemBuffer(nIndex);

		m_imageMats.push_back(matImage);
	}

	if (nCaptureNum < pCam->getImageBufferNum())
	{
		pCam->unlockCameraCapture();
		System->setTrackInfo(QString("System captureImages error, Image Num: %1").arg(nCaptureNum));

		bool bCaptureImage = System->getParam("camera_cap_image_enable").toBool();
		if (!bCaptureImage)
		{
			QString capturePath = System->getParam("camera_cap_image_path").toString();

			QDateTime dtm = QDateTime::currentDateTime();
			QString fileDir = capturePath + "/" + dtm.toString("MMddhhmmss") + "/";
			QDir dir; dir.mkdir(fileDir);

			for (int i = 0; i < m_imageMats.size(); i++)
			{
				IplImage frameImg = IplImage(m_imageMats[i]);
				QString name = QString("%1").arg(i + 1, 2, 10, QChar('0')) + QStringLiteral(".bmp");
				cvSaveImage((fileDir + name).toStdString().c_str(), &frameImg);

				QString nameEncrypt = fileDir + QString("%1").arg(i + 1, 2, 10, QChar('0')) + ".ent";
				AOI::Crypt::EncryptFileNfg((fileDir + name).toStdString(), nameEncrypt.toStdString());
				QFile::remove(fileDir + name);
			}
		}

		return false;
	}
	//System->setTrackInfo(QString("System captureImages Image Num: %1").arg(nCaptureNum));

	pCam->unlockCameraCapture();

	return true;
}

bool QCheckerRunable::calculate3DHeight(bool bMotionCardTrigger)
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	int iStation = getStationID();
	//QEos::Notify(EVENT_CHECK_STATE, iStation, STATION_STATE_CALCULATE_3D, 0);

	if (pVision)
	{
		if (pVision->calculate3DHeight(iStation, bMotionCardTrigger ? m_dataParam->_srcImageMats : m_imageMats, m_3DMatHeight, m_matHeightResultImg));
		{
			//pCam->setImage(m_matHeightResultImg);
			//pCam->setHeightData(m_3DMatHeight);
			m_dataParam->_3DMatHeight = m_3DMatHeight;
			return true;
		}
	}

	return false;
}

bool QCheckerRunable::generateGrayImage(bool bMotionCardTrigger)
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	int iStation = getStationID();
	//QEos::Notify(EVENT_CHECK_STATE, iStation, STATION_STATE_GENERATE_GRAY, 0);

	if (pVision)
	{
		if (pVision->generateGrayImage(bMotionCardTrigger ? m_dataParam->_srcImageMats : m_imageMats, m_matImage))
		{
			m_dataParam->_matImage = m_matImage;
			//pCam->setImage(m_matImage, false);
			//QEos::Notify(EVENT_RESULT_DISPLAY, 0, STATION_RESULT_IMAGE_DISPLAY);
			return true;
		}
	}

	return false;
}

bool QCheckerRunable::waitCheckDone()
{
	//int iStation = getStationID();

	//IData * pData = getModule<IData>(DATA_MODEL);
	//if (pData)
	//{
	//}

	m_dataParam->_bStartCapturing = false;
	m_dataParam->_bCapturedDone = true;

	return true;
}

bool QCheckerRunable::isExit()
{
	return m_exit;
}

void QCheckerRunable::addImageText(cv::Mat image, Point ptPos, QString szText)
{
	double dScaleFactor = 2.0;


	cv::String text = szText.toStdString();

	Point ptPos1;
	ptPos1.x = ptPos.x - 10 * dScaleFactor;
	ptPos1.y = ptPos.y + 10 * dScaleFactor;
	Point ptPos2;
	ptPos2.x = ptPos.x + 10 * dScaleFactor * text.length();
	ptPos2.y = ptPos.y - 20 * dScaleFactor;
	rectangle(image, ptPos1, ptPos2, Scalar(125, 125, 125), -1);
	rectangle(image, ptPos1, ptPos2, Scalar(200, 200, 200), 1);

	double fontScale = dScaleFactor*0.5f;
	cv::putText(image, text, ptPos, CV_FONT_HERSHEY_COMPLEX, fontScale, Scalar(0, 255, 255), 1);
}

int QCheckerRunable::getStationID()
{
	return m_paramMap->value(PARAM_STATION_ID).toInt();
}

//*****************************************************************************
//
//
//******************************************************************************

QMainRunable::QMainRunable(QCheckerParamMap * paramMap, QCheckerParamDataList* paramData, DataCtrl* pCtrl)
	:m_paramMap(paramMap), m_paramData(paramData), m_pCtrl(pCtrl), m_exit(false)
{
}

QMainRunable::~QMainRunable()
{

}

void QMainRunable::quit()
{
	resetResoultLight();
	m_exit = true;
}

void QMainRunable::imgStop()
{
}

void QMainRunable::run()
{
	System->setTrackInfo(QString(QStringLiteral("主流程启动成功!")));

	while (!isExit())
	{
		QThread::msleep(1);

		if (!waitStartBtn())continue;
		if (isExit())break;

		double dtime_start = double(clock());
		if (!captureImages())continue;
		if (isExit())break;
		double dtime_movePos = double(clock());
		System->setTrackInfo(QStringLiteral("Board%1:采集图像并生成3D数据: %2 ms").arg(0).arg(dtime_movePos - dtime_start), true);	

		dtime_start = double(clock());
		if (!matchPosition())continue;
		if (isExit())break;
		dtime_movePos = double(clock());
		System->setTrackInfo(QStringLiteral("Board%1:模板匹配元件: %2 ms").arg(0).arg(dtime_movePos - dtime_start), true);		

		dtime_start = double(clock());
		if (!calculateDetectHeight())continue;
		if (isExit())break;
		dtime_movePos = double(clock());
		System->setTrackInfo(QStringLiteral("Board%1:计算3D测量数据: %2 ms").arg(0).arg(dtime_movePos - dtime_start), true);

		dtime_start = double(clock());
		if (!calculateDetectProfile())continue;
		if (isExit())break;
		dtime_movePos = double(clock());
		System->setTrackInfo(QStringLiteral("Board%1:计算Profile数据: %2 ms").arg(0).arg(dtime_movePos - dtime_start), true);

		if (!waitCheckDone())continue;
		if (isExit())break;
	}

	if (isExit())
	{
		IData * pData = getModule<IData>(DATA_MODEL);
		if (pData)
		{
			pData->decrementCycleTests();
		}
	}

	System->setTrackInfo(QString(QStringLiteral("主流程已停止")));
}

bool QMainRunable::waitStartBtn()
{
	if (m_pCtrl)
	{
		int iState = 0;

		QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_WAIT_START, 0);

		while (1)
		{
			if (m_pCtrl->getCycleTests() > 0)
			{
				return true;
			}

			if (isExit())return false;
			QThread::msleep(10);
		}

		return true;
	}

	return false;
}

bool QMainRunable::captureAllImages(QVector<cv::Mat>& imageMats)
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return false;

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return false;

	imageMats.clear();

	int nWaitTime = 5 * 100;
	while (!pCam->isCameraCaptureAvaiable() && nWaitTime-- > 0 && !isExit())
	{
		QThread::msleep(10);
	}
	if (nWaitTime <= 0) return false;

	int iStation = 0;

	if (!pCam->lockCameraCapture(iStation))
	{
		return false;
	}

	if (!pCam->startCapturing())
	{
		System->setTrackInfo(QString("startCapturing error,station %1").arg(iStation));
		pCam->unlockCameraCapture();
		return false;
	}

	//System->setTrackInfo(QString("Trigger Station:%1").arg(iStation));

	if (!pMotion->triggerCapturing(IMotion::TRIGGER_ALL, true))
	{
		System->setTrackInfo(QString("triggerCapturing error,station %1").arg(iStation));
		pCam->unlockCameraCapture();
		return false;
	}

	//double dtime_start = double(clock());
	nWaitTime = 10 * 100;
	while (!pCam->isCaptureImageBufferDone() && nWaitTime-- > 0 && !isExit())
	{
		QThread::msleep(10);
	}

	if (nWaitTime <= 0)
	{
		System->setTrackInfo(QString("CaptureImageBufferDone error,station %1").arg(iStation));
		pCam->unlockCameraCapture();
		return false;
	}
	//double dtime_movePos = double(clock());
	//System->setTrackInfo(QStringLiteral("Waiting Capturing Done: %1 ms").arg(dtime_movePos - dtime_start), true);

	bool bCaptureImageAsMatlab = System->getParam("camera_cap_image_matlab").toBool();
	int nCaptureNum = pCam->getImageBufferCaptureNum();
	int nPatternSeqNum = pCam->getImageBufferNum();
	for (int i = 0; i < nCaptureNum; i++)
	{
		int nIndex = i;

		if (bCaptureImageAsMatlab)
		{
			if (5 == nIndex%nPatternSeqNum)
			{
				nIndex += 1;
			}
			else if (6 == nIndex%nPatternSeqNum)
			{
				nIndex -= 1;
			}
		}

		cv::Mat matImage = pCam->getImageItemBuffer(nIndex);

		imageMats.push_back(matImage);
		//imageMats.push_back(matImage.clone());
	}

	if (nCaptureNum != pCam->getImageBufferNum())
	{
		pCam->unlockCameraCapture();
		System->setTrackInfo(QString("System captureImages error, Image Num: %1").arg(nCaptureNum));

		bool bCaptureImage = System->getParam("camera_cap_image_enable").toBool();
		if (!bCaptureImage)
		{
			QString capturePath = System->getParam("camera_cap_image_path").toString();

			QDateTime dtm = QDateTime::currentDateTime();
			QString fileDir = capturePath + "/" + dtm.toString("MMddhhmmss") + "/";
			QDir dir; dir.mkdir(fileDir);

			for (int i = 0; i < imageMats.size(); i++)
			{
				IplImage frameImg = IplImage(imageMats[i]);
				QString name = QString("%1").arg(i + 1, 2, 10, QChar('0')) + QStringLiteral(".bmp");
				cvSaveImage((fileDir + name).toStdString().c_str(), &frameImg);

				QString nameEncrypt = fileDir + QString("%1").arg(i + 1, 2, 10, QChar('0')) + ".ent";
				AOI::Crypt::EncryptFileNfg((fileDir + name).toStdString(), nameEncrypt.toStdString());
				QFile::remove(fileDir + name);
			}
		}

		return false;
	}
	//System->setTrackInfo(QString("System captureImages Image Num: %1").arg(nCaptureNum));

	pCam->unlockCameraCapture();

	return true;
}

bool QMainRunable::captureImages()
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return false;

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_CAPTURING, 0);

	int nDlpMode = System->getParam("sys_run_dlp_mode").toInt();
	bool bMotionCardTrigger = (1 == nDlpMode);

	int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;

	int nCaptureMode = System->getParam("camera_capture_mode").toInt();
	bool isImageFloder = (2 == nCaptureMode);
	if (!isImageFloder)
	{
		for (int i = 0; i < nStationNum; i++)
		{
			if (!pDlp->isConnected(i))
			{
				System->setTrackInfo(QString("System DLP not connect error!"));
				return false;
			}
		}
	}

	if (bMotionCardTrigger)
	{
		double dtime_start = double(clock());
		QVector<cv::Mat> imageMats;
		if (!captureAllImages(imageMats)) return false;
		double dtime_movePos = double(clock());
		System->setTrackInfo(QStringLiteral("Board%1:采集图像数据: %2 ms").arg(0).arg(dtime_movePos - dtime_start), true);

		int nPatternNum = System->getParam("motion_trigger_pattern_num").toInt();
		for (int i = 0; i < nStationNum; i++)
		{
			(*m_paramData)[i]._srcImageMats.clear();
			for (int j = 0; j < nPatternNum; j++)
			{
				(*m_paramData)[i]._srcImageMats.push_back(imageMats[i*nPatternNum + j]);
			}
		}
	}

	for (int i = 0; i < nStationNum; i++)
	{
		(*m_paramData)[i]._bCapturedDone = false;
		(*m_paramData)[i]._bStartCapturing = true;
	}

	int nWaitTime = 30 * 100;
	while (nWaitTime-- > 0 && !isExit())
	{
		bool bCapturedDone = true;
		for (int i = 0; i < nStationNum; i++)
		{
			if (!(*m_paramData)[i]._bCapturedDone)
			{
				bCapturedDone = false;
				break;
			}
		}

		if (bCapturedDone) break;
		QThread::msleep(10);
	}
	if (nWaitTime <= 0)
	{
		System->setTrackInfo(QStringLiteral("等待DLP计算TimeOut. Try again!"));
		return false;
	}

	if (isExit()) return false;
	QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_GENERATE_GRAY, 0);

	cv::Mat matGray; int nCapturedNum = 0;
	for (int i = 0; i < nStationNum; i++)
	{
		if (matGray.empty())
		{
			matGray = (*m_paramData)[i]._matImage.clone();
			matGray.setTo(0);
		}
		matGray += (*m_paramData)[i]._matImage;
		nCapturedNum += 1;
	}
	matGray /= nCapturedNum;

	m_matImage.release();
	m_matImage = matGray;

	pCam->setImage(m_matImage, false);
	pVision->setProfGrayImg(m_matImage);
	if (isExit()) return false;
	QEos::Notify(EVENT_RESULT_DISPLAY, 0, STATION_RESULT_IMAGE_DISPLAY);

	pVision->loadImage(m_matImage);

	if (isExit()) return false;
	QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_CALCULATE_3D, 0);

	QVector<cv::Mat> matHeights;
	for (int i = 0; i < nStationNum; i++)
	{
		matHeights.push_back((*m_paramData)[i]._3DMatHeight);
	}

	m_3DMatHeight.release();
	if (!pVision->merge3DHeight(matHeights, m_3DMatHeight))
	{
		return false;
	}

	pCam->setHeightData(m_3DMatHeight);
	//pVision->setHeightData(m_3DMatHeight);

	return true;
}

bool QMainRunable::matchPosition()
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_MATCH_POSITION, 0);

	clearTestObjs();
	clearTestProfObjs();

	cv::Mat matDisplay = m_matImage.clone();

	if (!matDisplay.empty())
	{
		if (!pVision->matchPosition(matDisplay, m_objTests)) return false;	
		if (!pVision->matchAlignment(matDisplay, m_objProfTests)) return false;

		matDisplay.release();
	}

	//pCam->setImage(matDisplay);

	return true;
}

bool QMainRunable::calculateDetectHeight()
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_CALCULATE_HEIGHT, 0);

	if (pVision->calculateDetectHeight(m_3DMatHeight, m_objTests))
	{
		return true;
	}

	return false;
}

bool QMainRunable::calculateDetectProfile()
{
	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	if (pVision->calculateDetectProfile(m_3DMatHeight, m_objProfTests))
	{
		return true;
	}

	return false;
}

bool QMainRunable::waitCheckDone()
{

	QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_RESOULT, 1);

	//IData * pData = getModule<IData>(DATA_MODEL);
	if (m_pCtrl)
	{
		displayAllObjs();
		displayAllProfObjs();

		int n = System->getSysParam("OK_COUNT_SYSTEM").toInt();
		System->setTrackInfo(QStringLiteral("统计:已完成测量数据: %1 个").arg(n + 1), true);

		m_pCtrl->decrementCycleTests();
	}

	return true;
}

bool QMainRunable::isExit()
{
	return m_exit;
}

void QMainRunable::clearTestObjs()
{
	for (int i = 0; i < m_objTests.size(); i++)
	{
		delete m_objTests[i];
	}
	m_objTests.clear();
}

void QMainRunable::clearTestProfObjs()
{
	for (int i = 0; i < m_objProfTests.size(); i++)
	{
		delete m_objProfTests[i];
	}
	m_objProfTests.clear();
}

void QMainRunable::displayAllObjs()
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return;

	cv::Mat matDisplay = m_matImage.clone();

	if (matDisplay.type() == CV_8UC1)
	{
		//input image is grayscale
		cvtColor(matDisplay, matDisplay, CV_GRAY2RGB);

	}

	if (!matDisplay.empty())
	{
		QEos::Notify(EVENT_RESULT_DISPLAY, 0, STATION_RESULT_DISPLAY_CLEAR_DATA);
		for (int i = 0; i < m_objTests.size(); i++)
		{
			QDetectObj* pObj = m_objTests[i];
			if (pObj)
			{
				cv::RotatedRect rtFrame = pObj->getFrame();
				cv::rectangle(matDisplay, rtFrame.boundingRect(), Scalar(0, 255, 0), 2);

				if (pObj->isTested())
				{
					for (int j = 0; j < pObj->getHeightDetectNum(); j++)
					{
						cv::RotatedRect rtDetect = pObj->getHeightDetect(j);
						cv::rectangle(matDisplay, rtDetect.boundingRect(), Scalar(255, 0, 0), 2);

						Point ptPos = Point(rtDetect.center.x, rtDetect.center.y);
						addImageText(matDisplay, ptPos, QString::number(pObj->getHeightValue(j), 'f', 3) /*+ QStringLiteral("°")*/);

						if (0 == j)
						{
							QEos::Notify(EVENT_RESULT_DISPLAY, 0, STATION_RESULT_DISPLAY, pObj->getName(), i, pObj->getHeightValue(j));
						}						
					}
				}
			}
		}
	}

	pCam->setImage(matDisplay, false);
	QEos::Notify(EVENT_RESULT_DISPLAY, 0, STATION_RESULT_IMAGE_DISPLAY);
}

void QMainRunable::displayAllProfObjs()
{
	for (int i = 0; i < m_objProfTests.size(); i++)
	{
		QProfileObj* pObj = m_objProfTests.at(i);
		if (pObj)
		{
			QVector<cv::Point2d> profDatas = pObj->getProfData();
			QVariantList varList;
			for (cv::Point2d pt : profDatas)
			{
				varList << QPointF(pt.x, pt.y);
			}

			QVariantList varRangeList;
			QVariantList varRangeTypeList;
			QVariantList varRangeValueList;

			if (pObj->getMeasObjNum() <= 0)
			{
				QEos::Notify(EVENT_RESULT_DISPLAY, 0, STATION_RESULT_DISPLAY_PROF, pObj->getName(), i, QStringLiteral(""), varList, varRangeList, varRangeTypeList, varRangeValueList);
				return;
			}
			else
			{
				for (int j = 0; j < pObj->getMeasObjNum(); j++)
				{
					QProfMeasureObj*  pMeasObj = pObj->getMeasObj(j);
					if (pMeasObj)
					{
						for (int m = 0; m < pMeasObj->getRangeNum(); m++)
						{
							QPoint ptRange;
							ptRange.setX(pMeasObj->getRange(m).x);
							ptRange.setY(pMeasObj->getRange(m).y);
							varRangeList << ptRange;
							varRangeTypeList << pMeasObj->getRangeType(m);

							QPoint ptRangeValue;
							ptRangeValue.setX(pMeasObj->getRangeValue(m).x);
							ptRangeValue.setY(pMeasObj->getRangeValue(m).y);
							varRangeValueList << ptRangeValue;
						}
						QEos::Notify(EVENT_RESULT_DISPLAY, 0, STATION_RESULT_DISPLAY_PROF, pObj->getName(), i, pMeasObj->getValueStr(), varList, varRangeList, varRangeTypeList, varRangeValueList);
						return;
					}
				}
			}
		}	
	}
}

void QMainRunable::addImageText(cv::Mat image, Point ptPos, QString szText)
{
	double dScaleFactor = 2.0;

	cv::String text = szText.toStdString();

	Point ptPos1;
	ptPos1.x = ptPos.x - 10 * dScaleFactor;
	ptPos1.y = ptPos.y + 10 * dScaleFactor;
	Point ptPos2;
	ptPos2.x = ptPos.x + 10 * dScaleFactor * text.length();
	ptPos2.y = ptPos.y - 20 * dScaleFactor;
	rectangle(image, ptPos1, ptPos2, Scalar(125, 125, 125), -1);
	rectangle(image, ptPos1, ptPos2, Scalar(200, 200, 200), 1);

	double fontScale = dScaleFactor*0.5f;
	cv::putText(image, text, ptPos, CV_FONT_HERSHEY_COMPLEX, fontScale, Scalar(0, 255, 255), 1);
}

void QMainRunable::setResoultLight(bool isOk)
{
	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if (p)
	{
		int okLight = 0;
		int ngLight = 0;
		if (getLightIO(okLight, ngLight))
		{
			if (isOk)
			{
				//p->setExtDO(okLight, 1);
				//p->setExtDO(ngLight, 0);
			}
			else
			{
				//p->setExtDO(okLight, 0);
				//p->setExtDO(ngLight, 1);
			}
		}
	}
}

void QMainRunable::resetResoultLight()
{
	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if (p)
	{
		int okLight = 0;
		int ngLight = 0;
		if (getLightIO(okLight, ngLight))
		{
			//p->setExtDO(okLight, 0);
			//p->setExtDO(ngLight, 0);
		}
	}
}

bool QMainRunable::getLightIO(int &okLight, int &ngLight)
{
	if (m_paramMap->contains(PARAM_STATION_OK_LIGHT)
		&& m_paramMap->contains(PARAM_STATION_NG_LIGHT))
	{
		okLight = m_paramMap->value(PARAM_STATION_OK_LIGHT).toInt();
		ngLight = m_paramMap->value(PARAM_STATION_NG_LIGHT).toInt();

		return true;
	}

	return false;
}

QDetectRunnable::QDetectRunnable(QObject *parent)
	: QObject(parent)
{	
}

QDetectRunnable::~QDetectRunnable()
{
}
