#include "ThreadPrioc.h"


QAutoLocker::QAutoLocker(QMutex * pMutex,bool AutoLock)
    :m_locker(pMutex)
{
    if(AutoLock && m_locker)m_locker->lock();
}


QAutoLocker::~QAutoLocker(void)
{
    if(m_locker)m_locker->unlock();
}

void QAutoLocker::Lock()
{
    if(m_locker)m_locker->lock();
}

void QAutoLocker::unLock()
{
    if(m_locker)m_locker->unlock();
}