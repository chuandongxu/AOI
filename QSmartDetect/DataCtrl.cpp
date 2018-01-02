#include "DataCtrl.h"
#include "../Common/ThreadPrioc.h"
#include <QApplication>
#include "../Common/SystemData.h"
#include "qdebug.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

DataCtrl::DataCtrl(QObject *parent)
	: QObject(parent)
{
	m_nCycleTestNum = 0;

	m_bInfiniteCycles = false;

	m_nRunMode = 0;
	m_nRunType = 0;

	m_isStart = false;

	m_pWidget = new QRunTabWidget();
}

DataCtrl::~DataCtrl()
{	
	delete m_pWidget;
}

void DataCtrl::setInfiniteCycles(bool bInfinite)
{
	m_bInfiniteCycles = bInfinite;
}

void DataCtrl::incrementCycleTests()
{
	QAutoLocker loacker(&m_mutex);
	m_nCycleTestNum += 1;
}

void DataCtrl::decrementCycleTests()
{
	QAutoLocker loacker(&m_mutex);
	if (m_nCycleTestNum > 0)
	{
		m_nCycleTestNum -= 1;
	}
	else
	{
		m_nCycleTestNum = 0;
	}
}

int	DataCtrl::getCycleTests()
{
	QAutoLocker loacker(&m_mutex);

	if (m_bInfiniteCycles) return 1;

	return m_nCycleTestNum;
}

void DataCtrl::setRunMode(int nRunMode)
{
	m_nRunMode = nRunMode;
}

bool DataCtrl::isRunOnce()
{
	return 0 == m_nRunMode;
}


void DataCtrl::setRunType(int nRunType)
{
	m_nRunType = nRunType;
}

bool DataCtrl::isRunByImages()
{
	return 2 == m_nRunType;
}