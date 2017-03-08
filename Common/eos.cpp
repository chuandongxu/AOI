#include "eos.h"

QMap<QString,QDispth*> QEos::m_dispthes;
QMutex QEos::m_mutex;

QEos::QEos(QObject *parent)
	: QObject(parent)
{

}

QEos::~QEos()
{

}

void QEos::Attach(const QString &notifyKey,const QObject * receiver, const char * method)
{
	m_mutex.lock();
	if(m_dispthes.contains(notifyKey))
	{
		connect(m_dispthes[notifyKey],SIGNAL(onNotify(const QVariantList &)),receiver,method);
	}
	else
	{
		QDispth * p = new QDispth;
		connect(p,SIGNAL(onNotify(const QVariantList &)),receiver,method);
		m_dispthes[notifyKey] = p;
	}
	m_mutex.unlock();
}

void QEos::Detach(const QString &notifyKey,const QObject * receiver, const char * method)
{
	m_mutex.lock();
	if(m_dispthes.contains(notifyKey))
	{
		disconnect(m_dispthes[notifyKey],SIGNAL(onNotify(const QVariantList &)),receiver,method);
	}
	m_mutex.unlock();
}

void QEos::Notify(const QString notifyKey,const QVariantList &data)
{
	m_mutex.lock();
	if(m_dispthes.contains(notifyKey))
	{
		emit m_dispthes[notifyKey]->onNotify(data);
	}
	m_mutex.unlock();
}

void QEos::Notify(const QString notifyKey,const QVariant &d1)
{
	QVariantList vl;
	vl << d1;
	Notify(notifyKey,vl);
}

void QEos::Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2)
{
	QVariantList vl;
	vl << d1 << d2;
	Notify(notifyKey,vl);
}

void QEos::Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3)
{
	QVariantList vl;
	vl << d1 << d2 << d3;
	Notify(notifyKey,vl);
}

void QEos::Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4)
{
	QVariantList vl;
	vl << d1 << d2 << d3 << d4;
	Notify(notifyKey,vl);
}

void QEos::Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4,const QVariant &d5)
{
	QVariantList vl;
	vl << d1 << d2 << d3 << d4 << d5;
	Notify(notifyKey,vl);
}

void QEos::Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4,const QVariant &d5,
		               const QVariant &d6)
{
	QVariantList vl;
	vl << d1 << d2 << d3 << d4 << d5 << d6;
	Notify(notifyKey,vl);
}
	
void QEos::Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4,const QVariant &d5,
		               const QVariant &d6,const QVariant &d7)
{
	QVariantList vl;
	vl << d1 << d2 << d3 << d4 << d5 << d6 << d7;
	Notify(notifyKey,vl);
}

void QEos::Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4,const QVariant &d5,
		               const QVariant &d6,const QVariant &d7,const QVariant &d8)
{
	QVariantList vl;
	vl << d1 << d2 << d3 << d4 << d5 << d6 << d7 << d8;
	Notify(notifyKey,vl);
}

void QEos::Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4,const QVariant &d5,
				  const QVariant &d6,const QVariant &d7,const QVariant &d8,const QVariant &d9)
{
	QVariantList vl;
	vl << d1 << d2 << d3 << d4 << d5 << d6 << d7 << d8<<d9;
	Notify(notifyKey,vl);
}