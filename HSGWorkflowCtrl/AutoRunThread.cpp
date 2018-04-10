#include <QDir>
#include <QThread>
#include <QThreadPool>
#include <QDateTime>
#include <QMessageBox>
#include <time.h>

#include "opencv2/highgui.hpp"
#include "opencv2/video.hpp"

#include "AutoRunThread.h"
#include "../Common/eos.h"
#include "../Common/ModuleMgr.h"
#include "../Common/SystemData.h"
#include "../include/IMotion.h"
#include "../include/IData.h"
#include "../include/IDlp.h"
#include "../include/ICamera.h"
#include "../include/IVision.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "hsgworkflowctrl_global.h"
#include "../include/constants.h"
#include "TimeLog.h"
#include "../DataModule/DataUtils.h"
#include "AlignmentRunnable.h"
#include "Calc3DHeightRunnable.h"
#include "Insp3DHeightRunnable.h"
#include "Insp2DRunnable.h"

AutoRunThread::AutoRunThread(QCheckerParamMap                    *paramMap,
                           QCheckerParamDataList                 *paramData,
                           const Engine::AlignmentVector         &vecAlignments,
                           const Engine::WindowVector            &vecWindows,
                           const Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr)
	:m_paramMap       (paramMap),
     m_paramData      (paramData),
     m_vecAlignments  (vecAlignments),
     m_vecWindows     (vecWindows),
     m_vecVecFrameCtr (vecVecFrameCtr),
     m_exit           (false)
{
	m_nImageIndex = 0;
}

AutoRunThread::~AutoRunThread()
{
}

int AutoRunThread::getPositionNum()
{
	return m_positionMap.size();
}

int AutoRunThread::getPositionID(int nIndex)
{
	return m_positionMap.value(nIndex);
}

void AutoRunThread::quit()
{
	resetResoultLight();
	m_exit = true;
}

bool AutoRunThread::preRunning()
{
	m_3DMatHeights.clear();
	m_matImages.clear();	

	for (int i = 0; i < getPositionNum(); i++)
	{
		m_3DMatHeights.push_back(cv::Mat());
		m_matImages.push_back(QImageStruct());
	}

    m_dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    m_dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    m_nDLPCount = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;

	return true;
}

void AutoRunThread::run()
{
	if (! preRunning()) return;

	System->setTrackInfo(QString(QStringLiteral("主流程启动成功!")));	

	double dtime_start = 0, dtime_movePos = 0;
	while (! isExit())
	{
		if (!waitStartBtn()) continue;
		if (isExit()) break;
		
		QString szImagePath = generateImagePath();

		dtime_start = double(clock());
		if (!mergeImages(szImagePath)) continue;
		if (isExit()) break;
		dtime_movePos = double(clock());
		System->setTrackInfo(QStringLiteral("Board%1:合成数据: %2 ms").arg(0).arg(dtime_movePos - dtime_start), true);

		if (! _doAlignment()) continue;
		if (isExit()) break;
		TimeLogInstance->addTimeLog("Finished do alignment.");

        if (m_vecVecFrameCtr.empty())
            break;

        if(! _doInspection()) continue;
        if (isExit()) break;
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

bool AutoRunThread::waitStartBtn()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (pData)
	{
		int iState = 0;

		QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_WAIT_START, 0);

		while (1)
		{
			if (pData->getCycleTests() > 0)
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

bool AutoRunThread::moveToCapturePos(int nIndex)
{
	IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return false;

	if (!pMotion->moveToPosGroup(getPositionID(nIndex), true))
	{
		System->setTrackInfo(QString("move to position error"));
		return false;
	}
	return true;
}

bool AutoRunThread::moveToCapturePos(float fPosX, float fPosY)
{
    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return false;

    if (! pMotion->moveToGroup(std::vector<int>({AXIS_MOTOR_X, AXIS_MOTOR_Y}), std::vector<double>({fPosX, fPosY}), std::vector<int>({0, 0}), true))
    {
        System->setTrackInfo(QString("move to position error"));
		return false;
    }
    return true;
}

bool AutoRunThread::captureAllImages(QVector<cv::Mat>& imageMats)
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

	if (!pCam->startCapturing())
	{
		System->setTrackInfo(QString("startCapturing error"));	
		return false;
	}
	
	if (!pMotion->triggerCapturing(IMotion::TRIGGER_ALL, true))
	{
		System->setTrackInfo(QString("triggerCapturing error"));	
		return false;
	}	

	int nWaitTime = 10 * 100;
	while (! pCam->isCaptureImageBufferDone() && nWaitTime-- > 0 && ! isExit())
	{
		QThread::msleep(10);
	}

	if (nWaitTime <= 0)
	{
		System->setTrackInfo(QString("CaptureImageBufferDone error"));		
		return false;
	}

	int nCaptureNum = pCam->getImageBufferCaptureNum();
	for (int i = 0; i < nCaptureNum; i++)
	{
		cv::Mat matImage = pCam->getImageItemBuffer(i);
		imageMats.push_back(matImage);
	}

	if (nCaptureNum != pCam->getImageBufferNum())
	{	
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
				QString name = QString("%1").arg(i + 1, 2, 10, QChar('0')) + QStringLiteral(".bmp");
				cv::imwrite((fileDir + name).toStdString().c_str(), imageMats[i]);
			}
		}

		return false;
	}
	System->setTrackInfo(QString("System captureImages Image Num: %1").arg(nCaptureNum));

	return true;
}

