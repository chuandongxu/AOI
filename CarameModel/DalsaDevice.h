#pragma once

#include "CameraDevice.h"
#include "sapclassbasic.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <core/mat.hpp> 

#include <QVector>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

#include <pylon/cameralink/BaslerCameraLinkInstantCamera.h>
//typedef Pylon::CBaslerCameraLinkInstantCamera Camera_t;
using namespace Basler_CLCameraParams;

// Namespace for using pylon objects.
using namespace Pylon;

using namespace cv;

class DalsaCameraDevice : public CameraDevice
{
public:
	DalsaCameraDevice(SapLocation* loc, QString cameraName, QString cameraID);
	~DalsaCameraDevice(void);

public:
	void setCamera(IPylonDevice* dev);
	void openCamera();
	void closeCamera();

	virtual void setExposureTime(double expouserTime);
	virtual void getExposureTime(double *expouserTime);  // 注意, 有些相机是不支持的
	virtual void setHardwareTrigger(bool bOn);
	virtual void setTriggerActive(bool bActiveHigh);
	virtual bool captureImage(cv::Mat &imageMat);
	virtual bool getCameraScreenSize(int& nWidth, int& nHeight);

	virtual void softwareTrigger();
	virtual bool startGrabing(int nNum);
	virtual bool captureImageByFrameTrig(QVector<cv::Mat>& imageMats);
	virtual void stopGrabing();
	virtual void clearGrabing();
	virtual bool isGrabing();

	virtual void openDevice(QString cameraName, QString cameraID, bool bHWTrigger = true);
	virtual void closeDevice();

private:
	static void XferCallback(SapXferCallbackInfo *pInfo);
	static int _sXferIndex;

private:
	QString m_cameraName;
	QString m_cameraID;

	IPylonDevice* m_hv_AcqHandle;
	Pylon::CBaslerCameraLinkInstantCamera* m_camera;

	SapLocation		*m_loc;
	SapAcquisition	*m_Acq;
	//SapAcqDevice	*m_AcqDevice;
	SapBuffer		*m_Buffers;
	SapTransfer		*m_Xfer;
	SapFeature     *m_Feature;
	//SapView			*m_View;

	QVector<cv::Mat> m_imageMats;
	int m_nGrabNum;
	int m_nGrabCount;

	bool m_bStopFlag;
	bool m_bCapturedImage;
};
