#ifndef LIGHTCTRL_H
#define LIGHTCTRL_H

#include <QObject>

#include "../include/ILight.h"

#include "lightdevice.h"
#include "qlightcarddevice.h"

class QLightCtrl : public QObject
{
	Q_OBJECT

public:
	QLightCtrl(QObject *parent);
	~QLightCtrl();

    void init();
	int getDeviceCount(){return m_deviceList.size();};
	void addDevice(QLightDevice *pDevice){m_deviceList.append(pDevice);};
	void delDevice(int nIndex);
    QLightDevice * getLightDevice(int nIndex);


    void setLuminance(int nChannel, int nLum);
    QString getChName(int nChannel);
    int getChLuminace(int nChannel);
    void saveLuminance(int nChannel);

    bool triggerCapturing(ILight::TRIGGER emTrig, bool bWaitDone, bool bClearSetupConfig);

private:
    void setupTrigger(ILight::TRIGGER emTrig);

private:
	QLightDeviceList m_deviceList;
    bool m_bSetupTriggerConfig;
};

#endif // LIGHTCTRL_H
