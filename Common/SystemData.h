#ifndef _SYSTEM_DATA_H_
#define _SYSTEM_DATA_H_

#include <QObject>
#include <QMutex>
#include <QStandardItemModel>
#include "Common_global.h"
#include "ErrorCode.h"
#include "UserDataArch.h"

//定义登陆系统的权限 ---------------------------------
//操作员
#define USER_LEVEL_OPT 0
//设备管理员
#define USER_LEVEL_MANAGER 1
//技术维护人员
#define USER_LEVEL_TECH  2
//设备调试和测试人员
#define USER_LEVEL_DEBUG 5


struct QPoint1D
{
	double x;
};

struct QPoint2D
{
	double x;
	double y;
};

struct QPoint3D
{
	double x;
	double y;
	double z;
};

struct QPoint4D
{
	double x;
	double y;
	double z;
	double c;
	QPoint4D()
	{
		x = 0.0;
		y = 0.0;
	    z = 0.0;
		c = 0.0;
	}
};

struct QPoint6D
{
	double x;
	double y;
	double z;
	double a;
	double b;
	double c;

	QPoint6D()
		:x(0),y(0),z(0),a(0),b(0),c(0)
	{
	}
};


struct QAuthData
{
    char pre[10];
    char bAuth1;
    char pre1[512];
    char bAuth2;
    char pre2[20];
    char authCode[24];
    char pre3[100];
    char mac[24];
    char pre4[300];

    QAuthData();

    static QString toString(char *p, int size);
    static void fromString(char *p,int size,const QString &str);
};

void randData(char * buffer,int size);


class COMMON_EXPORT QSystem : public QObject
{
	Q_OBJECT
public:
	QSystem();
	~QSystem();

	static QSystem * instance();

	static void showMessage(const QString &title, const QString &msg, int ErrorLevel = 4);
	static void closeMessage();
	static bool isMessageShowed();

    static int showInteractMessage(const QString &title, const QString &msg);

	static bool ChangeUser();
	static void ManagerUser();

	static void loadQss(const QString &file);
	static void waitTime(int msTime);

	static void queryWarring(QStandardItemModel &model);
	static void queryOptLog(QStandardItemModel &model);	

public:
	void * dataPtr(const QString &key);
	void setDataPtr(const QString &key,void * data);

	QVariant data(const QString &key);
	void setData(const QString &key,const QVariant &data);

	QVariant getParam(const QString &name);
	void setParam(const QString &name,const QVariant &data);
	void delParam(const QString &name);
	QStringList getParamKeys(const QString &condtion);

	QVariant getSysParam(const QString &name);
	void setSysParam(const QString &name,const QVariant &data);
	void delSysParam(const QString &name);
	QStringList getSysParamKeys(const QString &condtion);

	void addOptLog(const QString &name,const QString &val,const QString &descr);

	QErrorModel * getErrorModel();
    void setTrackInfo(const QString &msg, bool bDisplayHM = true);
	void setErrorCode(unsigned int code);
	unsigned int getErrorCode(){return m_errorCode;};
	QString getErrorString();
	void addErrorMap(unsigned int id,const QString &descr);

	void setUser(const QString & user,int level);
	void getUser(QString & user,int &level);
	int getUserLevel();
	bool getUserPwd(const QString user, QString &pwd, int &level);
	bool checkUserPwd(const QString& user, const QString& pwd);

	void enableRecord(bool s);
	bool isEnableRecord();
	void setRecordPath(const QString &path);
	QString getRecordPath();

	void enableRecordDetails(bool s);
	bool isEnableRecordDetails();
	void setRecordDetailsPath(const QString &path);
	QString getRecordDetailPath();

	void enableBackupData(bool s);
	bool isEnableBackupData();
	void setBackupDataPath(const QString &path);
	QString getBackupDataPath();

	void enableOutline(bool s);
	bool isEnableOutline();

	void setRunSpeed(int n);
	int getRunSpeed();

	QString getLangConfig();
	void setLangConfig(const QString & lang);
    bool isRunOffline() const;

public:
	void userGoHome();
	void userImStop();
	void userStart();
	void userStop();
	void userPause();
	void userReset();
	bool isImStop();

public:
	bool doAuthright(const QString &authCode);
	bool checkAuthRight();
	bool execAuth();

	bool checkRuntimeAuthRight();

signals:
	void errorInfo(const QString &data,const QString &msg,unsigned int level);
	void goHome();	
	void imStop();
	void start();
	void stop();
	void pause();
	void reset();
	void dataChange(const QString &key);
	void paramChange(const QString &key,bool isSysParam);
	void productChange(const QString &product);

private:
	static void desstory();
	void LoadData();
	bool isExist(const QString &filtterSql);
	bool readAuthData(QAuthData * data);
	bool writeAuthData(QAuthData * data);
	void initErrorModel();
    void initConfig();

private:
	QMutex m_mutex;

	unsigned int m_errorCode;
	QErrorCode m_errCodeMap;
	bool m_imStop;

	QString m_user;
	int m_userLevel;

	QMap<QString,QVariant> m_params;
	QMap<QString,QVariant> m_paramUserData;
	QMap<QString,QVariant> m_data;
	QMap<QString,void*> m_ptrData;
	QErrorModel m_errModel;

    bool    m_bRunOffline = false;

private:
	static QSystem * _instance_;
	static bool _destoryed_;
};

#define System QSystem::instance()

#endif