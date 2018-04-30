#pragma once

#include <QObject>
#include <QVector>
#include "../include/ICamera.h"

#include <QMutex>

#include "opencv/cv.h"

class CameraCtrl;
class QMainProcess : public QObject
{
	Q_OBJECT

public:
	QMainProcess(CameraCtrl* pCameraCtrl, QObject *parent = NULL);
	~QMainProcess();

public:
	bool lockCameraCapture();
	void unlockCameraCapture();
	bool isCameraCaptureAvaiable();

	bool startUpCapture(bool bHWTrigger);
	bool isHWTrigger();
	bool endUpCapture();
	bool selectCaptureMode(ICamera::TRIGGER emCaptureMode, bool reStartUp);

	bool startCapturing();
	bool getImages(QVector<cv::Mat>& imageMats);
	bool getLastImages(QVector<cv::Mat>& imageMats);
	bool stopCapturing();
	bool isStartCapturing();

private:
	void saveImages(QVector<cv::Mat>& images);

private:
	CameraCtrl* m_pCameraCtrl;

	cv::Mat	m_hoImage;
	cv::Mat	m_dispImage;

private:
	QVector<cv::Mat> m_bufferImages;
	ICamera::TRIGGER m_emCaptureMode;
	int m_nCaptureNum;
	bool m_bHWTrigger;
	bool m_bCaptureDone;
	bool m_bCaptureLocker;
	QMutex m_mutex;
};
