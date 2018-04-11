﻿#include <QDir>
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

AutoRunThread::AutoRunThread(const Engine::AlignmentVector         &vecAlignments,
                             const Engine::WindowVector            &vecWindows,
                             const Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr)
	:m_vecAlignments  (vecAlignments),
     m_vecWindows     (vecWindows),
     m_vecVecFrameCtr (vecVecFrameCtr),
     m_exit           (false)
{
	m_nImageIndex = 0;
}

AutoRunThread::~AutoRunThread()
{
}

void AutoRunThread::quit()
{
	resetResoultLight();
	m_exit = true;
}

bool AutoRunThread::preRunning()
{
    m_dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    m_dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    m_nDLPCount = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
    m_nImageWidthPixel =  2032; //This value temporarily hard code here, later it should get from config file for camera API.
    m_nImageHeightPixel = 2032;
	return true;
}

void AutoRunThread::run()
{
	if (! preRunning()) return;

	System->setTrackInfo(QString(QStringLiteral("主流程启动成功!")));

	double dtime_start = 0, dtime_movePos = 0;
	while (! isExit())
	{
		if (! waitStartBtn()) continue;
		if (isExit()) break;

        _feedBoard();
        _readBarcode();

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

	IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return false;

	imageMats.clear();

	if (!pCam->startCapturing())
	{
		System->setTrackInfo(QString("startCapturing error."));	
		return false;
	}
	
	if (!pMotion->triggerCapturing(IMotion::TRIGGER_ALL, true))
	{
		System->setTrackInfo(QString("triggerCapturing error."));	
		return false;
	}	

	int nWaitTime = 10 * 100;
	while (! pCam->isCaptureImageBufferDone() && nWaitTime-- > 0 && ! isExit())
	{
		QThread::msleep(10);
	}

	if (nWaitTime <= 0)
	{
		System->setTrackInfo(QString("CaptureImageBufferDone error."));		
		return false;
	}

	int nCaptureNum = pCam->getImageBufferCaptureNum();
	for (int i = 0; i < nCaptureNum; ++ i)
	{
		cv::Mat matImage = pCam->getImageItemBuffer(i);
		imageMats.push_back(matImage);
	}

	if (nCaptureNum != pCam->getImageBufferNum())
	{	
		System->setTrackInfo(QString("System captureAllImages error, Image Num: %1").arg(nCaptureNum));

		bool bCaptureImage = System->getParam("camera_cap_image_enable").toBool();
		if (! bCaptureImage)
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

	return true;
}

bool AutoRunThread::mergeImages(QString& szImagePath)
{
	return true;
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

bool AutoRunThread::_feedBoard()
{
    // Track motor on to drag the PCB board into the machine and put under camera.
    // Need to implement later.
    return true;
}

bool AutoRunThread::_readBarcode()
{
    // Read the bard code.
    // Need to implement later.
    return true;
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

            Engine::WindowVector vecWindows = _getWindowInFrame(ptFrameCtr);
            Vision::VectorOfMat vec2DCaptureImages(vecMatImages.begin() + m_nDLPCount * DLP_IMG_COUNT, vecMatImages.end());
            if (vec2DCaptureImages.size() != CAPTURE_2D_IMAGE_SEQUENCE::TOTAL_COUNT) {
                System->setTrackInfo(QString(QStringLiteral("2D image count %0 not correct.")).arg(vec2DCaptureImages.size()));
                bGood = false;
                break;
            }

            auto vec2DImages = _generate2DImages(vec2DCaptureImages);
            auto pInsp2DRunnable = new Insp2DRunnable(vec2DImages, vecWindows, ptFrameCtr);
            pInsp2DRunnable->setResolution(m_dResolutionX, m_dResolutionY);
            pInsp2DRunnable->setImageSize(m_nImageWidthPixel, m_nImageHeightPixel);
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
        if (DataUtils::isWindowInFrame(cv::Point2f(window.x, window.y), window.width, window.height, ptFrameCtr, m_fFovWidthUm, m_fFovHeightUm))
            vecResult.push_back(window);
    }
    return vecResult;
}

Vision::VectorOfMat AutoRunThread::_generate2DImages(const Vision::VectorOfMat &vecInputImages)
{
    assert(vecInputImages.size() == CAPTURE_2D_IMAGE_SEQUENCE::TOTAL_COUNT);

    Vision::VectorOfMat vecResultImages;
    cv::Mat matRed, matGreen, matBlue;
    bool bColorCamera = false;
    if (bColorCamera) {
        cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::RED_LIGHT],   matRed,   CV_BayerGR2GRAY);
        cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::GREEN_LIGHT], matGreen, CV_BayerGR2GRAY);
        cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::BLUE_LIGHT],  matBlue,  CV_BayerGR2GRAY);
    }else {
        matRed   = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::RED_LIGHT];
        matGreen = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::GREEN_LIGHT];
        matBlue  = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::BLUE_LIGHT];
    }

    Vision::VectorOfMat vecChannels{matBlue, matGreen, matRed};
    cv::Mat matPseudocolorImage;
    cv::merge(vecChannels, matPseudocolorImage);

    cv::Mat matTopLightImage = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::WHITE_LIGHT];
    if (bColorCamera)
	    cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::WHITE_LIGHT], matTopLightImage, CV_BayerGR2BGR);
	vecResultImages.push_back(matTopLightImage);

    cv::Mat matLowAngleLightImage = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::LOW_ANGLE_LIGHT];
    if (bColorCamera)
	    cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::LOW_ANGLE_LIGHT], matLowAngleLightImage, CV_BayerGR2BGR);
    vecResultImages.push_back(matLowAngleLightImage);

    vecResultImages.push_back(matPseudocolorImage);

	cv::Mat matUniformLightImage = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::UNIFORM_LIGHT];
    if (bColorCamera)
	    cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::UNIFORM_LIGHT], matUniformLightImage, CV_BayerGR2BGR);
	vecResultImages.push_back(matUniformLightImage);

    return vecResultImages;
}