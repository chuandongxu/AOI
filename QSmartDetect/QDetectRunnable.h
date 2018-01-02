#pragma once

#include <QObject>
#include <qmap.h>
#include <qvector.h>
#include <qrunnable.h>
#include <qstringlist.h>
#include <QDateTime>
#include "DataCtrl.h"
#include "opencv/cv.h"

using namespace cv;

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

//-------------------------------------------------------------
class QDetectObj;
class QProfileObj;
class QCheckerRunable : public QRunnable
{
public:
	QCheckerRunable(QCheckerParamMap *paramMap, Q3DStructData* dataParam);
	~QCheckerRunable();

	void quit();
	void imgStop();//stop steppers
	bool isRunning();

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
	bool m_bRunning;
	QMutex m_mutex;

	QVector<cv::Mat> m_imageMats;
	cv::Mat m_3DMatHeight;
	cv::Mat m_matHeightResultImg;
	cv::Mat m_matImage;
};

class QMainRunable : public QRunnable
{
public:
	QMainRunable(QCheckerParamMap *paramMap, QCheckerParamDataList* paramData, DataCtrl* pCtrl);
	~QMainRunable();

	void quit();
	void imgStop();//stop steppers
protected:
	void run();

	bool waitStartBtn();
	bool captureImages();
	bool matchPosition();
	bool calculateDetectHeight();
	bool calculateDetectProfile();
	bool waitCheckDone();

	bool isExit();

private:
	bool captureAllImages(QVector<cv::Mat>& imageMats);
	void setResoultLight(bool isOk);
	void resetResoultLight();

	bool getLightIO(int &okLight, int &ngLight);

private:
	void clearTestObjs();
	void clearTestProfObjs();
	void displayAllObjs();
	void displayAllProfObjs();
	void addImageText(cv::Mat image, Point ptPos, QString szText);

private:
	QCheckerParamMap *m_paramMap;
	QCheckerParamDataList *m_paramData;
	bool m_exit;
	QMutex m_mutex;

	cv::Mat m_3DMatHeight;
	cv::Mat m_matImage;

	QVector<QDetectObj*> m_objTests;

	QVector<QProfileObj*> m_objProfTests;	

	DataCtrl* m_pCtrl;
};


class QDetectRunnable : public QObject
{
	Q_OBJECT

public:
	QDetectRunnable(QObject *parent);
	~QDetectRunnable();
};
