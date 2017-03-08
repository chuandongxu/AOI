#include "BarCodeDevMgr.h"
#include "../Common/SystemData.h"

QBarCodeDevMgr::QBarCodeDevMgr(QObject *parent)
	: QObject(parent)
{
	this->loadDevice();
}

QBarCodeDevMgr::~QBarCodeDevMgr()
{

}

void QBarCodeDevMgr::loadDevice()
{
	QStringList ls = System->getSysParamKeys("barDev-name");
	for(int i=0; i<ls.size(); i++)
	{
		QString name = System->getSysParam(ls[i]).toString();
		m_devList.append(new QBarCodeDevice(name));
	}
}

QBarCodeDevice * QBarCodeDevMgr::getDevice(int n)
{
	if(n >-1 && n<m_devList.size())
	{
		return m_devList[n];
	}

	return NULL;
}

QBarCodeDevice * QBarCodeDevMgr::addDevice(const QString &name)
{
	int n = m_devList.size();
	QBarCodeDevice * p = new QBarCodeDevice(name);
	m_devList.append(p);

	return p;
}

void QBarCodeDevMgr::removeDevice(int n)
{
	m_devList.removeAt(n);
}

int QBarCodeDevMgr::getDevcount()
{
	return m_devList.size();
}