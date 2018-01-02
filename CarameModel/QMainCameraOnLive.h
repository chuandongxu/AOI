#pragma once

#include <QObject>
#include <QThread>

#include "opencv/cv.h"

class QMainView;
class CameraDevice;
class MainCameraOnLive : public QThread
{
	Q_OBJECT

public:
	MainCameraOnLive(QMainView* pView, CameraDevice *pCameraTmp);
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
	QMainView*		m_pView;
	CameraDevice *m_pCameraTmp;
	bool          m_bQuit;
	bool          m_bRuning;
	bool		  m_bPreCondition;
};