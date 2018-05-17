#include "CommPort.h"
#include <qdebug.h>
#include <qthread.h>

QCommPort::QCommPort(QObject *parent)
	: QObject(parent),m_hCom(INVALID_HANDLE_VALUE)
{
    memset(&m_ovWait, 0, sizeof(m_ovWait));

    m_bOpen = false;
}

QCommPort::~QCommPort()
{

}

bool QCommPort::open(const QString &port,int BaudRate)
{
	m_hCom = CreateFile(port.toStdWString().c_str(),
		GENERIC_READ|GENERIC_WRITE,                //允许读和写
		FILE_SHARE_READ,                           //独占方式
		NULL,
		OPEN_EXISTING,                             //打开而不是创建
		FILE_ATTRIBUTE_READONLY,                     //同步方式
		NULL);
	if (INVALID_HANDLE_VALUE == m_hCom)
	{
		qDebug() << "barcode OpenDev Error!";
		return false;
	}

	SetupComm(m_hCom, 1024, 1024); //输入缓冲区和输出缓冲区的大小都是

	DCB dcb;
	memset(&dcb,0,sizeof(DCB));
	if(!GetCommState(m_hCom, &dcb))
	{
		qDebug() << "GetCommState Error";
		return false;
	}

	dcb.BaudRate= BaudRate;       //波特率为9600
	dcb.ByteSize=8;              //每个字节有8位
	dcb.Parity=NOPARITY;         //无奇偶校验位
	dcb.StopBits=ONESTOPBIT;              //1个停止位
	if(!SetCommState(m_hCom, &dcb))
	{
		qDebug() << "SetCommState Error";
		return false;
	}

	//清除收发缓存区
    PurgeComm(m_hCom, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);//清空缓存

	COMMTIMEOUTS timeOutis;
	GetCommTimeouts(m_hCom,&timeOutis);
	timeOutis.ReadIntervalTimeout = MAXDWORD;
	timeOutis.ReadTotalTimeoutConstant = 0;
	timeOutis.ReadTotalTimeoutMultiplier = 0;
	timeOutis.WriteTotalTimeoutConstant = 0;
	timeOutis.WriteTotalTimeoutMultiplier = 0;
	SetCommTimeouts(m_hCom,&timeOutis);

    m_ovWait.hEvent = CreateEvent(NULL, false, false, NULL);
    SetCommMask(m_hCom, EV_ERR | EV_RXCHAR);//设置接受事件

	DWORD dwErrorFlags;
	COMSTAT ComStat;
	ClearCommError(m_hCom, &dwErrorFlags, &ComStat);

    m_bOpen = true;

	return true;
}

void QCommPort::close()
{
	if(INVALID_HANDLE_VALUE == m_hCom)return;

	CloseHandle(m_hCom);
	m_hCom = INVALID_HANDLE_VALUE;

    if (NULL != m_ovWait.hEvent)
    {
        CloseHandle(m_ovWait.hEvent);
        m_ovWait.hEvent = NULL;
    }

    m_bOpen = false;
}


bool QCommPort::read(QByteArray &ar)
{
	if(INVALID_HANDLE_VALUE == m_hCom)return false;

	char readBuffer[512];
    memset(readBuffer, 0, sizeof(readBuffer));
	DWORD wCount= 500;//读取的字节数
	BOOL bReadStat;

	bReadStat = ReadFile(m_hCom, readBuffer, wCount, &wCount, NULL);
	if(wCount <= 0)
	{
		return false;
	}

    ar.append(readBuffer, wCount);

    PurgeComm(m_hCom, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);//清空缓存

	return true;

}

bool QCommPort::readSyn(QByteArray &ar)
{
    if (INVALID_HANDLE_VALUE == m_hCom)return false;

    DWORD WaitEvent = 0, Bytes = 0;
    BOOL Status = FALSE;
    char ReadBuf[512];
    DWORD Error;
    COMSTAT cs = { 0 };  

    memset(ReadBuf, 0, sizeof(ReadBuf));

    m_ovWait.Offset = 0;
    Status = WaitCommEvent(m_hCom, &WaitEvent, &m_ovWait);

    ClearCommError(m_hCom, &Error, &cs);

    if (TRUE == Status //等待事件成功
        &&WaitEvent&EV_RXCHAR//缓存中有数据到达
        && cs.cbInQue > 0)//有数据
    {
        //数据已经到达缓存区，ReadFile不会当成异步命令，而是立即读取并返回True  
        Status = ReadFile(m_hCom, ReadBuf, sizeof(ReadBuf), &Bytes, NULL);
        if (Status != FALSE)
        {
            ar.append(ReadBuf, Bytes);
        }      

        PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_RXABORT);

        return true;
    }

    return false;
}

bool QCommPort::write(const QByteArray &ar)
{
	if(INVALID_HANDLE_VALUE == m_hCom)return false;

    PurgeComm(m_hCom, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);//清空缓存

	unsigned long dwBytesWrite;
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	BOOL bWriteStat;
	ClearCommError(m_hCom, &dwErrorFlags, &ComStat);

	dwBytesWrite = 0;
	bWriteStat = WriteFile(m_hCom, ar.data(), ar.size(), &dwBytesWrite, NULL);
	if(!bWriteStat)
	{
		return false;
	}

	return true;
}
