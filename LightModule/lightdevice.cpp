#include "lightdevice.h"
#include "../Common/SystemData.h"
#include "LightDefine.h"
#include <qvariant>
#include <QtSerialPort\\QSerialPortInfo>
#include <qdebug.h>
#include <qthread.h>
#include <qmutex.h>


QLightDevice::QLightDevice(const QString & devName,QObject *parent)
	: m_devName(devName),QObject(parent),m_bOpened(false)
{
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
	m_light = openLigh(nPort,bound);
}
	
void QLightDevice::closeCommPort()
{
	close(m_light);
	m_light = NULL;
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
	setLightValue(m_light,ch,luminance);
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