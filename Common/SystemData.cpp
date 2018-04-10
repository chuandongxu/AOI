#include <qdatetime>
#include <QApplication>
#include <QSqldatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QCryptographicHash>
#include <QFile>
#include <QNetworkinterface>
#include <qsettings.h>
#include <qdebug>
#include "ThreadPrioc.h"
#include "Track.h"
#include "authright.h"
#include "SystemData.h"
#include <QWaitCondition>
#include <QThread>

#include "dog_api_cpp.h"
#include "dog_vcode.h"
#include "errorprinter.h"
#include "encrypted_string1.h"
#include "userdialog.h"
#include "usermanager.h"
#include "sysmessagebox.h"
#include <memory>

#define ENABLE_RECORD   "enableRecord"
#define ENABLE_RECORD_DETAILS   "enableRecordDetails"
#define ENABLE_OUTLINE   "enableOutline"
#define RECORD_PATH  "recordPath"
#define RECORD_DETAILS_PATH "recordDetailsPath"
#define ENABLE_BACKUP   "enableBackupData"
#define BACKUP_PATH "backupDataPath"

#define DEFALUT_USER  "aoiDebuger"
#define DEFALUT_PASS  "aoi.com"

#define AUCH_RIGHT_ENT_STRING "NFG_AOI_ENC#!@.txt"


void randData(char * buffer,int size)
{
	static char dic[] = "abcdefghijklmnopqrstuvwxyz=0123456789-+_ABCDEFGHIJKLMNOPQRSTUVWXYZ<>?&*#";

	int m = sizeof(dic);
	for(int i=0; i<size; i++)
	{
		int n = rand() % m;
		buffer[i] = dic[n];
	}
}

QAuthData::QAuthData()
{
	char * p = (char*)this;
	int size = sizeof(QAuthData);
	randData(p,size);
}

QString QAuthData::toString(char *p, int size)
{
    char buffer[1025];
    int n = size<1204?size:1024;
    memcpy(buffer,p,n);
    buffer[n] = 0;

    QString str = buffer;
    return str;
}

void QAuthData::fromString(char *p,int size,const QString &str)
{
    int n = size < str.size()?size:str.size();
    for(int i=0; i<n; i++)
    {
        p[i] = str[i].cell();
    }
}
//------------------------------------------------------------
QSystem * QSystem::_instance_ = NULL;
bool QSystem::_destoryed_ = false;
//------------------------------------------------------------
QSystem::QSystem()
	:m_mutex(QMutex::Recursive)
	,m_errorCode(ERROR_NO_ERROR)
	,m_imStop(false)
	,m_userLevel(0)
{
	QString path = QApplication::applicationDirPath();
	path += "/data/systemData.s3db";
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(path);

	if( db.open())
	{
		m_errCodeMap.init();
		LoadData();
	}
	else
	{
		qDebug() << "error open systemData.s3db";
	}

	initErrorModel();
    initConfig();
}
	
QSystem::~QSystem()
{
	QSqlDatabase::database().close();
}


 QSystem * QSystem::instance()
 {
	 if(!_instance_)
	 {
		 if(!_destoryed_)
		 {
			 _instance_ = new QSystem;
			 atexit(&QSystem::desstory);
		 }
	 }

	 return _instance_;
 }

 void QSystem::desstory()
 {
	 if(_instance_)
	 {
		 delete _instance_;
		 _instance_ = NULL;
		 _destoryed_ = true;
	 }
 }

void QSystem::loadQss(const QString &fileName)
{
	QString path = QApplication::applicationDirPath();
    path += "/image/";
    path += fileName;

    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
		 QApplication * p = (QApplication*)QApplication::instance();
		 if(p)
		 {
			 p->setStyleSheet(file.readAll().data());
		 }
    }
	else qDebug() << "load qss fail!";

}

void QSystem::queryWarring(QStandardItemModel &model)
{
	QString str = QString("select errortime,errorCode,errorMessage from T_warring order by errortime desc");
	QSqlQuery query(str);
	QSqlRecord rec = query.record();

	int n = rec.count();
	model.insertColumns(0,n);
	int row = 0;
	while (query.next())
	{
		model.insertRow(row);
		for(int i = 0; i< n; i++)
		{
			if(i == 2)
			{
				QString str = query.value(i).toByteArray();
				//QByteArray arr = query.value(i).toByteArray();
				//QString str = QString::fromRawData(arr.data(),arr.size());
				model.setData(model.index(row,i),str);
			}
			else if(i == 1)
			{
				int code = query.value(i).toInt();
				QString str = QString("%0").arg(code,0,16);
				while(str.size() < 8)str.insert(0,'0');
				str.insert(0,"0x");
				model.setData(model.index(row,i),str);
			}
			else
			{
				model.setData(model.index(row,i),query.value(i));
			}
		}
		row++;
	}
}

