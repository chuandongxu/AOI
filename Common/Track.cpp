/********************************************************************************
 功能简介：track信息输出模块，提供track的输出和调试信息输出功能，
           对调试信息的输出还需对象定义时给足够的支持(定义出输出)
		   函数的实现部分
 作    者：wolfseek
 联系方式：email:wolfseek@163.com,qq:535789574
 版权声明：本软件使用GPL开源协议。作者不能保证此软件在各种环境下都能健壮运行。
           作者不承担应用此软件产生的任何责任。请保留作者信息的，请谨慎使用。
 创建日期：2012-5-10
 维护记录：2012-7-8 重新定义启用track功能的宏，由QT_NO_DEBUG_STREAM
                    改为ENABLE_DEBUG，使在release下也能去用此模块功能，
					并追加LOG。
 后继完善： 暂无
 *********************************************************************************/

//#include "option.h"
#include "Track.h"
#include <stdarg.h>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <vector>
#include <time.h>
#include <QDir>
#include <QStandardPaths>
#include <QMutex>


using std::vector;
const QString TimeFormatString = "yyyy-MM-dd hh:mm:ss:zzz";
QtMessageHandler m_oldhandel;
QMutex g_mutex;


class CTrackHandel
{
protected:
	CTrackHandel();
	~CTrackHandel();

	static void Destory();

	void filterSpace(QString &str);
	void ReadConfig(const QString & path);
	void Splite(const QString & str,char deom, vector<QString> &strls);

	QString GetSuffix();
    QString CurrPath();
    QString AppDataPath();
	void ReProcessPath();

public:
	void track_track(const QString & state,const QString &str,const QString &fun,int line);
	void track_info(const QString &str,const QString &fun,int line);
	void track_warring(const QString &str,const QString &fun,int line);
	void track_error(const QString &str,const QString &fun,int line);

public:
	static CTrackHandel * Instance();
	QString GetLogFileName();
	int GetTrackLevel(){return m_trackLevel;}

protected:
	int m_trackLevel;
	QString m_path;
	QString m_logFile;
	QString m_suffix;
	
protected:
	static CTrackHandel * _instance_;
	static bool _bDestoryed_;
};
//////////////////////////////////////////////////////
void customMessageHandler(QtMsgType type,const QMessageLogContext &context, const QString &msg)
{
	if(msg.contains("QSocketNotifier: Socket notifiers cannot be enabled or disabled"))return;

	CTrackHandel * p = CTrackHandel::Instance();
	if(p)
	{
		QString path = p->GetLogFileName();
		if(path.isEmpty())return;

		QFile outFile(path);
		if(outFile.open(QIODevice::WriteOnly | QIODevice::Append))
		{
			QTextStream ts(&outFile);
			ts<< msg << "\r\n";

			(*m_oldhandel)(type,context,msg);
		}
	}
}

//////////////////////////////////////////////////
CTrackHandel * CTrackHandel::_instance_ = NULL;
bool CTrackHandel::_bDestoryed_ = false;
//////////////////////////////////////////////////
CTrackHandel::CTrackHandel()
	:m_trackLevel(0)
{
	m_oldhandel = qInstallMessageHandler(customMessageHandler);

    QString config = CurrPath();
    config += "/config/runing.ini";
	this->ReadConfig(config);
	this->ReProcessPath();

	m_suffix = GetSuffix();
	QDir dir;
	dir.mkpath(m_path);
}

CTrackHandel::~CTrackHandel()
{
	qInstallMessageHandler(0);
}

QString CTrackHandel::GetSuffix()
{
	time_t t;
	char buffer[100] = {0};

	time(&t);
	tm * ntm = localtime(&t);
	strftime(buffer,100,"%Y%m%d%H%M%S",ntm);
	return buffer;
}

QString CTrackHandel::GetLogFileName()
{
	static QString path = "";
	if(!path.isEmpty())return path;

	if(m_path.isEmpty())return "";
	if(m_logFile.isEmpty())return "";

	path = m_path + "/" + m_logFile + "_" + m_suffix;
	return path;
}

QString CTrackHandel::CurrPath()
{
    return QCoreApplication::applicationDirPath();
}

QString CTrackHandel::AppDataPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

void CTrackHandel::ReProcessPath()
{
	if(m_path.isEmpty())return;

	vector<QString> strls;
	Splite(m_path,'/',strls);
	if("APPDATA" == strls[0])
	{
        m_path = AppDataPath();
	}
	else if("CURDIR" == strls[0])
	{
		m_path = CurrPath();
	}
	else
	{
		m_path = CurrPath();
	}

	for(int i=1; i<strls.size(); i++)
	{
		m_path += "/" + strls[i];
	}
}

CTrackHandel * CTrackHandel::Instance()
{
	if(NULL == _instance_)
	{
		if(!_bDestoryed_)
		{
			_instance_ = new CTrackHandel;
			atexit(&CTrackHandel::Destory);
		}
	}

	return _instance_;
}

