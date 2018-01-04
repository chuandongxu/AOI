#include "lightdevice.h"
#include "../Common/SystemData.h"
#include "LightDefine.h"
#include <qvariant>
#include <QtSerialPort\\QSerialPortInfo>
#include <qdebug.h>
#include <qthread.h>
#include <qmutex.h>

#include "CommPort.h"


QLightDevice::QLightDevice(const QString & devName,QObject *parent)
	: m_devName(devName),QObject(parent),m_bOpened(false)
{
	m_comPort = NULL;

	this->open();
	//qDebug() << "thread : " << (unsigned int)this;
	
	//connect(this,SIGNAL(emSendData(int,int)),SLOT(sendData(int,int)),Qt::QueuedConnection);
}

QLightDevice::~QLightDevice()
{
	this->closeCommPort();
}

QStringList QLightDevice::enumCommPort()
{
	static QStringList strs;
	
	if(strs.isEmpty())
	{
		Q_FOREACH(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
		{
			strs << info.portName();
		}
	}

	return strs;
}

QStringList QLightDevice::enumCommBound()
{
	static QStringList ls;

	if(ls.isEmpty())
	{
		Q_FOREACH(qint32 bound,QSerialPortInfo::standardBaudRates())
		{
			ls << QString::number(bound);
		}
	}
	return ls;
}

void QLightDevice::open()
{
	QString key = QString("%0-%1").arg(m_devName).arg(COMM_PORT);
	QString prot = System->getParam(key).toString();
	key = QString("%0-%1").arg(m_devName).arg(COMM_BOUND);
	int bound = System->getParam(key).toInt();

	this->openCommPort(prot,bound);
}

void QLightDevice::openCommPort(const QString & name,int bound)
{
	int nPort = mapPortName(name);
	m_comPort = new QCommPort();

	if (!m_comPort->open(name, bound))
	{
		delete m_comPort;
		m_comPort = NULL;
		System->setTrackInfo("open com Device Fail");

		return;
	}
}
	
void QLightDevice::closeCommPort()
{
	if (m_comPort)
	{
		m_comPort->close();	
		delete m_comPort;
		m_comPort = NULL;	
	}
}


void QLightDevice::openLight(int ch)
{
	m_data[ch].bOpen = true;
}
	
void QLightDevice::closeLight(int ch)
{
	m_data[ch].bOpen = false;
}

bool QLightDevice::isOpenLight(int ch)
{
	return m_data[ch].bOpen;
}

void QLightDevice::setChLuminance(int ch,int luminance)
{
	if (m_comPort)
	{
		QString strValue = QString("%1").arg(luminance, 4, 10, QLatin1Char('0'));

		QString szCh = "";
		switch (ch)
		{
		case 0:
			szCh = "A";
			break;
		case 1:
			szCh = "B";
			break;
		case 2:
			szCh = "C";
			break;
		case 3:
			szCh = "D";
			break;
		default:
			break;
		}

		QString szCmd = "SI" + strValue + szCh + "#";
		m_comPort->write(szCmd.toLocal8Bit());

		QString szRetValue = "I" + szCh;

		int nWaitTime = 1 * 100;
		while (nWaitTime-- > 0 )
		{
			QByteArray readLine;
			if (m_comPort->read(readLine))
			{
				QString value = readLine;
				if (!value.trimmed().isEmpty())
				{
					if (value != szRetValue)
					{
						System->setTrackInfo("set light setChLuminance fail, ret=" + value);
					}
					break;
				}			
			}

			QThread::msleep(10);
		}

		if (nWaitTime <= 0)
		{
			System->setTrackInfo("set light setChLuminance fail. Wait Timeout!");
		}		
	}
}

	
void QLightDevice::setChName(int ch,const QString & name)
{
	m_data[ch].named = name;
}

int QLightDevice::getChLuminance(int ch)
{
	return m_data[ch].iLuminance;
}
	
QString QLightDevice::getChName(int ch)
{
	return m_data[ch].named;
}

int QLightDevice::mapPortName(const QString &str)
{
	QString port  = str.toLower();
	if("com1" == port)return 1;
	else if("com2" == port)return 2;
	else if("com3" == port)return 3;
	else if("com4" == port)return 4;
	else if("com5" == port)return 5;
	else if("com6" == port)return 6;
	else if("com7" == port)return 7;
	else if("com8" == port)return 8;
	else if("com9" == port)return 9;
	else if("com10" == port)return 10;
	else if("com11" == port)return 11;
	else if("com12" == port)return 12;
	else if("com13" == port)return 13;
	else if("com14" == port)return 14;
	else if("com15" == port)return 15;
	else if("com16" == port)return 16;
	else if("com17" == port)return 17;
	else if("com18" == port)return 18;

	return 0;
}