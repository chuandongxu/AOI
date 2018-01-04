#ifndef LIGHTDEVICE_H
#define LIGHTDEVICE_H

#include <QObject>

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

class QCommPort;
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
	bool isOpenCommPort(){ return (m_comPort? true : false); };

	void openLight(int ch);
	void closeLight(int ch);
	bool isOpenLight(int ch);

	void setChLuminance(int ch,int luminance);
	void setChName(int ch,const QString & name);

	int getChLuminance(int ch);
	QString getChName(int ch);

protected:
    void open();
	int mapPortName(const QString &str);

private:
	QCommPort * m_comPort;

	QString m_devName;
	bool m_bOpened;
	CHData m_data[4];
};

typedef QList<QLightDevice*> QLightDeviceList;

#endif // LIGHTDEVICE_H