void CTrackHandel::Destory()
{
	if(_instance_)
	{
		delete _instance_;
		_bDestoryed_ = true;
	}
}

void CTrackHandel::track_track(const QString & state,const QString &str,const QString &fun,int line)
{
	if(m_trackLevel > 4)
	{
		qDebug("%s %s %s %d: %s",qPrintable(QDateTime::currentDateTime().toString(TimeFormatString)),
			qPrintable(state),qPrintable(fun),line,qPrintable(str));
	}
}

void CTrackHandel::track_info(const QString &str,const QString &fun,int line)
{
	if(m_trackLevel > 3)
	{
		qDebug("%s %s %s %d: %s",qPrintable(QDateTime::currentDateTime().toString(TimeFormatString)),
			"Info",qPrintable(fun),line,qPrintable(str));
	}
}

void CTrackHandel::track_warring(const QString &str,const QString &fun,int line)
{
	if(m_trackLevel >2)
	{
	qDebug("%s %s %s %d: %s",qPrintable(QDateTime::currentDateTime().toString(TimeFormatString)),
		"Warring",qPrintable(fun),line,qPrintable(str));
	}
}

void CTrackHandel::track_error(const QString &str,const QString &fun,int line)
{
	if(m_trackLevel > 1)
	{
	qDebug("%s %s %s %d: %s",qPrintable(QDateTime::currentDateTime().toString(TimeFormatString)),
		"Error",qPrintable(fun),line,qPrintable(str));
	}
}


void CTrackHandel::ReadConfig(const QString & path)
{
	QString filename = path;

	QFile file(filename);
	if(!file.open(QFile::ReadOnly))return;

	QTextStream is(&file);

	QString line;
	while(!is.atEnd())
	{
		line = is.readLine();

		vector<QString> strls;
		Splite(line, '=',strls);
		if(strls.size() == 2)
		{
			if(strls[0] == "TRACK_LEVEL")
			{
				m_trackLevel = atoi(strls[1].toStdString().c_str());
			}
			else if(strls[0] == "TRACK_FILE")
			{
				m_logFile = strls[1];
			}
			else if(strls[0] == "TRACK_FILE_PATH")
			{
				m_path = strls[1];
			}
		}
	}
	
}

void CTrackHandel::Splite(const QString & str,char deom, vector<QString> &strls)
{
	QString tem;
	for(int i=0;i<str.size();i++)
	{
		if(str.at(i) != deom)tem += str.at(i);
		else 
		{
			filterSpace(tem);
			if(!tem.isEmpty())strls.push_back(tem);
			tem.clear();
		}
	}
	filterSpace(tem);
	if(!tem.isEmpty())strls.push_back(tem);
}

void CTrackHandel::filterSpace(QString &str)
{
	str = str.trimmed();
}


///////////////////////////////////////////////////////////////////////
CFuncTrack::CFuncTrack(const char *fun,int line,const char *str,...)
:m_func(NULL),m_line(line)
{
	g_mutex.lock();

	if(fun)m_func = new QString(fun);
	char buffer[2048] = {0};

	va_list valist;
	va_start(valist, str);
	vsnprintf(buffer, 2048, str, valist);
	va_end(valist);

	CTrackHandel * p = CTrackHandel::Instance();
	if(p)
	{
		if(p->GetTrackLevel() > 4)
		{
			qDebug("%s %s %s %d %s: %s",qPrintable(QDateTime::currentDateTime().toString(TimeFormatString)),
				"FUNC",fun,line,"Enter",qPrintable(buffer));
		}
	}
	g_mutex.unlock();
}

CFuncTrack::~CFuncTrack()
{
	g_mutex.lock();

	CTrackHandel * p = CTrackHandel::Instance();
	if(p)
	{
		QString fun;
		if(m_func)fun = *(QString*)m_func;
		if(p->GetTrackLevel() > 4)
		{
			qDebug("%s %s %s %d %s",qPrintable(QDateTime::currentDateTime().toString(TimeFormatString)),
				"FUNC",qPrintable(fun),m_line,"Leave");
		}
	}

	if(m_func)delete m_func;
	g_mutex.unlock();
}


void track_print(int iLevel,const char* fun,int line,const char * str,...)
{
	g_mutex.lock();
	char buffer[2048] = {0};

	va_list valist;
	va_start(valist, str);
	vsnprintf(buffer, 2048, str, valist);
	va_end(valist);

	CTrackHandel * p = CTrackHandel::Instance();
	if(p)
	{
		switch(iLevel)
		{
		case 1:p->track_error(buffer,fun,line);break;
		//case 2:p->track_warring(buffer,fun,line);break;
		case 3:p->track_warring(buffer,fun,line);break;
		case 4:p->track_info(buffer,fun,line);break;
		case 5:p->track_track("track",buffer,fun,line);break;
		}
	}

	g_mutex.unlock();
}