bool AutoRunThread::captureImages(int nIndex, QString& szImagePath)
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return false;

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
	if (!pUI) return false;

	QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_CAPTURING, 0);

	int nDlpMode = System->getParam("sys_run_dlp_mode").toInt();
	bool bMotionCardTrigger = (1 == nDlpMode);

	int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
	for (int i = 0; i < nStationNum; i++)
	{
		if (!pDlp->isConnected(i))
		{
			System->setTrackInfo(QString("System DLP not connect error!"));
			return false;
		}
	}

	QVector<cv::Mat> imageMats;
	int nPatternNum = System->getParam("motion_trigger_pattern_num").toInt();
	if (bMotionCardTrigger)
	{
		double dtime_start = double(clock());		
		if (!captureAllImages(imageMats)) return false;
		double dtime_movePos = double(clock());
		System->setTrackInfo(QStringLiteral("Board%1:采集图像数据: %2 ms").arg(0).arg(dtime_movePos - dtime_start), true);
		
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

	//bool bCaptureLightImage = System->getParam("camera_cap_image_light").toBool();
	//if (bCaptureLightImage)
	//{
	//	saveImages(szImagePath, nIndex, imageMats.mid(nStationNum*nPatternNum));
	//}

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
	cv::cvtColor(matGray, matGray, CV_BayerGR2BGR);
	m_matImages[nIndex]._img[4] = matGray;

	//pUI->setImage(matGray, true);
	//QEos::Notify(EVENT_RESULT_DISPLAY, 0, STATION_RESULT_IMAGE_DISPLAY);

	QVector<cv::Mat> lightImages = imageMats.mid(nStationNum*nPatternNum);
	cv::Mat outputPseudocolor(matGray.size(), CV_8UC3);
	unsigned char grayValueB, grayValueG, grayValueR;
	for (int y = 0; y < matGray.rows; y++)
	{
		for (int x = 0; x < matGray.cols; x++)
		{
			grayValueB = lightImages[4].at<uchar>(y, x);
			grayValueG = lightImages[3].at<uchar>(y, x);
			grayValueR = lightImages[2].at<uchar>(y, x);

			cv::Vec3b& pixel = outputPseudocolor.at<cv::Vec3b>(y, x);
			pixel[0] = abs(grayValueB);
			pixel[1] = abs(grayValueG);
			pixel[2] = abs(grayValueR);
		}
	}
	cv::cvtColor(lightImages[0], lightImages[0], CV_BayerGR2BGR);
	m_matImages[nIndex]._img[0] = lightImages[0];
	cv::cvtColor(lightImages[1], lightImages[1], CV_BayerGR2BGR);
	m_matImages[nIndex]._img[1] = lightImages[1];
	m_matImages[nIndex]._img[2] = outputPseudocolor;
	cv::cvtColor(lightImages[5], lightImages[5], CV_BayerGR2BGR);
	m_matImages[nIndex]._img[3] = lightImages[5];

	bool bCaptureLightImage = System->getParam("camera_cap_image_light").toBool();
	if (bCaptureLightImage)
	{
		QVector<cv::Mat> savingImages;
		for (int i = 0; i < g_nImageStructDataNum; i++)
			savingImages.push_back(m_matImages[nIndex]._img[i]);
		saveImages(szImagePath, 0, nIndex, g_nImageStructDataNum*getPositionNum(), savingImages);
	}

	pUI->setImage(m_matImages[nIndex]._img[m_nImageIndex], true);

	QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_CALCULATE_3D, 0);

	QVector<cv::Mat> matHeights;
	for (int i = 0; i < nStationNum; i++)
	{
		matHeights.push_back((*m_paramData)[i]._3DMatHeight);
	}

	if (!pVision->merge3DHeight(matHeights, m_3DMatHeights[nIndex]))
	{		
		return false;
	}	
	
	return true;
}

