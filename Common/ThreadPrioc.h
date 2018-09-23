#pragma once
#include "Common_global.h"

#include <QMutex>

class COMMON_EXPORT QAutoLocker
{
public:
    QAutoLocker(QMutex * pMutex,bool AutoLock = true);
    ~QAutoLocker(void);

    void Lock();
    void unLock();
private:
    QMutex *m_locker;
};

