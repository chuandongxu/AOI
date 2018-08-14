#include "TowerLight.h"
#include "../common/SystemData.h"
#include "../Common/eos.h"
#include "../include/IdDefine.h"
#include "TowerSetting.h"

TowerLight::TowerLight(int id, const QString &name)
    :QModuleBase(id, name)
{
}

TowerLight::~TowerLight()
{
}

void TowerLight::addSettingWiddget(QTabWidget * tabWidget)
{
    QString user;
    int level = 0;
    System->getUser(user, level);
    if (USER_LEVEL_MANAGER > level) return;
    if (tabWidget)
    {
        tabWidget->addTab(new TowerSetting(&m_ctrl), QStringLiteral("三色灯设置"));
    }
}

bool TowerLight::triggerLight(bool bRed, bool bYellow, bool bGreen, bool bBuzzer)
{
    return m_ctrl.triggerLight(bRed ? 1 : 0, bYellow ? 1 : 0, bGreen ? 1 : 0, bBuzzer ? 1 : 0);
}

bool TowerLight::stopAllLight()
{
    return m_ctrl.stopLight();
}

bool TowerLight::setLightMode(LightMode mode)
{
    return m_ctrl.setLightMode(mode);
}

QMOUDLE_INSTANCE(TowerLight)
