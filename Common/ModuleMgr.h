#ifndef _COMMON_H_
#define _COMMON_H_

#include <QMap>
#include "../include/Module.h"
#include "Common_global.h"
#include <QLibrary>


class COMMON_EXPORT QMoudleMgr : public QObject
{
    Q_OBJECT

protected:
    QMoudleMgr();

public:
    ~QMoudleMgr();

    static QMoudleMgr * instance();
	static QModuleInterface * loadPlugin(const QString &dllfile,int id,const QString &name);
	static void removePlugin(int id);
	static void loadPlugins();
	static void removePlugins();
	

    QList<int> getModelIdList();
    QModuleInterface * getModule(int id);
	QModuleInterface * getMainModule();
    void addModlue(QModuleInterface * pMoudle);
    void removeModlue(QModuleInterface * pMoudle);

	QModuleInterface * loadModule(const QString &dllfile,int id,const QString &name);
	void freeModule(int id);
	void freeModules();
protected:
    static void destory();

private:
    QMap<int,QModuleInterface *> m_moduleList;
	QMap<int,QLibrary*> m_moduleLib;

    static QMoudleMgr * _instance_;
    static bool _bDestoryed_;
	static int _mainModuleId_;
};

template<typename T>
T * getModule(int moduleId){
	QMoudleMgr * p = QMoudleMgr::instance();
	if(p)
	{
		return dynamic_cast<T*>(p->getModule(moduleId));
	}
	return NULL;
}

template<typename T>
T * getMainModule(){
	QMoudleMgr * p = QMoudleMgr::instance();
	if(p)
	{
		return dynamic_cast<T*>(p->getMainModule());
	}
	return NULL;
}

#endif
