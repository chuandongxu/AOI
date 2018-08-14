#pragma once

#include "towerlight_global.h"

#include "../Common/modulebase.h"
#include "../include/ITowerLight.h"
#include "TowerCtrl.h"

class TowerLight : public QModuleBase, public ITowerLight
{
public:
    TowerLight(int id, const QString &name);
    ~TowerLight();

    virtual void addSettingWiddget(QTabWidget * tabWidget);

public:
    // General Functions:
    virtual bool triggerLight(bool bRed, bool bYellow, bool bGreen, bool bBuzzer) override;
    virtual bool stopAllLight() override;
    virtual bool setLightMode(LightMode mode = MODE_IDLE) override;

private:
    TowerCtrl m_ctrl;
};
