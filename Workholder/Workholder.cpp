#include "Workholder.h"
#include "../common/SystemData.h"
#include "../Common/eos.h"
#include "../include/IdDefine.h"
#include "WorkSetting.h"

Workholder::Workholder(int id, const QString &name)
    :QModuleBase(id, name)
{  
}

Workholder::~Workholder()
{
}

void Workholder::addSettingWiddget(QTabWidget * tabWidget)
{
    QString user;
    int level = 0;
    System->getUser(user, level);
    if (USER_LEVEL_MANAGER > level) return;
    if (tabWidget)
    {
        tabWidget->addTab(new WorkSetting(&m_ctrl), QStringLiteral("上下料设置"));
    }
}

bool Workholder::IsBoardArrived()
{
    return m_ctrl.IsBoardArrived();
}

bool Workholder::IsBoardDelivered()
{
    return m_ctrl.IsBoardDelivered();
}

bool Workholder::IsBoardReady()
{
    return m_ctrl.IsBoardReady();
}

bool Workholder::pullBoard(bool bWait)
{
    return m_ctrl.moveInput(bWait);
}

bool Workholder::pushBoard(bool bWait)
{
    return m_ctrl.moveOutput(bWait);
}

QMOUDLE_INSTANCE(Workholder)