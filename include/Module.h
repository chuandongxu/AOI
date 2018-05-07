#ifndef _MODEL_H_
#define _MODEL_H_

#include <QString>
#include <QWidget>
#include <QTabWidget>

struct QHalEvent
{
    int moudleId;
    QString moduleName;
    int event;
    int wParam;
    unsigned long lParam;
};

class QHalEventObserver
{
public:
    virtual void onEvent(const QHalEvent & ev) = 0;
};

class QModuleInterface
{
public:
    virtual QString getDisplayName() = 0;
    virtual int getId() = 0;

	//virtual void init() = 0;
	//virtual void unInit() = 0;

    virtual void * getHal() = 0;

    virtual void addSettingWiddget(QTabWidget * tabWidget) = 0;
    virtual void freeSettingWidget(QTabWidget * tabWidget) = 0;

    virtual void addDiagonseWidget(QTabWidget * tabWidget) = 0;
    virtual void freeDiagonseWidget(QTabWidget * tabWidget) = 0;

	virtual void preStartUp() = 0;

    virtual void insertEventObserver(QHalEventObserver * pObs) = 0;
    virtual void sendNotifyHalEvent(const QHalEvent &ev) = 0;
};

#define QMOUDLE_INSTANCE(type) \
extern "C"{ \
   Q_DECL_EXPORT QModuleInterface * createModule(int id,const QString &name){return new type(id,name);}\
   Q_DECL_EXPORT void destoryModule(QModuleInterface * p){type * tem = (type*)p;if(tem)delete tem;}\
};

#endif
