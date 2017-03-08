#ifndef COMMPORT_H
#define COMMPORT_H

#include <QObject>
#include <Windows.h>

class QCommPort : public QObject
{
	Q_OBJECT

public:
	QCommPort(QObject *parent = NULL);
	~QCommPort();

	bool open(const QString &port,int bound);
	void close();

	bool read(QByteArray &ar);
	bool write(const QByteArray &ar);

private:
	HANDLE m_hCom;
};

#endif // COMMPORT_H
