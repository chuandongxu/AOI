#pragma once

#include <QObject>
#include <qmutex.h>
#include <QVector>
#include "QRunTabWidget.h"

class DataCtrl : public QObject
{
	Q_OBJECT

public:
	DataCtrl(QObject *parent = NULL);
	~DataCtrl();

public:
	void setInfiniteCycles(bool bInfinite);
	void incrementCycleTests();
	void decrementCycleTests();
	int	getCycleTests();

	void setRunMode(int nRunMode);
	bool isRunOnce();

	void setRunType(int nRunType);
	bool isRunByImages();

	QRunTabWidget* getWidget() { return m_pWidget; }

	bool isRunning() { return m_isStart; }
	void run(bool bStart) { m_isStart = bStart; }

private:
	QMutex m_mutex;
	int m_nCycleTestNum;
	bool m_bInfiniteCycles;
	int m_nRunMode;
	int m_nRunType;

	QRunTabWidget* m_pWidget;

	bool m_isStart;
};
