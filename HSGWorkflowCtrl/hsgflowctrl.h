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
#include "AutoRunDataStructs.h"

class QFlowCtrl : public QObject
{
	Q_OBJECT
public:
	QFlowCtrl(QObject *parent);
	~QFlowCtrl();

	bool isRuning();

protected slots:
	void onImageEvent(const QVariantList &data);
    void onThreadState(const QVariantList &data);

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
    int _prepareRunData();
    void _refreshDisplayImage();

private:
	int                             m_timerId;
	bool                            m_isHome;
	bool                            m_homeIng;
	bool                            m_isStart;

	AutoRunThread                  *m_pAutoRunThread;

	QDateTime                       m_dateTime;
	int                             m_errorCode;
    MapBoardInspResult              m_mapBoardInspResult;
    Engine::AlignmentVector         m_vecAlignments;
    float                           m_fBoardLeft = 0, m_fBoardTop = 0, m_fBoardRight = 0, m_fBoardBottom = 0;
    int                             m_nImgWidth = 0, m_nImgHeight = 0;
    float                           m_fOverlapX = 0, m_fOverlapY = 0;
    Vision::VectorOfVectorOfPoint2f m_vecVecFrameCtr;
    DeviceInspWindowVector          m_vecDeviceInspWindow;
};

#endif // FLOWCTRL_H
