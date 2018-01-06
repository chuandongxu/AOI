#ifndef CARAMEMODEL_H
#define CARAMEMODEL_H

#include "caramemodel_global.h"
#include "../include/ICamera.h"
#include "../common/modulebase.h"
#include "QMainView.h"
#include "CameraCtrl.h"
#include "viewctrl.h"

#include "opencv/cv.h"

class CameraSetting;
class CameraModule : public QModuleBase, public ICamera
{
public:
	CameraModule(int id,const QString &name);
	~CameraModule();

     void initial(int nWindow = 0 );
	 void unInit();

public:
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

	void addImageText(QString& szText);
	void clearImage();
	void setImage(cv::Mat& image, bool bDisplay);

	void setHeightData(cv::Mat& matHeight);

public:
	 void openCamera();
	 void closeCamera();
	 int getCameraNum();	

	 bool grabCamImage(int nCamera, cv::Mat& image, bool bSync);
	 void setExposureTime(int nCamera, double exposureTime);	

	 //»ñÈ¡Í¼Ïñ´°¿Ú	
	 virtual QWidget* getMainView();

	 void load3DViewData(int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues);
	 void show3DView();

private:
	virtual void addSettingWiddget(QTabWidget *tabWidget);
    virtual void showSettingWidget();
	void setErrorMap();

private: 
	CameraCtrl*      m_pCameraCtrl;
	CameraSetting* m_pSetting;
	QMainView* m_pMainView;
};

#endif // CARAMEMODEL_H
