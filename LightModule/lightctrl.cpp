#include "lightctrl.h"
#include "../Common/SystemData.h"
#include "LightDefine.h"
#include <qvariant.h>

QLightCtrl::QLightCtrl(QObject *parent)
	: QObject(parent)
{
	int n = System->getParam(LIGHT_COUNT).toInt();
	for(int i = 0; i<n; i++)
	{
		QString name = QString("light%0").arg(i+1);
		m_deviceList.append(new QLightDevice(name,NULL));
	}
	
}
void QLightCtrl::init()
{
	int n = System->getParam(LIGHT_COUNT).toInt();
	for (int i = 0; i<n; i++)
	{
		QString key = QString("%0-%1").arg(m_deviceList[i]->getDeviceName()).arg(COMM_PORT);
		QString port = System->getParam(key).toString();
		key = QString("%0-%1").arg(m_deviceList[i]->getDeviceName()).arg(COMM_BOUND);
		int baud = System->getParam(key).toInt();
		m_deviceList[i]->openCommPort(port,baud);
		for (int j = 0; j<4;j++)
		{
			m_deviceList[i]->openLight(j);
		}
		//m_deviceList[i]->openCommPort(->getDeviceName(),m_deviceList[i]->ge);
	}
}
QLightCtrl::~QLightCtrl()
{

}

void QLightCtrl::delDevice(int n)
{
	QLightDevice * device = m_deviceList[n];
	if(device)delete device;
	m_deviceList.removeAt(n);
}
QLightDevice * QLightCtrl::getLightDevice(int n)
{
	if(n < m_deviceList.size())
	{
		return m_deviceList[n];
	}
	return NULL;
}