void QSystem::queryOptLog(QStandardItemModel &model)
{
	QString str = QString("select tm,name,value,descr from T_warring order by tm desc");
	QSqlQuery query(str);
	QSqlRecord rec = query.record();

	int n = rec.count();
	model.insertColumns(0,n);
	int row = 0;
	while (query.next())
	{
		model.insertRow(row);
		for(int i = 0; i< n; i++)
		{
			model.setData(model.index(row,i),query.value(i));
		}
		row++;
	}
}

void QSystem::waitTime(int msTime)
{
	//QMutex mutex;
	//QWaitCondition condtion;
	//mutex.lock();
	//condtion.wait(&mutex,msTime);
	int count = 0;
	while(1)
	{
		if(++count >= msTime)break;
		QApplication::processEvents();
		QThread::msleep(1);
	}
}
	

void QSystem::LoadData()
{	
	QString queryStr = QString("SELECT name,value from T_param");
	QSqlQuery query(queryStr);
	while (query.next())
	{
		QString name = query.value(0).toString();
		m_params[name] = query.value(1);
    }

	QString queryStrUserData = QString("SELECT name,value from T_paramUserData");
	QSqlQuery queryUserData(queryStrUserData);
	while (queryUserData.next())
	{
		QString name = queryUserData.value(0).toString();
		m_paramUserData[name] = queryUserData.value(1);
	}
}

QVariant QSystem::getParam(const QString &name)
{
	QAutoLocker locker(&m_mutex);

	if (m_paramUserData.contains(name))
	{
		return m_paramUserData[name];
	}

	return QVariant();
}

void QSystem::setParam(const QString &name,const QVariant &data)
{
	QAutoLocker loacker(&m_mutex);

	m_paramUserData[name] = data;

	if (isExist(QString("select id from T_paramUserData where name = '%0'").arg(name)))
	{
		QString queryStr = QString("update T_paramUserData set value='%0' where name = '%1'")
			.arg(data.toString()).arg(name);
		QSqlQuery query;
		query.exec(queryStr);
	}
	else
	{
		QString queryStr = QString("insert into T_paramUserData(name,value) values('%0','%1')")
			.arg(name).arg(data.toString());
		QSqlQuery query;
		query.exec(queryStr);
	}

	emit paramChange(name, true);
}

void QSystem::delParam(const QString &name)
{
	QAutoLocker loacker(&m_mutex);

	m_paramUserData.remove(name);

	QString queryStr = QString("delete from T_paramUserData where name = '%0'").arg(name);
	//QString queryStr = QString("delete from T_param");

	QSqlQuery query;
	query.exec(queryStr);
}

QStringList QSystem::getParamKeys(const QString &condtion)
{
	QStringList keys;
	QStringList ls = m_paramUserData.keys();
	for (int i = 0; i < ls.size(); i++)
	{
		if (ls[i].contains(condtion))
		{
			keys.append(ls[i]);
		}
	}

	return keys;
}

QVariant QSystem::getSysParam(const QString &name)
{
	QAutoLocker locker(&m_mutex);

	if(m_params.contains(name))
	{
		return m_params[name];
	}

	return QVariant();
}
	
void QSystem::setSysParam(const QString &name,const QVariant &data)
{
	QAutoLocker locker(&m_mutex);

	m_params[name] = data;

	if(isExist(QString("select id from T_param where name = '%0'").arg(name)))
	{
		QString queryStr = QString("update T_param set value='%0' where name = '%1'")
			.arg(data.toString()).arg(name);
		QSqlQuery query;
		query.exec(queryStr);
	}
	else
	{
		QString queryStr = QString("insert into T_param(name,value) values('%0','%1')")
			.arg(name).arg(data.toString());
		QSqlQuery query;
		query.exec(queryStr);
	}

	emit paramChange(name,true);
}

void QSystem::delSysParam(const QString &name)
{
	QAutoLocker loacker(&m_mutex);

	m_params.remove(name);

	QString queryStr = QString("delete from T_param where name = '%0'").arg(name);
	//QString queryStr = QString("delete from T_param");
	
	QSqlQuery query;
	query.exec(queryStr);
}

