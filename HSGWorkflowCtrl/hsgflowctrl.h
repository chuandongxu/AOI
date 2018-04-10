#ifndef FLOWCTRL_H
#define FLOWCTRL_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QRunnable>
#include <QStringList>
#include <QElapsedTimer>
#include <QDateTime>

#include "hsgworkflowctrl_global.h"
#include "../include/ICAN.h"

#include "AutoRunThread.h"
#include "opencv/cv.h"

class QFlowCtrl : public QObject
{
	typedef QList<QCheckerParamMap*> QCheckerParamMapList;
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
    void autoThreadFinish();

protected:
	virtual void timerEvent(QTimerEvent * event);

	void checkImStop();
	void checkReset();
	void checkStart();
	void checkStop();
	void readbarCode();
	void checkMotionState();
	void checkError();
	void checkAuthError();

	void initStationParam();
	void initErrorCode();

	void imStop();
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
	QCheckerParamMap *m_pMainParamMap;
	AutoRunThread *m_pAutoRunThread;

	QDateTime m_dateTime;
	int m_errorCode;
};

#endif // FLOWCTRL_H
