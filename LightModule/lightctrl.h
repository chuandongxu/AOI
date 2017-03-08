#ifndef LIGHTCTRL_H
#define LIGHTCTRL_H

#include <QObject>
#include "lightdevice.h"

class QLightCtrl : public QObject
{
	Q_OBJECT

public:
	QLightCtrl(QObject *parent);
	~QLightCtrl();

	int getDeviceCount(){return m_deviceList.size();};
	void addDevice(QLightDevice *pDevice){m_deviceList.append(pDevice);};
	void delDevice(int n);
	void init();
	QLightDevice * getLightDevice(int n);

private:
	QLightDeviceList m_deviceList;
};

#endif // LIGHTCTRL_H