QStringList QSystem::getSysParamKeys(const QString &condtion)
{
	QStringList keys;
	QStringList ls = m_params.keys();
	for(int i=0; i< ls.size(); i++)
	{
		if(ls[i].contains(condtion))
		{
			keys.append(ls[i]);
		}
	}

	return keys;
}

bool QSystem::isExist(const QString &filtterSql)
{
	QSqlQuery query(filtterSql);
	while (query.next()) 
	{
		return true;
	}

	return false;
}

void QSystem::addOptLog(const QString &name,const QString &val,const QString &descr)
{
	QDateTime dtm = QDateTime::currentDateTime();
	QString strdate = dtm.toString("yyyy-MM-dd hh:mm:ss.zzz");

	QString queryStr = QString("insert into T_opt(tm,name,value,descr) values('%0','%1','%2','%3')")
			.arg(strdate).arg(name).arg(val).arg(descr);
	QSqlQuery query;
	query.exec(queryStr);
}

QErrorModel * QSystem::getErrorModel()
{
	return &m_errModel;
}

void QSystem::setTrackInfo(const QString &msg, bool bDisplayHM)
{
	QDateTime dtm = QDateTime::currentDateTime();
	QString strdate = dtm.toString("yyyy-MM-dd hh:mm:ss");

	TR_TRACK(msg.toLocal8Bit().data());

	//if(this->getErrorDispLevel() == -1)return;

	if(!bDisplayHM) return;
	emit errorInfo(strdate,msg,0x90000000);
}

void QSystem::setErrorCode(unsigned int code)
{
	QAutoLocker loacker(&m_mutex);

	m_errorCode = code;
	QString msg =  m_errCodeMap.getErrorString(m_errorCode);

	//QString str;
	//if(msg.isEmpty())str = QString(QStringLiteral("未知确切含义错误 %0")).arg(code);
	//else str = QString(QStringLiteral("发生错误 : %0")).arg(msg);
	//QSystem::showMessage(str,true);

	QDateTime dtm = QDateTime::currentDateTime();
	QString strdate = dtm.toString("yyyy-MM-dd hh:mm:ss");

	if(code < 0x60000000)
	{
		QString queryStr = QString("insert into T_warring(errorCode,errorMessage,errortime) values(%0,'%1','%2')")
			.arg(m_errorCode).arg(msg).arg(strdate);
		QSqlQuery query;
		query.exec(queryStr);
	}
	if(code >= 0x40000000)TR_INFO(msg.toLocal8Bit().data());
	else if(code >= 0x30000000)TR_WARRING(msg.toLocal8Bit().data());
	else TR_ERROR(msg.toLocal8Bit().data());
	
	emit errorInfo(strdate,msg,code);
}

QString QSystem::getErrorString()
{
	QAutoLocker loacker(&m_mutex);
	return m_errCodeMap.getErrorString(m_errorCode);
}

void QSystem::addErrorMap(unsigned int id,const QString &descr)
{
	QAutoLocker loacker(&m_mutex);
	m_errCodeMap.addErrorMap(id,descr);
}

void * QSystem::dataPtr(const QString &key)
{
	QAutoLocker loacker(&m_mutex);

	if(m_ptrData.contains(key))
	{
		return m_ptrData[key];
	}

	return NULL;
}

void QSystem::setDataPtr(const QString &key,void * data)
{
	QAutoLocker loacker(&m_mutex);
	m_ptrData[key] = data;
}

QVariant QSystem::data(const QString &key)
{
	QAutoLocker loacker(&m_mutex);
	if(m_data.contains(key))
	{
		return m_data[key];
	}

	return QVariant();
}
	
void QSystem::setData(const QString &key,const QVariant &data)
{
	QAutoLocker loacker(&m_mutex);
	m_data[key] = data;

	emit dataChange(key);
}

void QSystem::setUser(const QString & user,int level)
{
	m_user = user;
	m_userLevel = level;
}
	
void QSystem::getUser(QString & user,int &level)
{
	user = m_user;
	level = m_userLevel;
}

int QSystem::getUserLevel()
{
	return m_userLevel;
}