bool AutoRunThread::mergeImages(QString& szImagePath)
{
	IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
	if (!pUI) return false;

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	QVector<cv::Mat> matInputImages;
	for each (QImageStruct var in m_matImages)
	{
		for (int i = 0; i < g_nImageStructDataNum; i++)
		{
			matInputImages.push_back(var._img[i]);
		}
	}
	
	QVector<cv::Mat> matOutputImages;
	pVision->mergeImage(matInputImages, matOutputImages);
	if (matOutputImages.size() == g_nImageStructDataNum)
	{
		for (int i = 0; i < matOutputImages.size(); i++)
		{
			m_stCombinedImage._img[i] = matOutputImages.at(i);
		}
	}

	pUI->setImage(m_stCombinedImage._img[m_nImageIndex], true);
	bool bCaptureLightImage = System->getParam("camera_cap_image_light").toBool();
	if (bCaptureLightImage)
	{		
		saveCombineImages(szImagePath, matOutputImages);
	}

	for (int i = 0; i < getPositionNum(); i++)
	{

	}
	pUI->setHeightData(m_3DMatHeight);

	return true;
}

bool AutoRunThread::calculateDetectHeight()
{
	//ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	//if (!pCam) return false;

	//IVision* pVision = getModule<IVision>(VISION_MODEL);
	//if (!pVision) return false;

	//QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_CALCULATE_HEIGHT, 0);

	//if (pVision->calculateDetectHeight(m_3DMatHeight, m_objTests))
	//{
	//	return true;
	//}

	return true;
}

bool AutoRunThread::waitCheckDone()
{
	QEos::Notify(EVENT_CHECK_STATE, 0, STATION_STATE_RESOULT, 1);

	IData * pData = getModule<IData>(DATA_MODEL);
	if (pData)
	{
		pData->decrementCycleTests();
	}

	return true;
}

bool AutoRunThread::doAlignment()
{
    IData * pData = getModule<IData>(DATA_MODEL);
    Vision::VectorOfMat vecFrameImage;
    for (const auto &stFrameImages : m_matImages) {
        vecFrameImage.push_back(stFrameImages._img[m_nImageIndex]);
    }
    return pData->doAlignment(vecFrameImage);
}

bool AutoRunThread::isExit()
{
	return m_exit;
}

void AutoRunThread::setResoultLight(bool isOk)
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

void AutoRunThread::resetResoultLight()
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

bool AutoRunThread::getLightIO(int &okLight, int &ngLight)
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

QString AutoRunThread::generateImagePath()
{
	QString capturePath = System->getParam("camera_cap_image_path").toString();

	QDateTime dtm = QDateTime::currentDateTime();
	QString fileDir = capturePath + "/image/" + dtm.toString("MMddhhmmss") + "/";
	QDir dir; dir.mkdir(fileDir);

	return fileDir;
}

