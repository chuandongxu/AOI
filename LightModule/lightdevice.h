#ifndef LIGHTDEVICE_H
#define LIGHTDEVICE_H

#include <QObject>
#include "LightCtrl32.h"

struct CHData
{
	bool bOpen;
	int iLuminance;
	QString named;

	CHData()
		:bOpen(false),iLuminance(0)
	{
	}
};

class QLightDevice : public QObject
{
	Q_OBJECT
public:
	QLightDevice(const QString & devName,QObject *parent);
	~QLightDevice();

	QString getDeviceName(){return m_devName;};

	static QStringList enumCommPort();
	static QStringList enumCommBound();

	void openCommPort(const QString & name,int bound);
	void closeCommPort();
	bool isOpenCommPort(){return (m_light?true:false);};

	void openLight(int ch);
	void closeLight(int ch);
	bool isOpenLight(int ch);

	void setChLuminance(int ch,int luminance);
	void setChName(int ch,const QString & name);

	int getChLuminance(int ch);
	QString getChName(int ch);

public:
	//void sendTestData(const QString &hexString);
	//void recvHexData(QString &data);

	//void sendData(const QByteArray &data);
	//void recvData(QByteArray &data);
	//void waitRecvData(QByteArray &data,int nTime);

//signals:
	//void dataReady();
	//void emSendData(int ch,int lum);

protected:
    void open();
	//void initChannel(int n);
	//QByteArray HexStringToData(const QString &str);
	//QString DataToHexString(const QByteArray &data);

	//QByteArray formatData();
	int mapPortName(const QString &str);

//protected slots:
	//void sendData(int ch,int lum);

private:
	HLIGHT m_light;
	QString m_devName;
	bool m_bOpened;
	CHData m_data[4];
};

typedef QList<QLightDevice*> QLightDeviceList;

#endif // LIGHTDEVICE_H
