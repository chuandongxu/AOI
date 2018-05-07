#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "../lib/VisionLibrary/include/opencv/cv.h"

#include <QWidget>

class ViewCtrl;
class ICamera
{
public:
	enum TRIGGER
	{
		TRIGGER_ALL,// 54 pcs
		TRIGGER_DLP_ALL, // 48 pcs
		TRIGGER_DLP, // 12 pcs
		TRIGGER_ONE, // 1 pcs
		TRIGGER_LIGHT, // 6 pcs		
	};

	/************************************************************************/
	/*  ϵͳ��ʼ��   */
	/*  Ӳ����������   */
	/************************************************************************/
	virtual void initial(int nWindow = 0 )=0;   // nWindow ������ʾ���ڵĸ����� = 0 �����ô��ڸ���������������� �����������һ������
	virtual void unInit() = 0;

	virtual QWidget* getCalibrationView() = 0;

	virtual void openCamera() = 0;
	virtual void closeCamera() = 0;
	virtual int getCameraNum() = 0;

	virtual bool grabCamImage(int nCamera, cv::Mat& image, bool bSyns = true) = 0;  // ץȡ���ͼ��
	virtual void setExposureTime(int nCamera, double exposureTime) = 0;             // �����ع�ʱ��


	/************************************************************************/
	/*  ��������غ������漰ͼ��ɼ���������������   */
	/************************************************************************/
	virtual bool startUpCapture(bool bHWTrigger) = 0;
	virtual bool isHWTrigger() = 0;
	virtual bool endUpCapture() = 0;
	virtual bool selectCaptureMode(TRIGGER emCaptureMode, bool reStartUp = false) = 0; 
	
	virtual bool startCapturing() = 0;
	virtual bool getImages(QVector<cv::Mat>& imageMats) = 0;
	virtual bool getLastImages(QVector<cv::Mat>& imageMats) = 0;
	virtual bool stopCapturing() = 0;

    virtual bool captureAllImages(QVector<cv::Mat>& imageMats) = 0;

	/************************************************************************/
	/*  ��������   */
	/************************************************************************/	
	virtual bool getCameraScreenSize(int& nWidth, int& nHeight) = 0;
};

#endif