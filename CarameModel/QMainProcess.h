#pragma once

#include <QObject>
#include <QVector>
#include "../include/ICamera.h"

#include <QMutex>

#include "opencv/cv.h"

class CameraCtrl;
class MainCameraOnLive;
class QMainProcess : public QObject
{
	Q_OBJECT

public:
	QMainProcess(CameraCtrl* pCameraCtrl, QObject *parent = NULL);
	~QMainProcess();

public:
	void pushImageBuffer(cv::Mat& matImage);
	void setImageBuffer(QVector<cv::Mat>& matImages);
	const QVector<cv::Mat>& getImageBuffer();
	const cv::Mat& getImageItemBuffer(int nIndex);
	int getImageBufferNum();
	int getImageBufferCaptureNum();
	void clearImageBuffer();
	bool startCapturing();
	void setCaptureImageBufferDone();
	bool isCaptureImageBufferDone();
	bool lockCameraCapture();
	void unlockCameraCapture();
	bool isCameraCaptureAvaiable();

	bool startUpCapture(bool bHWTrigger);
	bool isHWTrigger();
	bool endUpCapture();
	bool selectCaptureMode(ICamera::TRIGGER emCaptureMode);

private:
	CameraCtrl* m_pCameraCtrl;

	cv::Mat	m_hoImage;
	cv::Mat	m_dispImage;

private:
	MainCameraOnLive * m_pCameraOnLive;
	QVector<cv::Mat> m_bufferImages;
	int m_nCaptureNum;
	bool m_bHWTrigger;
	bool m_bCaptureDone;
	bool m_bCaptureLocker;
	QMutex m_mutex;
};
