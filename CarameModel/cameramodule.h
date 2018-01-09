#ifndef CARAMEMODEL_H
#define CARAMEMODEL_H

#include "caramemodel_global.h"
#include "../include/ICamera.h"
#include "../common/modulebase.h"

#include "opencv/cv.h"

class CameraCtrl;
class CameraSetting;
class QMainProcess;
class CameraModule : public QModuleBase, public ICamera
{
public:
	CameraModule(int id,const QString &name);
	~CameraModule();

     void initial(int nWindow = 0 );
	 void unInit();

public:
	QWidget* getSettingView();

	bool startUpCapture();
	bool endUpCapture();

	const QVector<cv::Mat>& getImageBuffer();
	const cv::Mat& getImageItemBuffer(int nIndex);
	int getImageBufferNum();
	int getImageBufferCaptureNum();
	bool startCapturing();
	void clearImageBuffer();
	bool isCaptureImageBufferDone();

	bool lockCameraCapture(int iStation);
	void unlockCameraCapture();
	bool isCameraCaptureAvaiable();

public:
	 void openCamera();
	 void closeCamera();
	 int getCameraNum();	

	 bool grabCamImage(int nCamera, cv::Mat& image, bool bSync);
	 void setExposureTime(int nCamera, double exposureTime);	

private:
	virtual void addSettingWiddget(QTabWidget *tabWidget);
    virtual void showSettingWidget();
	void setErrorMap();

private: 
	CameraCtrl*      m_pCameraCtrl;
	CameraSetting* m_pSetting;
	QSharedPointer<QMainProcess> m_pMainProcess;
};

#endif // CARAMEMODEL_H
