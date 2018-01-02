#pragma once

#include <QVector>
#include "CameraDevice.h"

#include "opencv/cv.h"
using namespace cv;

class ImageDevice : public CameraDevice
{
public:
	ImageDevice(QString cameraName, QString cameraID);
	~ImageDevice();

public:
	virtual void setExposureTime(double expouserTime);
	virtual void getExposureTime(double *expouserTime);  // 注意, 有些相机是不支持的
	virtual void setHardwareTrigger(bool bOn);
	virtual bool captureImage(cv::Mat &imageMat);

	virtual void softwareTrigger();
	virtual bool startGrabing(int nNum);
	virtual bool captureImageByFrameTrig(QVector<cv::Mat>& imageMats);
	virtual void stopGrabing();
	virtual void clearGrabing();
	virtual bool isGrabing();

	virtual void openDevice(QString cameraName, QString cameraID, bool bHWTrigger = true);
	virtual void closeDevice();

private:
	bool convertToGrayImage(QString& szFilePath, cv::Mat &matGray);
	bool readImages(QString& szFilePath, QVector<cv::Mat>& matImgs);
	QString getImageFilePath();

private:
	QString m_cameraName;
	QString m_cameraID;

	QVector<cv::Mat> m_imageMats;

	int m_nGrabNum;
	int m_nGrabCount;
	bool m_bStopFlag;
};
