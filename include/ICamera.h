#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "../lib/VisionLibrary/include/opencv/cv.h"

#include <QWidget>

class ViewCtrl;
class ICamera
{
public:
	/************************************************************************/
	/*  系统初始化   */
	/*  硬件操作功能   */
	/************************************************************************/
	virtual void initial(int nWindow = 0 )=0;   // nWindow 设置显示窗口的个数， = 0 则设置窗口个数等于相机个数， 如果无相机则打开一个窗口
	virtual void unInit() = 0;

	virtual QWidget* getSettingView() = 0;

	virtual void openCamera() = 0;
	virtual void closeCamera() = 0;
	virtual int getCameraNum() = 0;

	virtual bool grabCamImage(int nCamera, cv::Mat& image, bool bSyns = true) = 0;  // 抓取相机图像
	virtual void setExposureTime(int nCamera, double exposureTime) = 0;             // 设置曝光时间

	/************************************************************************/
	/*  主流程相关函数，涉及图像采集，交互操作功能   */
	/************************************************************************/
	virtual bool startUpCapture() = 0;
	virtual bool endUpCapture() = 0;
	
	virtual const QVector<cv::Mat>& getImageBuffer() = 0;
	virtual const cv::Mat& getImageItemBuffer(int nIndex) = 0;
	virtual int getImageBufferNum() = 0;
	virtual int getImageBufferCaptureNum() = 0;
	virtual bool startCapturing() = 0;
	virtual void clearImageBuffer() = 0;
	virtual bool isCaptureImageBufferDone() = 0;

	virtual bool lockCameraCapture(int iStation) = 0;
	virtual void unlockCameraCapture() = 0;
	virtual bool isCameraCaptureAvaiable() = 0;
	

	/************************************************************************/
	/*  辅助功能   */
	/************************************************************************/	
};

#endif