bool QSystem::getUserPwd(const QString user, QString &pwd, int &level)
{
	if(DEFALUT_USER == user)
	{
		pwd = QCryptographicHash::hash(DEFALUT_PASS,QCryptographicHash::Md5).toBase64().data();
		level = USER_LEVEL_DEBUG;

		return true;
	}

	QString queryStr = QString("SELECT password,level FROM T_userManager where username = '%0'").arg(user);
	QSqlQuery query(queryStr);
    while (query.next()) 
	{
		pwd = query.value(0).toString();
		level = query.value(1).toInt();

		return true;
	}

	return false;
}

bool QSystem::checkUserPwd(const QString& user, const QString& pwd)
{
	if (!checkRuntimeAuthRight())
	{
		setTrackInfo(QStringLiteral("系统没有授权！"));
		return false;
	}
	
	QString targPwd;
	int level;
	if (getUserPwd(user, targPwd, level))
	{
		if (level > USER_LEVEL_OPT)
		{
			QDateTime dtm = QDateTime::currentDateTime();
			QString dateTM = dtm.toString("yyyyMM");
			QByteArray pwdArr;
			if (USER_LEVEL_MANAGER == level)
			{
				targPwd = "admin";
			}
			else if (USER_LEVEL_TECH == level)
			{
				targPwd = "root";
			}
			pwdArr.append(targPwd);
			pwdArr.append(dateTM);
			QString pwdHash = QCryptographicHash::hash(pwdArr, QCryptographicHash::Md5).toBase64().data();
			QString pwdCacl = pwdHash.left(6);
			if (pwd == pwdCacl)
			{
				setUser(user, level);
				return true;
			}
		}
		else
		{
			QByteArray pwdArr;
			pwdArr.append(pwd);
			QString pwdHash = QCryptographicHash::hash(pwdArr, QCryptographicHash::Md5).toBase64().data();
			if (pwdHash == targPwd)
			{
				setUser(user, level);
				return true;
			}
		}
	}

	return false;
}

void QSystem::enableRecord(bool s)
{
	if(s)
	{
		setSysParam(ENABLE_RECORD,1);
	}
	else
	{
		setSysParam(ENABLE_RECORD,0);
	}
}
	
bool QSystem::isEnableRecord()
{
	if(1 == getSysParam(ENABLE_RECORD).toInt())return true;

	return false;
}

void QSystem::setRecordPath(const QString &path)
{
	setSysParam(RECORD_PATH,path);
}

QString QSystem::getRecordPath()
{
	return getSysParam(RECORD_PATH).toString();
}

void QSystem::enableRecordDetails(bool s)
{
	if(s)
	{
		setSysParam(ENABLE_RECORD_DETAILS,1);
	}
	else
	{
		setSysParam(ENABLE_RECORD_DETAILS,0);
	}
}
	
bool QSystem::isEnableRecordDetails()
{
	if(1 == getSysParam(ENABLE_RECORD_DETAILS).toInt())return true;

	return false;
}

void QSystem::setRecordDetailsPath(const QString &path)
{
	setSysParam(RECORD_DETAILS_PATH,path);
}

QString QSystem::getRecordDetailPath()
{
	return getSysParam(RECORD_DETAILS_PATH).toString();
}

void QSystem::enableBackupData(bool s)
{
	if(s)
	{
		setSysParam(ENABLE_BACKUP,1);
	}
	else
	{
		setSysParam(ENABLE_BACKUP,0);
	}
}

bool QSystem::isEnableBackupData()
{
	if(1 == getSysParam(ENABLE_BACKUP).toInt())return true;

	return false;
}

void QSystem::setBackupDataPath(const QString &path)
{
	setSysParam(BACKUP_PATH,path);
}

QString QSystem::getBackupDataPath()
{
	return getSysParam(BACKUP_PATH).toString();
}

void QSystem::enableOutline(bool s)
{
	if(s)
	{
		setData(ENABLE_OUTLINE,1);
	}
	else
	{
		setData(ENABLE_OUTLINE,0);
	}
}

bool QSystem::isEnableOutline()
{
	if(1 == data(ENABLE_OUTLINE).toInt())return true;

	return false;
}

void QSystem::setRunSpeed(int n)
{
	setSysParam("RUN_SPEED",n);
}

int QSystem::getRunSpeed()	
{
	return getSysParam("RUN_SPEED").toInt();
}

QString QSystem::getLangConfig()
{
	QString path = QApplication::applicationDirPath();
	path += "/config/runing.ini";
	QSettings setttings(path, QSettings::IniFormat);

	return setttings.value("lang/langName","ch").toString();
}

