#ifndef BARCODEDEVICE_H
#define BARCODEDEVICE_H

#pragma exec
#include <QObject>
#include <qserialport.h>
#include <QThread>
#include "CommPort.h"

class QBarCodeDevice;
class QReadThread : public  QThread
{
public:
	QReadThread(QBarCodeDevice *dev);

	void run();
	void quit(){m_bQuit = true;};

	QString readBarcode();

protected:
	QBarCodeDevice * m_dev;
	QString m_barCode;
	bool m_bQuit;
	QMutex m_mutex;
	int m_barcodeNum;
};


class QBarCodeDevice : public QObject
{
	Q_OBJECT
    friend class QReadThread;
public:
	QBarCodeDevice(const QString &name,QObject *parent = NULL);
	~QBarCodeDevice();

	void open();
	void close();

	void setName(const QString &name);
	void setCommPort(const QString &port);
	void setBoundRate(int boundRate);

	QString getDevName();
	QString getCommPort();
	int getBoundRate();
	bool isOpen();

	QString readBarCode();

protected slots:
		void onRecvData();

private:
	void loadParam();

private:
	QReadThread * m_readThread;
	//QSerialPort * m_comPort;
	QCommPort * m_comPort;
	QString m_name;
	QString m_port;
	int m_boundRate;
	bool m_isOpen;
	int m_timer;
};

#endif // BARCODEDEVICE_H
