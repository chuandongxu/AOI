#ifndef MODULEBASE_H
#define MODULEBASE_H

#include "../include/Module.h"
#include "Common_global.h"

class COMMON_EXPORT QModuleBase : public QModuleInterface
{
public:
    QModuleBase(int id,const QString &name);
	virtual ~QModuleBase();

    virtual QString getDisplayName();
	//virtual void init(){};
	//virtual void unInit(){};
    virtual int getId();	

    virtual void insertEventObserver(QHalEventObserver * pObs);
    virtual void sendNotifyHalEvent(const QHalEvent &ev);

    virtual void * getHal(){return NULL;};

    virtual void addSettingWiddget(QTabWidget * tabWidget){};
    virtual void freeSettingWidget(QTabWidget * tabWidget){};
	virtual void showSettingWidget(){};
	virtual void showDiagonseWidget(){};

    virtual void addDiagonseWidget(QTabWidget * tabWidget){};
    virtual void freeDiagonseWidget(QTabWidget * tabWidget){};

private:
    int m_id;
    QString m_name;
    QList<QHalEventObserver*> m_obs;
};

#endif // MODULEBASE_H
