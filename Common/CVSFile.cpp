#include <QDateTime.h>
#include "CVSFile.h"
#include <qtextstream.h>
#include <qapplication.h>
#include "ThreadPrioc.h"
#include "SystemData.h"
#include "qcryptographichash.h"

const QString TimeFormatString ="yyyyMMddhhmmss";

QCVSFile::QCVSFile(const QString &filepath,const QString Fileprofix)
	:m_bFirst(true),m_bOpened(false),m_filePath(filepath),m_fileProfix(Fileprofix)
{
	this->open(genFileName(2));
}

QCVSFile::QCVSFile(const QString &filepath,const QString Fileprofix,int method)
	:m_bFirst(true),m_bOpened(false),m_filePath(filepath),m_fileProfix(Fileprofix)
{
	this->open(genFileName(method));
}

QCVSFile::~QCVSFile(void)
{
	this->close();
}

void QCVSFile::open(const QString & file)
{
	QString path;
	if(m_fileProfix.isEmpty())path = m_filePath + "/" + file;
	else path = m_filePath + "/" + m_fileProfix + "_" + file;

	m_pFile = new QFile(path);
	if(m_pFile)
	{
		m_bOpened =  m_pFile->open(QIODevice::WriteOnly |QIODevice::Append);
		if(!m_bOpened)System->setErrorCode(ERROR_CVS_OPEN_ALM);
	}
}

void QCVSFile::close()
{
	if(m_pFile && m_bOpened)
	{
		m_pFile->close();
	}
}

void QCVSFile::append(const QString & str)
{
	if(m_pFile)
	{		
		//QByteArray bytesStr = QCryptographicHash::hash(str.toLocal8Bit(), QCryptographicHash::Md5);
		//QString md5 = bytesStr.toHex();

		/*QByteArray bytesStr;
		bytesStr.append(str);
		QByteArray bytes64 = bytesStr.toBase64();*/	

		QTextStream stream(m_pFile);
		if(!m_bFirst)
		{
			stream << ",";
			stream << str;
		}
		else
		{
			stream << str;
			m_bFirst = false;
		}
	}
}

void QCVSFile::appendNewLine()
{
	if(m_pFile)
	{
		QTextStream stream(m_pFile);
		stream << "\r\n";
		m_bFirst = true;
	}
}

QCVSFile & QCVSFile::operator<<(int data)
{
	this->append(QString::number(data));

	return *this;
}

QCVSFile & QCVSFile::operator<<(double data)
{
	this->append(QString("%0").arg(data));

	return *this;
}

QCVSFile & QCVSFile::operator<<(const QString & str)
{
	this->append(str);

	return *this;
}

QCVSFile & QCVSFile::operator<<(QCVSFile::optType  opt)
{
	if(opt == sNewLine)
	{
		this->appendNewLine();
	}
	
	return *this;
}

QString QCVSFile::genFileName(int method)
{
	if(1 == method)
	{
		QDateTime dtm = QDateTime::currentDateTime();
		QString strdate = dtm.toString("yyyy-MM-dd-hh");
		strdate += ".csv";
		return strdate;
	}
	else if(2 == method)
	{
		QDateTime dtm = QDateTime::currentDateTime();
		QString strdate = dtm.toString("yyyy-MM-dd-HH-mm-ss");
		strdate += ".csv";
		return strdate;
	}
	else if(3 == method)
	{
		QDateTime dtm = QDateTime::currentDateTime();
		QString strdate = dtm.toString("yyyy-MM-dd");
		strdate += ".csv";
		return strdate;
	}
	else if(4 == method)
	{
		QDateTime dtm = QDateTime::currentDateTime();
		QString strdate = dtm.toString("yyyy-MM");
		strdate += ".csv";
		return strdate;
	}
	else
	{
		QDateTime dtm = QDateTime::currentDateTime();
		QString strdate = dtm.toString("yyyy-MM-dd");
		strdate += ".csv";
		return strdate;
	}
}