void AutoRunThread::saveImages(const QString& szImagePath, int nRowIndex, int nColIndex, int nCountOfImgPerRow, const QVector<cv::Mat>& imageMats)
{
	int nCountOfImgPerFrame = imageMats.size();
	for (int i = 0; i < nCountOfImgPerFrame; i++)
	{
		int nImageIndex = nRowIndex * nCountOfImgPerRow + nColIndex*nCountOfImgPerFrame + i + 1;

		QString strSave = szImagePath + QString("F") + QString::number(nRowIndex + 1, 'g', 2) + QString("-") + QString::number(nImageIndex, 'g', 2) + QString("-") +
			QString("1") + QString(".bmp");

		IplImage frameImg = IplImage(imageMats[i]);
		cvSaveImage(strSave.toStdString().c_str(), &frameImg);
	}	
}

void AutoRunThread::saveCombineImages(const QString& szImagePath, const QVector<cv::Mat>& imageMats)
{
	for (int i = 0; i < imageMats.size(); i++)
	{
		QString strSave = szImagePath + QString("CombineResult_") + QString::number(i + 1, 'g', 2) + QString(".bmp");

		IplImage frameImg = IplImage(imageMats[i]);
		cvSaveImage(strSave.toStdString().c_str(), &frameImg);
	}
}

cv::Rect AutoRunThread::_calcImageRect(float fImgCapPosUmX, float fImgCapPosUmY, float fRectPosUmX, float fRectPosUmY, float fRectWidthUm, float fRectHeightUm)
{
    float fRectCtrX = (fRectPosUmX - fImgCapPosUmX) / m_dResolutionX;
    float fRectCtrY = (fRectPosUmY - fImgCapPosUmY) / m_dResolutionY;
    float fRectWidth  = fRectWidthUm  / m_dResolutionX;
    float fRectHeight = fRectHeightUm / m_dResolutionY;
    return cv::Rect(fRectCtrX - fRectWidth / 2.f, fRectCtrY - fRectHeight / 2.f, fRectWidth, fRectHeight);
}

bool AutoRunThread::_doAlignment()
{
    bool bResult = true;
    std::vector<AlignmentRunnablePtr> vecAlignmentRunnable;
    Vision::VectorOfPoint2f vecCadPoint, vecSrchPoint;

    for (const auto &alignment : m_vecAlignments) {
        vecCadPoint.push_back(cv::Point2f(alignment.tmplPosX, alignment.tmplPosY));

        if (! moveToCapturePos(alignment.tmplPosX, alignment.tmplPosY)) {
            bResult = false;
            break;
        }
        TimeLogInstance->addTimeLog("Move to capture position.");
        
        QVector<cv::Mat> vecMatImages;
        if (! captureAllImages(vecMatImages)) {
            bResult = false;
            break;
        }
        TimeLogInstance->addTimeLog("Capture all images.");

        cv::Mat matAlignmentImg = vecMatImages[m_nDLPCount * DLP_IMG_COUNT];
        cv::Rect rectSrchWindow = _calcImageRect(alignment.tmplPosX, alignment.tmplPosY, alignment.tmplPosX, alignment.tmplPosY, alignment.srchWinWidth, alignment.srchWinHeight);
        auto pAlignmentRunnable = std::make_unique<AlignmentRunnable>(matAlignmentImg, alignment, rectSrchWindow);
        pAlignmentRunnable->setAutoDelete(false);
        QThreadPool::globalInstance()->start(pAlignmentRunnable.get());
        vecAlignmentRunnable.push_back(std::move(pAlignmentRunnable));
    }

    QThreadPool::globalInstance()->waitForDone();
    TimeLogInstance->addTimeLog("Search for alignment point done.");
    
    for (size_t i = 0; i < m_vecAlignments.size(); ++ i) {        
        auto ptOffset = vecAlignmentRunnable[i]->getResultCtrOffset();
        cv::Point2f ptResult(m_vecAlignments[i].tmplPosX + ptOffset.x, m_vecAlignments[i].tmplPosY + ptOffset.y);
        vecSrchPoint.push_back(ptResult);
    }

    float fRotationInRadian, Tx, Ty, fScale;
    if (vecCadPoint.size() >= 3) {
        if (vecCadPoint.size() == 3)
            m_matTransform = cv::getAffineTransform(vecCadPoint, vecSrchPoint);
        else
            m_matTransform = cv::estimateRigidTransform(vecCadPoint, vecSrchPoint, true);

        m_matTransform.convertTo(m_matTransform, CV_32FC1);
    }else {
        DataUtils::alignWithTwoPoints(vecCadPoint[0],
                                      vecCadPoint[1],
                                      vecSrchPoint[0],
                                      vecSrchPoint[1],
                                      fRotationInRadian, Tx, Ty, fScale);

        cv::Point2f ptCtr(vecCadPoint[0].x / 2.f + vecCadPoint[1].x / 2.f,  vecCadPoint[0].y / 2.f + vecCadPoint[1].y / 2.f);
        double fDegree = fRotationInRadian * 180. / CV_PI;
        m_matTransform = cv::getRotationMatrix2D ( ptCtr, fDegree, fScale );
        m_matTransform.at<double>(0, 2) += Tx;
        m_matTransform.at<double>(1, 2) += Ty;
        m_matTransform.convertTo(m_matTransform, CV_32FC1);
    }
    return bResult;
}

