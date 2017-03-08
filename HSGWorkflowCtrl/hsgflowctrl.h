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


//---------------------------------------------------------------
typedef QMap<QString,QVariant> QCheckerParamMap;


//-------------------------------------------------------------
class QCheckerRunable : public QRunnable
{
public:
	QCheckerRunable( QCheckerParamMap *paramMap);
	~QCheckerRunable();

	void quit();
	void imgStop();//stop steppers
protected:
	void run();

	//bool waitStartBtn();
	//bool readBarcode();
	//bool moveToPressPos();
	//bool addPress();
	//bool addSidePress();
	//bool stopSidePress();
	//bool startCheck(int nDevice = -1);
	//bool waitCheckDone();
	//bool releaseSidePress();
	//bool releasePress();
	//bool moveToBack();

	bool isExit();
private:
	void setResoultLight(bool isOk);
	void resetResoultLight();

	bool getLightIO(int &okLight, int &ngLight);

private:
	QCheckerParamMap *m_paramMap;
	bool m_exit;
	QLineNormal m_normal;
	QString m_barCode;
	QMutex m_mutex;
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
	void home();

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
	QCheckerRunableList m_checkStantion;
	QStringList m_barCode;
	QLineNormal m_normal;

	int m_errorCode;
};

#endif // FLOWCTRL_H
