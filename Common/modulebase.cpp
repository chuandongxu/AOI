#include "modulebase.h"
#include "ModuleMgr.h"

QModuleBase::QModuleBase(int id,const QString &name)
    :m_id(id),m_name(name)
{
	QMoudleMgr * pMgr = QMoudleMgr::instance();
	if(pMgr)pMgr->addModlue(this);
}

QModuleBase::~QModuleBase()
{
	QMoudleMgr * pMgr = QMoudleMgr::instance();
	if(pMgr)pMgr->removeModlue(this);
}

QString QModuleBase::getDisplayName()
{
    return m_name;
}

int QModuleBase::getId()
{
    return m_id;
}

void QModuleBase::insertEventObserver(QHalEventObserver * pObs)
{
    if(m_obs.contains(pObs))return;
    m_obs.append(pObs);
}

void QModuleBase::sendNotifyHalEvent(const QHalEvent &ev)
{
    int n = m_obs.size();
    for(int i = 0; i< n; i++)
    {
        m_obs[i]->onEvent(ev);
    }
}