bool AutoRunThread::_alignWindows()
{
    for (auto window : m_vecWindows) {
        std::vector<float> vecSrcPos;
        vecSrcPos.push_back(window.x);
        vecSrcPos.push_back(window.y);
        vecSrcPos.push_back(1);
        cv::Mat matSrcPos(vecSrcPos);
        cv::Mat matDestPos = m_matTransform * matSrcPos;
        window.x = matDestPos.at<float>(0);
        window.y = matDestPos.at<float>(1);
        m_vecAlignedWindows.push_back(window);
    }
    return true;
}

bool AutoRunThread::_doInspection()
{
    bool bGood = true;
    for (int row = 0; row < m_vecVecFrameCtr.size(); ++ row) {
        for (int col = 0; col < m_vecVecFrameCtr.size(); ++ col) {
            auto ptFrameCtr = m_vecVecFrameCtr[row][col];
            if (! moveToCapturePos(ptFrameCtr.x, ptFrameCtr.y)) {
                bGood = false;
                break;
            }

            TimeLogInstance->addTimeLog("Move to capture position.");
        
            QVector<cv::Mat> vecMatImages;
            if (! captureAllImages(vecMatImages)) {
                bGood = false;
                break;
            }
            TimeLogInstance->addTimeLog("Capture all images.");

            std::vector<Calc3dHeightRunnablePtr> vecCalc3dHeightRunnable;
            for (int nDlpId = 0; nDlpId < m_nDLPCount; ++ nDlpId) {
                auto pCalc3DHeightRunnable = std::make_shared<Calc3DHeightRunnable>(nDlpId, vecMatImages);
                pCalc3DHeightRunnable->setAutoDelete(false);
                m_threadPoolCalc3DHeight.start(pCalc3DHeightRunnable.get());
                vecCalc3dHeightRunnable.push_back(std::move(pCalc3DHeightRunnable));
            }

            auto pInsp3DHeightRunnalbe = new Insp3DHeightRunnable(&m_threadPoolCalc3DHeight, vecCalc3dHeightRunnable);
            QThreadPool::globalInstance()->start(pInsp3DHeightRunnalbe);

            Engine::WindowVector vecWindows = _getWindowInFrame(cv::Point2f(ptFrameCtr.x, ptFrameCtr.y));
            Vision::VectorOfMat vec2DImages(vecMatImages.begin() + m_nDLPCount * DLP_IMG_COUNT, vecMatImages.end());
            auto pInsp2DRunnable = new Insp2DRunnable(vec2DImages, vecWindows);
            QThreadPool::globalInstance()->start(pInsp3DHeightRunnalbe);
        }

        if (! bGood)
            break;
    }

    return bGood;
}

Engine::WindowVector AutoRunThread::_getWindowInFrame(const cv::Point2f &ptFrameCtr)
{
    Engine::WindowVector vecResult;
    for (const auto &window : m_vecAlignedWindows) {
        if (DataUtils::isWindowInFrame(cv::Point2f(window.x, window.y), window.width, window.height, ptFrameCtr, m_fFovWidth, m_fFovHeight))
            vecResult.push_back(window);
    }
    return vecResult;
}