void QSystem::setLangConfig(const QString & lang)
{
	QString path = QApplication::applicationDirPath();
	path += "/config/runing.ini";
	QSettings setttings(path,QSettings::IniFormat);

	setttings.setValue("lang/langName",lang);
}

bool QSystem::readAuthData(QAuthData * data)
{
	char * p = (char*)data;
	int size = sizeof(QAuthData);

	QString path = QApplication::applicationDirPath();
	path += "/sa.key";
	QFile file(path);
	if(file.open(QIODevice::ReadOnly))
	{
		int n = file.read(p,size);
		if(n != size)return false;

		return true;
	}

	return false;
}

bool QSystem::writeAuthData(QAuthData * data)
{
	char * p = (char*)data;
	int size = sizeof(QAuthData);

	QString path = QApplication::applicationDirPath();
	path += "/sa.key";
	QFile file(path);
	if(file.open(QIODevice::WriteOnly))
	{
		int n = file.write(p,size);
		if(n != size)return false;

		return true;
	}

	return false;
}

void QSystem::userGoHome()
{
	emit goHome();
}

void QSystem::userImStop()
{
	m_imStop = true;
	emit imStop();
}

void QSystem::userStart()
{
	emit start();
}

void QSystem::userStop()
{
	emit stop();
}

void QSystem::userPause()
{
	emit pause();
}

void QSystem::userReset()
{
	emit reset();
}

bool QSystem::isImStop()
{
	return m_imStop;
}

bool QSystem::doAuthright(const QString &authCode)
{
	QByteArray arr;
	arr.append(authCode);
	QString authMd5 = QCryptographicHash::hash(arr,QCryptographicHash::Md5).toBase64().data();

	QAuthData data;
	this->readAuthData(&data);
	QString authkey = QAuthData::toString(data.authCode,24);
	if(authMd5 != authkey)return false;
	
	QString str;
	QList<QNetworkInterface> ls = QNetworkInterface::allInterfaces();
    for(int i=0; i<ls.size(); i++)
    {
       QString tem = ls[i].hardwareAddress();
       str += tem;
    }

	arr.clear();
	arr.append(str);
	QString mac = QCryptographicHash::hash(arr,QCryptographicHash::Md5).toBase64().data();
	QAuthData authdata;
	memcpy(authdata.authCode,data.authCode,24);
	authdata.bAuth1 = 'a';
	authdata.bAuth2 = 'a';
	QAuthData::fromString(authdata.mac,24,mac);

	return this->writeAuthData(&authdata);
}

bool QSystem::checkAuthRight()
{
	QString str;
	QList<QNetworkInterface> ls = QNetworkInterface::allInterfaces();
    for(int i=0; i<ls.size(); i++)
    {
       QString tem = ls[i].hardwareAddress();
       str += tem;
    }

	QByteArray arr;
	arr.append(str);
	QString mac = QCryptographicHash::hash(arr,QCryptographicHash::Md5).toBase64().data();

	QAuthData data;
	this->readAuthData(&data);

	QString mackey = QAuthData::toString(data.mac,24);
	if(mac == mackey)return true;

	return false;
}

bool QSystem::execAuth()
{
	QAuthRight dlg;
	if(dlg.exec() == QDialog::Accepted)return true;

	return false;
}

bool QSystem::checkRuntimeAuthRight()
{
	return true;

	//Prints the error messages for the return values of the functions
	ErrorPrinter errorPrinter;

	//Used to hold the return value of the called functions
	dogStatus status;

	CDog dog1(CDogFeature::fromFeature(1));
	status = dog1.login(vendor_code);
	errorPrinter.printError(status);

	if (!DOG_SUCCEEDED(status))
	{
		status = dog1.logout();
		errorPrinter.printError(status);
		return false;
	}

	unsigned char encryptStrArr1Test[ENCRYPT_BUFFER_LENGTH1];
	memcpy(encryptStrArr1Test, encryptStrArr1, ENCRYPT_BUFFER_LENGTH1);

	status = dog1.decrypt(encryptStrArr1Test, ENCRYPT_BUFFER_LENGTH1);
	errorPrinter.printError(status);
	if (!DOG_SUCCEEDED(status))
	{
		status = dog1.logout();
		errorPrinter.printError(status);
		return false;
	}

	unsigned char decryptStrArr[ENCRYPT_BUFFER_LENGTH1];
	memcpy(decryptStrArr, AUCH_RIGHT_ENT_STRING, ENCRYPT_BUFFER_LENGTH1);
	for (int i = 0; i < ENCRYPT_BUFFER_LENGTH1; i++)
	{
		if (encryptStrArr1Test[i] != decryptStrArr[i])
		{
			qDebug() << "String is not correct!";
			status = dog1.logout();
			errorPrinter.printError(status);
			return false;
		}
	}

	//if (strcmp(encryptStrArr1Test, decryptStrArr) != 0)
	
	status = dog1.logout();
	errorPrinter.printError(status);

	return true;
}

