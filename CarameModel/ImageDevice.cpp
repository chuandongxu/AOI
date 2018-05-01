#include "ImageDevice.h"
#include <QDir>

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IVision.h"
#include "QDebug.h"
#include <QDateTime>


#include "CryptLib.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

ImageDevice::ImageDevice(QString cameraName, QString  cameraID)
	:CameraDevice(cameraName, cameraID)
{
	m_cameraName = cameraName;
	m_cameraID = cameraID;

	openDevice(cameraName, cameraID);

	m_nGrabNum = 0;
	m_nGrabCount = 0;	
}

ImageDevice::~ImageDevice()
{
	closeDevice();
}

void ImageDevice::openDevice(QString cameraName, QString cameraID, bool bHWTrigger)
{
	try
	{
		m_bOpen = true;
	}
	catch (...)
	{
		// Error handling.
		qDebug() << "An exception occurred." << endl;
		m_bOpen = false;
	}
}

void ImageDevice::closeDevice()
{
	if (m_bOpen)
	{
		try
		{
			m_bOpen = false;
		}
		catch (...)
		{
			// Error handling.
			qDebug() << "An exception occurred." << endl;
		}
	}
}

void ImageDevice::getExposureTime(double *exposureTime)
{
	if (m_bOpen)
	{
	}
	else
	{
		*exposureTime = 0;
	}
}

void ImageDevice::setExposureTime(double exposureTime)
{
	if (m_bOpen)
	{
	}
}

void ImageDevice::setHardwareTrigger(bool bOn)
{
	if (m_bOpen)
	{
	}
}


bool ImageDevice::captureImage(cv::Mat &imageMat)
{
	if (!m_bOpen) return false;

	m_imageMats.clear();
	m_nGrabNum = 1;
	m_nGrabCount = 0;	

	QString filePath = "";
	filePath = getImageFilePath();

	if (filePath.isEmpty())
	{
		return false;
	}

	if (convertToGrayImage(filePath, imageMat))
	{
		return true;
	}

	return false;
}

void ImageDevice::softwareTrigger()
{
	if (!m_bOpen) return;	
}

bool ImageDevice::startGrabing(int nNum)
{
	if (!m_bOpen) return false;

	m_imageMats.clear();
	m_nGrabNum = nNum;
	m_nGrabCount = 0;
	m_bStopFlag = false;
		
	return true;
}

bool ImageDevice::captureImageByFrameTrig(QVector<cv::Mat>& imageMats)
{
	if (!m_bOpen) return false;

	QString filePath = "";
	filePath = getImageFilePath();

	if (filePath.isEmpty())
	{
		return false;
	}

	if (!readImages(filePath, m_imageMats))
	{
		return false;
	}

	for (int i = 0; i < m_imageMats.size(); i++)
	{
		imageMats.push_back(m_imageMats[i]);
	}

	return true;
}

void ImageDevice::stopGrabing()
{
	if (!m_bOpen) return;
	m_bStopFlag = true;
}

void ImageDevice::clearGrabing()
{
	m_imageMats.clear();
	m_nGrabNum = 0;
	m_nGrabCount = 0;
}

bool ImageDevice::isGrabing()
{
	if (!m_bOpen) return false;

	return !m_bStopFlag;
}

bool ImageDevice::convertToGrayImage(QString& szFilePath, cv::Mat &matGray)
{
	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	//判断路径是否存在
	QDir dir(szFilePath);
	if (!dir.exists())
	{
		return false;
	}
	dir.setFilter(QDir::Files | QDir::NoSymLinks);
	QFileInfoList list = dir.entryInfoList();

	int file_count = list.count();
	if (file_count <= 0)
	{
		return false;
	}

	QVector<cv::Mat> imageMats;
	for (int i = 0; i < file_count; i++)
	{
		QFileInfo file_info = list.at(i);
		QString suffix = file_info.suffix();
		if (QString::compare(suffix, QString("ent"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			QString nameDecrypt = file_info.absolutePath() + "/" + file_info.baseName() + ".bmp";
			AOI::Crypt::DecryptFileNfg(absolute_file_path.toStdString(), nameDecrypt.toStdString());

			cv::Mat mat = cv::imread(nameDecrypt.toStdString(), cv::IMREAD_GRAYSCALE);
			imageMats.push_back(mat);

			QFile::remove(nameDecrypt);
		}
		else if (QString::compare(suffix, QString("bmp"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			cv::Mat mat = cv::imread(absolute_file_path.toStdString(), cv::IMREAD_GRAYSCALE);
			imageMats.push_back(mat);
		}
		else
		{
			return false;
		}
	}

	pVision->generateAverageImage(imageMats, matGray);

	return true;
}

bool ImageDevice::readImages(QString& szFilePath, QVector<cv::Mat>& matImgs)
{
	//判断路径是否存在
	QDir dir(szFilePath);
	if (!dir.exists())
	{
		return false;
	}
	dir.setFilter(QDir::Files | QDir::NoSymLinks);
	QFileInfoList list = dir.entryInfoList();

	const int IMAGE_COUNT = m_nGrabNum;

	int file_count = list.count();
	if (file_count < IMAGE_COUNT)
	{
		return false;
	}

	for (int i = 0; i < file_count; i++)
	{
		QFileInfo file_info = list.at(i);
		QString suffix = file_info.suffix();
		if (QString::compare(suffix, QString("ent"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			QString nameDecrypt = file_info.absolutePath() + "/" + file_info.baseName() + ".bmp";
			AOI::Crypt::DecryptFileNfg(absolute_file_path.toStdString(), nameDecrypt.toStdString());

			cv::Mat mat = cv::imread(nameDecrypt.toStdString(), cv::IMREAD_GRAYSCALE);
			matImgs.push_back(mat);

			QFile::remove(nameDecrypt);
		}
		else if (QString::compare(suffix, QString("bmp"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			cv::Mat mat = cv::imread(absolute_file_path.toStdString(), cv::IMREAD_GRAYSCALE);
			matImgs.push_back(mat);
			qDebug() << file_info.fileName();
		}
		else
		{
			return false;
		}
	}
	return true;
}

QString ImageDevice::getImageFilePath()
{
	int nDLPIndex = System->getParam("dlp_image_capture_index").toInt();
	QString capturePath = System->getParam(QString("camera_cap_image_dlp%1_path").arg(nDLPIndex + 1)).toString();
	if (capturePath.isEmpty())
	{
		return "";
	}

	QDir dir(capturePath);
	if (!dir.exists())
	{
		return "";
	}

	dir.setFilter(QDir::Dirs);
	dir.setSorting(QDir::Time);
	QFileInfoList list = dir.entryInfoList();

	int file_count = list.count();
	if (file_count <= 0)
	{
		return "";
	}

	QString filePath = "";
	QDateTime dateTime;  dateTime.setTime_t(0);
	for (int i = 0; i < file_count; i++)
	{
		QFileInfo file_info = list.at(i);
		QString absolute_file_path = file_info.absoluteFilePath();
		if (file_info.isDir() && !(file_info.fileName() == "." | file_info.fileName() == ".."))
		{
			QDateTime fileDate = file_info.lastModified();
			if (fileDate > dateTime)
			{
				dateTime = fileDate;
				filePath = absolute_file_path;				
			}
		}
	}

	return filePath;
}

