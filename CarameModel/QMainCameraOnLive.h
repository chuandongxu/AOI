#pragma once

#include <QObject>
#include <QThread>

#include "opencv/cv.h"

class QMainProcess;
class CameraDevice;
class MainCameraOnLive : public QThread
{
	Q_OBJECT

public:
	MainCameraOnLive(QMainProcess* pMainProcess, CameraDevice *pCameraTmp, bool bHWTrigger);
	~MainCameraOnLive(){};

public:
	void setQuitFlag();
	bool isRuning(){ return m_bRuning; };

private:	
	void preProcess();
	void run();
	void postProcess();

private:
	void saveImages(QVector<cv::Mat>& images);

	void drawCross(cv::Mat& image);
private:
	QMainProcess*		m_pMainProcess;
	CameraDevice *m_pCameraTmp;
	bool		  m_bHWTrigger;
	bool          m_bQuit;
	bool          m_bRuning;
	bool		  m_bPreCondition;
};