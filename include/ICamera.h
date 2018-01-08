#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "../lib/VisionLibrary/include/opencv/cv.h"

#include <QWidget>

class ViewCtrl;
class ICamera
{
public:
	/************************************************************************/
	/*  ϵͳ��ʼ��   */
	/*  Ӳ����������   */
	/************************************************************************/
	virtual void initial(int nWindow = 0 )=0;   // nWindow ������ʾ���ڵĸ����� = 0 �����ô��ڸ���������������� �����������һ������
	virtual void unInit() = 0;

	virtual QWidget* getSettingView() = 0;

	virtual void openCamera() = 0;
	virtual void closeCamera() = 0;
	virtual int getCameraNum() = 0;

	virtual bool grabCamImage(int nCamera, cv::Mat& image, bool bSyns = true) = 0;  // ץȡ���ͼ��
	virtual void setExposureTime(int nCamera, double exposureTime) = 0;             // �����ع�ʱ��

	/************************************************************************/
	/*  ��������غ������漰ͼ��ɼ���������������   */
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
	/*  ��������   */
	/************************************************************************/	
};

#endif