void QSystem::initErrorModel()
{
	QStringList ls;
	ls << "" << QStringLiteral("时间")  << QStringLiteral("运行信息");

	m_errModel.setHorizontalHeaderLabels(ls);

	connect(this,SIGNAL(errorInfo(const QString &,const QString&,unsigned int)),
			&m_errModel,SLOT(onErrorInfo(const QString &,const QString&,unsigned int)));	
}

void QSystem::initConfig()
{
    QString path = QApplication::applicationDirPath();
	path += "/config/runing.ini";
	QSettings setttings(path, QSettings::IniFormat);

	m_bRunOffline = setttings.value("General/RunOffLine","true").toBool();
}

//----------------------------------------------------------------------------------------------------
static QSysMessageBox *g_box = NULL;
static bool g_bShowed = false;

void QSystem::showMessage(const QString &title,const QString &msg,int ErrorLevel)
{
	if(NULL == g_box)
	{
		g_box = new QSysMessageBox;
		
		g_box->setWindowModality(Qt::ApplicationModal);
		Qt::WindowFlags fs = g_box->windowFlags();
		fs = Qt::CustomizeWindowHint|Qt::FramelessWindowHint;
		g_box->setWindowFlags(fs);

		//connect(QApplication::instance(),SIGNAL(aboutToQuit()),SLOT(onQiut()));
	}

	if(g_bShowed)
	{
		g_box->setText(msg);
		g_box->setTitle(title);
		if(ErrorLevel > 3)g_box->enableCloseBtn(true);
		else g_box->enableCloseBtn(false);

		g_box->show();
		g_bShowed = true;

		QApplication::processEvents();
	}
	else
	{
		g_box->setText(msg);
		g_box->setTitle(title);
		if(ErrorLevel > 3)g_box->enableCloseBtn(true);
		else g_box->enableCloseBtn(false);
		
		g_box->show();
		g_bShowed = true;
	}
}

void QSystem::closeMessage()
{
	if(g_box)
	{
		g_box->hide();
		g_bShowed = false;
		QApplication::processEvents();
	}
}

bool QSystem::isMessageShowed()
{
	return g_bShowed;
}

/*static*/ int QSystem::showInteractMessage(const QString &title,const QString &msg)
{
    QSysMessageBox msgBox(NULL, true);
    msgBox.setText(msg);
    msgBox.setTitle(title);
    msgBox.setWindowModality(Qt::NonModal);
	msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox.show();
    msgBox.raise();
    msgBox.activateWindow();
    return msgBox.exec();
}

bool QSystem::ChangeUser()
{
	QUserDialog dlg;
	if(dlg.exec() == QDialog::Accepted)return true;

	return false;
}

void QSystem::ManagerUser()
{
	QUserManager dlg;
	dlg.exec();
}
//-----------------------------------------------------------

void QErrorModel::onErrorInfo(const QString &data,const QString &msg,unsigned int level)
{
	int row = rowCount();
	if(row > 30)
	{
		removeRow(0);
		row = rowCount();
	}
	
	unsigned int warringFlsg = (level & 0xf0000000) >> 28;
	QString path = QApplication::applicationDirPath();
	//if(level >= 0x40000000)path += "/image/tr_info.png";
	//else if(level >= 0x30000000)path += "/image/tr_warring.png";
	//else path += "/image/tr_error.png";

	if(0 == warringFlsg || 1 == warringFlsg) path += "/image/tr_error.png";
	else if(3 == warringFlsg || 5 == warringFlsg) path += "/image/tr_warring.png";
	else path += "/image/tr_info.png";
	
	QPixmap pixmap;
	pixmap.load(path);

	insertRow(row);
	setData(index(row,0),pixmap,Qt::DecorationRole);
	setData(index(row,1),data);
	setData(index(row,2),msg);
	//ui.treeView->scrollTo(m_model.index(row,0),QAbstractItemView::PositionAtBottom);
}
