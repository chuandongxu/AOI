#pragma once
#include "QString"
// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

// Namespace for using pylon objects.
using namespace Pylon;

#include "opencv/cv.h"

// Settings to use Basler GigE cameras.
#include <pylon/gige/BaslerGigEInstantCamera.h>
#include <pylon/AcquireSingleFrameConfiguration.h>
#include <pylon/SoftwareTriggerConfiguration.h> 

//typedef Pylon::CBaslerGigEInstantCamera Camera_t;
using namespace Basler_GigECameraParams;

class CameraDevice 
{
public:
    // CameraDevice(void);
	CameraDevice(QString cameraName, QString cameraID);
	~CameraDevice(void);

public:
	virtual void setExposureTime( double expouserTime) = 0;
	virtual void getExposureTime(double *expouserTime) = 0;  // 注意, 有些相机是不支持的
	virtual void setHardwareTrigger(bool bOn) = 0;
	virtual void setTriggerActive(bool bActiveHigh) = 0;
	virtual bool captureImage(cv::Mat &imageMat) = 0;
	virtual bool getCameraScreenSize(int& nWidth, int& nHeight) = 0;

	virtual void softwareTrigger() = 0;
	virtual bool startGrabing(int nNum) = 0;
	virtual bool captureImageByFrameTrig(QVector<cv::Mat>& imageMats) = 0;
	virtual void stopGrabing() = 0;	
	virtual void clearGrabing() = 0;
	virtual bool isGrabing() = 0;
	
	virtual void openDevice(QString cameraName, QString cameraID, bool bHWTrigger = true) = 0;
	virtual void closeDevice() = 0;

public:
	bool getStatus();
	void saveImage(cv::Mat& image, QString name = QString(""), QString path = QString(""));

protected:	
	double m_hv_expouserTime;
	bool   m_bOpen ;
	bool   m_bSaveAsEncrypt;
};

typedef QList<CameraDevice*> CameraDeviceList;


class BaslerCameraDevice : public CameraDevice
{
public:
	BaslerCameraDevice(IPylonDevice* dev, QString cameraName, QString cameraID);
	~BaslerCameraDevice(void);

public:
	virtual void setExposureTime(double expouserTime);
	virtual void getExposureTime(double *expouserTime);  // 注意, 有些相机是不支持的
	virtual void setHardwareTrigger(bool bOn);
	virtual void setTriggerActive(bool bActiveHigh){}
	virtual bool captureImage(cv::Mat &imageMat);
	virtual bool getCameraScreenSize(int& nWidth, int& nHeight);

	virtual void softwareTrigger();
	virtual bool startGrabing(int nNum);
	virtual bool captureImageByFrameTrig(QVector<cv::Mat>& imageMats);
	virtual void stopGrabing();
	virtual void clearGrabing(){}
	virtual bool isGrabing();

	virtual void openDevice(QString cameraName, QString cameraID, bool bHWTrigger = true);
	virtual void closeDevice();

private:
	IPylonDevice* m_hv_AcqHandle;
	Pylon::CBaslerGigEInstantCamera* m_camera;
	bool m_bStopFlag;
};