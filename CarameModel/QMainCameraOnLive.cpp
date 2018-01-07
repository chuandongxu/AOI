#include "QMainCameraOnLive.h"

#include "QMainProcess.h"
#include "CameraCtrl.h"

#include <QFileDialog>
#include "../Common/SystemData.h"
#include <qdatetime.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define ToInt(value)                (static_cast<int>(value))

MainCameraOnLive::MainCameraOnLive(QMainProcess* pMainProcess, CameraDevice *pCameraTmp)
	:m_pMainProcess(pMainProcess), m_pCameraTmp(pCameraTmp)
{
	m_bQuit = false;
	m_bRuning = false;
	m_bPreCondition = false;
}

void MainCameraOnLive::preProcess()
{
	if (!m_bPreCondition)
	{
		//m_pCameraTmp->startGrabing(1);
		//m_pCameraTmp->softwareTrigger();
		//QThread::msleep(100);
		//m_pCameraTmp->softwareTrigger();
		//QVector<cv::Mat> buffers;
		//if (m_pCameraTmp->captureImageByFrameTrig(buffers))
		//{
		//	m_pCameraTmp->stopGrabing();
		//}
		//else
		//{
		//	m_pCameraTmp->stopGrabing();
		//}
		//m_pCameraTmp->clearGrabing();

		m_bPreCondition = false;
	}

	m_pMainProcess->clearImageBuffer();
	m_pMainProcess->setCaptureImageBufferDone();

	m_bRuning = true;
}

void MainCameraOnLive::run()
{
	preProcess();
	
	while (!m_bQuit)
	{
		if (m_pMainProcess->isCaptureImageBufferDone() && !m_bQuit)//debug functions
		{
			QThread::msleep(1);			
			continue;
		}		
		
		if (m_pMainProcess->getStation() == 0)
		{
			System->setTrackInfo(QStringLiteral("启动测试..."));
		}		
		else
		{
			//System->setTrackInfo(QStringLiteral("start grabing by station %1").arg(m_pView->getStation()));
		}

		if (!m_pCameraTmp->startGrabing(m_pMainProcess->getImageBufferNum()))
		{
			System->setTrackInfo("startGrabing error!");
			QThread::msleep(100);
			continue;
		}		

		QVector<cv::Mat> buffers;
		if (m_pCameraTmp->captureImageByFrameTrig(buffers))
		{
			m_pCameraTmp->stopGrabing();			
			m_pMainProcess->setImageBuffer(buffers);
			m_pMainProcess->setCaptureImageBufferDone();

			//System->setTrackInfo(QString("XXX System captureImages Image Num: %1").arg(buffers.size()));

			saveImages(buffers);
		}
		else
		{
			m_pCameraTmp->stopGrabing();
			m_pMainProcess->setCaptureImageBufferDone();
			System->setTrackInfo("Capture Images Time Out. Try again!");
		}

		m_pCameraTmp->clearGrabing();

		if (m_bQuit)break;

		QThread::msleep(1);		
	}

	postProcess();	
}

void MainCameraOnLive::postProcess()
{
	m_bRuning = false;
}

void MainCameraOnLive::saveImages(QVector<cv::Mat>& images)
{
	//showImageToScreen(buffers.last());
	bool bCaptureImage = System->getParam("camera_cap_image_enable").toBool();
	if (bCaptureImage)
	{
		QString capturePath = System->getParam("camera_cap_image_path").toString();

		QDateTime dtm = QDateTime::currentDateTime();
		QString fileDir = capturePath + "/" + dtm.toString("MMddhhmmss") + "/";
		QDir dir; dir.mkdir(fileDir);

		bool bCaptureImageAsMatlab = System->getParam("camera_cap_image_matlab").toBool();
		for (int i = 0; i < images.size(); i++)
		{
			cv::Mat image = images.at(i);
			if (bCaptureImageAsMatlab)
			{
				int nIndex = i + 1;
				if (6 == nIndex)
				{
					nIndex += 1;
				}
				else if (7 == nIndex)
				{
					nIndex -= 1;
				}
				QString name = QString("%1").arg(nIndex, 2, 10, QChar('0')) + QStringLiteral(".bmp");
				m_pCameraTmp->saveImage(image, name, fileDir);
			}
			else
			{
				m_pCameraTmp->saveImage(image);
			}
		}
	}
}

void MainCameraOnLive::setQuitFlag()
{
	bool bHardwareTrigger = System->getParam("camera_hw_tri_enable").toBool();
	if (bHardwareTrigger)
	{
		m_pCameraTmp->stopGrabing();
	}
	m_bQuit = true;
}

void MainCameraOnLive::drawCross(cv::Mat& image)
{
	cv::Point startPt, endPt;
	startPt.x = 0;
	startPt.y = image.size().height / 2;
	endPt.x = image.size().width;
	endPt.y = image.size().height / 2;
	cv::line(image, startPt, endPt, cv::Scalar(255, 0, 0), 2, 8);

	startPt.x = image.size().width / 2;
	startPt.y = 0;
	endPt.x = image.size().width / 2;
	endPt.y = image.size().height;
	cv::line(image, startPt, endPt, cv::Scalar(255, 0, 0), 2, 8);
}