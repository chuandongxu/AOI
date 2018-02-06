#ifndef FLOWCTRL_H
#define FLOWCTRL_H

#include <QObject>
#include <qmap.h>
#include <qvector.h>
#include "hsgworkflowctrl_global.h"
#include "../include/ICAN.h"
#include <qrunnable.h>
#include <qstringlist.h>
#include "LineNormal.h"
#include <QTcpServer>
#include <qtcpsocket.h>
#include <QElapsedTimer>
#include <QDateTime>

#include "opencv/cv.h"

using namespace cv;

const int g_nImageStructDataNum = 5;
struct QImageStruct
{
public:
	QImageStruct()
	{
	}

	cv::Mat _img[g_nImageStructDataNum];
};

struct Q3DStructData
{
public:
	Q3DStructData()
	{
		_bStartCapturing = false;
		_bCapturedDone = false;
	}

	void clear()
	{
		_bStartCapturing = false;
		_bCapturedDone = false;
		_3DMatHeight.release();
		_matImage.release();
		_srcImageMats.clear();
	}

	bool _bStartCapturing;
	bool _bCapturedDone;
	cv::Mat _3DMatHeight;
	cv::Mat _matImage;
	QVector<cv::Mat> _srcImageMats;	
};

//---------------------------------------------------------------
typedef QMap<QString, QVariant> QCheckerParamMap;
typedef QList<Q3DStructData> QCheckerParamDataList;
typedef QMap<int, int> QCheckerPositionMap;

//-------------------------------------------------------------
class QDetectObj;
class QCheckerRunable : public QRunnable
{
public:
	QCheckerRunable(QCheckerParamMap *paramMap, Q3DStructData* dataParam);
	~QCheckerRunable();

	void quit();
	void imgStop();//stop steppers
protected:
	void run();

	bool startUpSetup();

	bool waitStartBtn();
	bool captureImages();
	bool generateGrayImage(bool bMotionCardTrigger);
	bool calculate3DHeight(bool bMotionCardTrigger);
	bool waitCheckDone();

	bool endUpSetup();

	bool isExit();

private:
	int getStationID();

private:
	void addImageText(cv::Mat image, Point ptPos, QString szText);

private:
	QCheckerParamMap *m_paramMap;
	Q3DStructData* m_dataParam;

	bool m_exit;
	QLineNormal m_normal;
	QMutex m_mutex;

	QVector<cv::Mat> m_imageMats;
	cv::Mat m_3DMatHeight;
	cv::Mat m_matHeightResultImg;
	cv::Mat m_matImage;
};

class QMainRunable : public QRunnable
{
public:
	QMainRunable(QCheckerParamMap *paramMap, QCheckerParamDataList* paramData);
	~QMainRunable();

	void quit();
	void imgStop();//stop steppers

	void setImageIndex(int nIndex);

private:
	int getPositionNum();
	int getPositionID(int nIndex);

protected:
	bool preRunning();

	void run();
	
	bool waitStartBtn();
	bool moveToReadyPos();
	bool moveToCapturePos(int nIndex);
	bool captureImages(int nIndex, QString& szImagePath);
	bool mergeImages();
	bool matchPosition();
	bool calculateDetectHeight();
	bool waitCheckDone();

	bool isExit();
private:
	bool captureAllImages(QVector<cv::Mat>& imageMats);
	void setResoultLight(bool isOk);
	void resetResoultLight();

	bool getLightIO(int &okLight, int &ngLight);

	QString generateImagePath();
	void saveImages(QString& szImagePath, int nIndex, QVector<cv::Mat>& imageMats);

private:
	void addImageText(cv::Mat image, Point ptPos, QString szText);

private:
	QCheckerParamMap *m_paramMap;
	QCheckerParamDataList *m_paramData;
	QCheckerPositionMap m_positionMap;

	bool m_exit;
	QLineNormal m_normal;
	QMutex m_mutex;
	
	QVector<cv::Mat> m_3DMatHeights;
	cv::Mat m_3DMatHeight;

	QVector<QImageStruct> m_matImages;
	QImageStruct m_matImage;
	int m_nImageIndex;
};
	

//-------------------------------------------------------------
class QFlowCtrl : public QObject
{
	typedef QList<QCheckerParamMap*> QCheckerParamMapList;
	typedef QList<QCheckerRunable*> QCheckerRunableList;
	Q_OBJECT
public:
	QFlowCtrl(QObject *parent);
	~QFlowCtrl();

	/*void setEnableCheckStation(int *enableState);
	void setEnableCheckStation(int n,int state);
	void getEnableCheckStation(int *enableState);
	bool isEnableCheckStation(int iStation);*/
	bool isRuning();

protected slots:
	void onImageEvent(const QVariantList &data);

	void home();
	void startAutoRun();
	void stopAutoRun();

protected:
	virtual void timerEvent(QTimerEvent * event);

	void checkImStop();
	void checkReset();
	void checkStart();
	void checkStop();
	void readbarCode();
	void checkMotionState();
	void checkError();
	/*void checkAI();
	void checkSafeDoor();*/
	void checkPowError();
	void checkAuthError();

	void initStationParam();
	void initErrorCode();
protected:
	void imStop();
	void imStopStation(int nStation);
	void reset();
	void start();
	void stop();


private:
	int m_timerId;
	bool m_isHome;
	bool m_homeIng;
	bool m_isStart;
	QCheckerParamMapList m_stationParams;
	QCheckerParamDataList m_stationDatas;
	QCheckerRunableList m_checkStantion;
	QCheckerParamMap *m_pMainParamMap;
	QMainRunable* m_pMainStation;
	QLineNormal m_normal;

	QDateTime m_dateTime;
	int m_errorCode;
};

#endif // FLOWCTRL_H