//--------------------------------------------------------------------
QDataRecordInstance * QDataRecordInstance::_instance_ = NULL;
bool QDataRecordInstance::_bDestoryed_ = false;;
//--------------------------------------------------------------------

QDataRecordInstance * QDataRecordInstance::instance()
{
	if(!_instance_)
	{
		_instance_ = new QDataRecordInstance;
		atexit(&QDataRecordInstance::destory);
	}

	return _instance_;
}

void QDataRecordInstance::destory()
{
	if(!_bDestoryed_)
	{
		delete _instance_;
		_instance_ = NULL;
		_bDestoryed_ = true;
	}
}

void QDataRecordInstance::write(const QString &path,const QString &profix,const QStringList &data)
{
	QAutoLocker loacker(&m_mutex);

	QCVSFile file(path, profix);
	//file << QDateTime::currentDateTime().toString(TimeFormatString);

	for (int i = 0; i < data.size(); i++)
	{
		file << data[i];
	}

	file.appendNewLine();
}


	
QDataRecord::QDataRecord(recordType type,const QString &path)
	:m_type(type),m_path(path)
{
	if(m_path.isEmpty())m_path = this->getPath();
}

QDataRecord::~QDataRecord()
{
	this->flush();
}

QDataRecord & QDataRecord::operator<<(const QString &str)
{
	m_data.append(str);

	return *this;
}
	
QDataRecord & QDataRecord::operator<<(int data)
{
	m_data.append(QString::number(data));

	return *this;
}

QDataRecord & QDataRecord::operator<<(double data)
{
	m_data.append(QString("%0").arg(data));

	return *this;
}

QDataRecord & QDataRecord::operator<<(QDataRecord::endType pt)
{
	if(pt == endl)
	{
		this->flush();
	}

	return *this;
}

QDataRecord & QDataRecord::operator<<(QDataRecord& record)
{
	for(int i = 0; i < record.m_dataPre.size(); i++)
	{
		m_data.append(record.m_dataPre[i]);
	}

	return *this;
}

void QDataRecord::flush()
{
	QDataRecordInstance * p = QDataRecordInstance::instance();
	if(p)
	{
		if(!m_data.isEmpty())
		{
			if(isEnableReocrd())
			{
				p->write(m_path,"",m_data);
			}
			m_dataPre.clear();
			m_dataPre = m_data;
			m_data.clear();
		}
	}
}

bool QDataRecord::isNewFile()
{
	QString szFilePath = m_path + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss")  + ".csv";
	QFile file(szFilePath);	
	return !file.exists();
}

bool QDataRecord::isEnableReocrd()
{
	if(record == m_type )return System->isEnableRecord();
	else if(detail == m_type)return System->isEnableRecordDetails();

	return true;
}

QString QDataRecord::getPath()
{
	QString str;
	if(record == m_type )
	{
		str = System->getRecordPath();
		if(str.isEmpty())str = QApplication::applicationDirPath() + "/record";
	}
	else if(detail == m_type)
	{
		str = System->getRecordDetailPath();
		if(str.isEmpty())str = QApplication::applicationDirPath() + "/recordDetails";
	}
	else if(uploadmissing == m_type)
	{
		str = QApplication::applicationDirPath() + "/uploadMissingRecord";
	}
	else
	{
		if(!m_path.isEmpty())str = m_path;
		else str = QApplication::applicationDirPath() + "/record";
	}

	return str;
}


QDataRecord qDataRecord()
{
	return QDataRecord(QDataRecord::record,"");
}

QDataRecord qDataDetailRecord()
{
	return QDataRecord(QDataRecord::detail,"");
}

QDataRecord qUserRecord(const QString &path)
{
	return QDataRecord(QDataRecord::user,path);
}

QDataRecord qUploadMissingRecord()
{
	return QDataRecord(QDataRecord::uploadmissing,"");
}
