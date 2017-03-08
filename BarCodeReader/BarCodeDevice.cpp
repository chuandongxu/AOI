#include "BarCodeDevice.h"
#include "../Common/SystemData.h"
#include "QDebug.h"
#include <QSerialPortInfo>

QList<QSerialPortInfo>	ls;

#define barcodeNum 19
#define maxLengthBarcode 22

QReadThread::QReadThread(QBarCodeDevice *dev)
	:m_dev(dev),m_bQuit(false)
{

	m_barCode= "";

	if(System->getParam(QStringLiteral("barcodecounts")).isNull())  m_barcodeNum = 19;
	else
	{
		m_barcodeNum  = System->getParam(QStringLiteral("barcodecounts")).toInt();
	}
}

void QReadThread::run()
{
	if(!m_dev)return;

	while(!m_bQuit)
	{
		if(m_dev->m_comPort)
		{
			QByteArray ar;
			if(m_dev->m_comPort->read(ar))
			{
				QList<QByteArray> barcode_1 = ar.split('\r');
				for(int i = 0;i<barcode_1.size();i++)
				{
					QList<QByteArray> barcode_2 = barcode_1[i].split('\n');
					{
						for(int i =0;i<barcode_2.size();i++)
						{
							QList<QByteArray> barcode_3 = barcode_2[i].split('\x2');
							for(int i=0;i<barcode_3.size();i++)
							{
								//qDebug()<<barcode_3[i];
								if(barcode_3[i].size() == m_barcodeNum & !barcode_3[i].contains('\r') & !barcode_3[i].contains('\n'))
								{
									//qDebug()<<barcode_3[i];
									m_mutex.lock();
									m_barCode = barcode_3[i];
									m_mutex.unlock();
								}
							}
						}

					}
				}

			}
			//else qDebug() << "no comm data.";
		}

		QThread::msleep(100);
	}
}

QString QReadThread::readBarcode()
{
	QString str;
	m_mutex.lock();
	if(!m_barCode.isNull())
	{
		
		int barcodeindex = 0;
		if(m_barCode.size()>maxLengthBarcode)
		{
			for(int i = 0 ; i < m_barCode.size() ;i++)
			{
				if(i > 0 && i%maxLengthBarcode == 0 )
					str =str+"\n";
				str = str+m_barCode.at(i);
			}
		}
		else
		{
			str = m_barCode;
		}
		
	}
	m_mutex.unlock();
	m_barCode.clear();
	return str;
}


QBarCodeDevice::QBarCodeDevice(const QString &name,QObject *parent)
	: QObject(parent),m_comPort(NULL),m_name(name),m_boundRate(19200),m_isOpen(false),m_timer(0)
{
	this->loadParam();
	this->open();
}

QBarCodeDevice::~QBarCodeDevice()
{
	this->close();
}

void QBarCodeDevice::open()
{
	//ls = QSerialPortInfo::availablePorts();
	//m_comPort = new QSerialPort(ls[0]);
	//m_comPort->setPortName(m_port);
	//m_comPort->setBaudRate(9600);
	//m_comPort->setDataBits(QSerialPort::Data8);
	//m_comPort->setStopBits(QSerialPort::OneStop);
	//m_comPort->setParity(QSerialPort::NoParity);
	////m_comPort.setFlowControl( QSerialPort::FlowControl::NoFlowControl);
	//if(m_comPort->open(QIODevice::ReadWrite))
	//{
	//	m_comPort->setBaudRate(9600);
	//	m_comPort->setDataBits(QSerialPort::Data8);
	//	m_comPort->setStopBits(QSerialPort::OneStop);
	//	m_comPort->setParity(QSerialPort::NoParity);
	//	m_isOpen = true;
	//}
	qDebug() << m_name << "  " << m_port << "   " << m_boundRate;
	m_comPort = new QCommPort;
	if(!m_comPort->open(m_port,	m_boundRate))
	{
		delete m_comPort;
		m_comPort = NULL;
		System->setTrackInfo("open barcode Device Fail");

		return;
	}

	m_readThread = new QReadThread(this);
	m_readThread->start();
	m_isOpen = true;
}

void QBarCodeDevice::close()
{
	if(m_comPort)
	{
		m_comPort->close();

		m_readThread->quit();
		QThread::msleep(200);
		delete m_readThread;
		delete m_comPort;
		m_comPort = NULL;
		m_readThread = NULL;
		m_isOpen = false;
	}
}

void QBarCodeDevice::setName(const QString &name)
{
	if(m_name != name)
	{
		System->delSysParam(QString("barDev-name-%0").arg(m_name));
		System->delSysParam(QString("barDev-port-%0").arg(m_name));
		System->delSysParam(QString("barDev-boundRate-%0").arg(m_name));
		m_name = name;
	}
}

void QBarCodeDevice::setCommPort(const QString &port)
{
	if(m_port != port)
	{
		System->delSysParam(QString("barDev-port-%0").arg(m_name));
		m_port = port;
	}
}
	
void QBarCodeDevice::setBoundRate(int boundRate)
{
	if(m_boundRate != boundRate)
	{
		System->delSysParam(QString("barDev-boundRate-%0").arg(m_name));
		m_boundRate = boundRate;
	}
}

QString QBarCodeDevice::getDevName()
{
	return m_name;
}

QString QBarCodeDevice::getCommPort()
{
	return m_port;
}

int QBarCodeDevice::getBoundRate()
{
	return m_boundRate;
}
	
bool QBarCodeDevice::isOpen()
{
	return m_isOpen;
}

void QBarCodeDevice::loadParam()
{
	QString key = QString("barDev-port-%0").arg(m_name);
	m_port = System->getSysParam(key).toString();
	if(m_port.isEmpty())m_port = "COM0";

	key = QString("barDev-boundRate-%0").arg(m_name);
	m_boundRate = System->getSysParam(key).toInt();
	if(0 == m_boundRate)m_boundRate = 9600;
}

QString QBarCodeDevice::readBarCode()	
{
	if(m_readThread)
	{
		return m_readThread->readBarcode();
	}

	return "";
}

void QBarCodeDevice::onRecvData()
{
	//QByteArray arr = m_comPort->readAll();
	//m_barcode = arr.data();
}
