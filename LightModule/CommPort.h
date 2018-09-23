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
    bool readSyn(QByteArray &ar);
    bool write(const QByteArray &ar);

private:
    HANDLE m_hCom;
    OVERLAPPED m_ovWait;//用于等待数据
    bool m_bOpen;
};

#endif // COMMPORT_H
