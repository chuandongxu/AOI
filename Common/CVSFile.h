#pragma once
#include "Common_global.h"
#include <qstring.h>
#include <qfile.h>
#include <qmutex.h>
#include <qstringlist.h>

class COMMON_EXPORT QCVSFile
{
	enum optType{sNewLine};
public:
	QCVSFile(const QString &filepath,const QString Fileprofix);
	QCVSFile(const QString &filepath,const QString Fileprofix,int method);
	~QCVSFile(void);	

	void append(const QString & str);
	void appendNewLine();

	QCVSFile & operator<<(int data);
	QCVSFile & operator<<(double data);
	QCVSFile & operator<<(const QString & str);
	QCVSFile & operator<<(optType opt);
	
protected:
	void open(const QString & file);
	void close();
	QString genFileName(int method);
	
private:
	bool m_bFirst;
	QFile * m_pFile;
	bool m_bOpened;
	QString m_filePath;
	QString m_fileProfix; 
};


class COMMON_EXPORT QDataRecordInstance
{
protected:
	QDataRecordInstance(){};
	~QDataRecordInstance(){};

public:
	static QDataRecordInstance * instance();

	void write(const QString &path,const QString &profix,const QStringList &data);

protected:
	static void destory();

protected:
	QMutex m_mutex;

private:
	static QDataRecordInstance * _instance_;
	static bool _bDestoryed_;
};

class COMMON_EXPORT QDataRecord
{
public:
	enum endType{endl};
	enum recordType{record,detail,user,uploadmissing};
public:
	QDataRecord(recordType type = record,const QString &path = "");
	~QDataRecord();

	QDataRecord & operator<<(const QString &str);
	QDataRecord & operator<<(int data);
	QDataRecord & operator<<(double data);
	QDataRecord & operator<<(endType pt);
	QDataRecord & operator<<(QDataRecord& record);

	void flush();
	bool isNewFile();

protected:
	bool isEnableReocrd();
	QString getPath();

protected:
	QStringList m_data;
	QStringList m_dataPre;
	recordType m_type;
	QString m_path;
};

QDataRecord COMMON_EXPORT qDataRecord();
QDataRecord COMMON_EXPORT qDataDetailRecord();
QDataRecord COMMON_EXPORT qUserRecord(const QString &path = "");
QDataRecord COMMON_EXPORT qUploadMissingRecord();