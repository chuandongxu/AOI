#ifndef LIGHTDEVICE_H
#define LIGHTDEVICE_H

#include <QObject>

#include "lightmodule_global.h"
#include "../include/ILight.h"

struct CHData
{
	bool bOpen;
	int iLuminance;
	QString named;

	CHData()
        :bOpen(false), iLuminance(0), named("")
	{
	}
};

class QCommPort;
class QLightDevice : public QObject
{
	Q_OBJECT
public:
	QLightDevice(const QString & devName, int nChnNum, QObject *parent);
	~QLightDevice();

	QString getDeviceName(){return m_devName;};
    int getChnNum(){ return m_nChnNum; }

	static QStringList enumCommPort();
	static QStringList enumCommBound();

	void openCommPort(const QString & name,int bound);
	void closeCommPort();
	bool isOpenCommPort(){ return (m_comPort? true : false); };

	void openLight(int ch);
	void closeLight(int ch);
	bool isOpenLight(int ch);

	virtual void setChLuminance(int ch,int luminance) = 0;
	void setChName(int ch,const QString & name);

    virtual void setupTrigger(ILight::TRIGGER emTrig) = 0;
    virtual bool trigger() = 0;

	int getChLuminance(int ch);
	QString getChName(int ch);

protected:
    void open();
	int mapPortName(const QString &str);

private:
	QString m_devName;
	bool m_bOpened;  
    int m_nChnNum;

protected:
    QCommPort * m_comPort;
    CHData m_data[_MAX_CHDATA_NUM];
};

typedef QList<QLightDevice*> QLightDeviceList;

class QLightDerivedDevice : public QLightDevice
{  
    Q_OBJECT
public:
    QLightDerivedDevice(const QString & devName, int nChnNum, QObject *parent);
    ~QLightDerivedDevice();

    virtual void setChLuminance(int ch, int luminance) override;

    virtual void setupTrigger(ILight::TRIGGER emTrig) override{}

    virtual bool trigger() override{return true; }
};

#endif // LIGHTDEVICE_H
