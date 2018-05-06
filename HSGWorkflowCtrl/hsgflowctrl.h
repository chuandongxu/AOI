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
#include "BoardInspResult.h"

class QFlowCtrl : public QObject
{
	Q_OBJECT
public:
	QFlowCtrl(QObject *parent);
	~QFlowCtrl();

	bool isRuning();

protected slots:
	void onImageEvent(const QVariantList &data);

	void home();
	void startAutoRun();
	void stopAutoRun();
    void onError(const QString &strMsg);

protected:
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
	int                 m_timerId;
	bool                m_isHome;
	bool                m_homeIng;
	bool                m_isStart;

	AutoRunThread      *m_pAutoRunThread;

	QDateTime           m_dateTime;
	int                 m_errorCode;
    MapBoardInspResult  m_mapBoardInspResult;
};

#endif // FLOWCTRL_H
