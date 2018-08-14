#include "TowerCtrl.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IMotion.h"
#include "../include/constants.h"

TowerCtrl::TowerCtrl(QObject *parent)
    : QObject(parent)
{
}

TowerCtrl::~TowerCtrl()
{
}

bool TowerCtrl::triggerLight(int nRed, int nYellow, int nGreen, int nBuzzer)
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);

    //open red light or not
    int iState = 0;
    if (0 == nRed)
    {
        iState = 0;
    }
    else if (1 == nRed)
    {
        iState = 1;
    }
    else if (2 == nRed)
    {
        iState = 1;
    }
    if (!pMotion->setDO(DO_RED_LIGHT, iState)) return false;

    //open yellow light or not
    if (0 == nYellow)
    {
        iState = 0;
    }
    else if (1 == nYellow)
    {
        iState = 1;
    }
    else if (2 == nYellow)
    {
        iState = 1;
    }
    if (!pMotion->setDO(DO_YELLOW_LIGHT, iState)) return false;

    //open green light or not
    if (0 == nGreen)
    {
        iState = 0;
    }
    else if (1 == nGreen)
    {
        iState = 1;
    }
    else if (2 == nGreen)
    {
        iState = 1;
    }
    if (!pMotion->setDO(DO_GREEN_LIGHT, iState)) return false;

    //open buzzer or not
    if (0 == nBuzzer)
    {
        iState = 0;
    }
    else if (1 == nBuzzer)
    {
        iState = 1;
    }   
    if (!pMotion->setDO(DO_BUZZER, iState)) return false;

    return true;
}

bool TowerCtrl::setLightMode(LightMode mode)
{
    int nBgRed = 0, nBgYellow = 0, nBgGreen = 0, nBgBuzzer = 0;
    switch (mode)
    {
    case MODE_IDLE:
    {
        nBgRed = System->getParam("tower_light_idle_red").toInt();
        nBgYellow = System->getParam("tower_light_idle_yellow").toInt();
        nBgGreen = System->getParam("tower_light_idle_green").toInt();
        nBgBuzzer = System->getParam("tower_light_idle_buzzer").toInt();
        if (!triggerLight(nBgRed, nBgYellow, nBgGreen, nBgBuzzer)) return false;
    }
    break;
    case MODE_RUNNING:
    {
        nBgRed = System->getParam("tower_light_run_red").toInt();
        nBgYellow = System->getParam("tower_light_run_yellow").toInt();
        nBgGreen = System->getParam("tower_light_run_green").toInt();
        nBgBuzzer = System->getParam("tower_light_run_buzzer").toInt();
        if (!triggerLight(nBgRed, nBgYellow, nBgGreen, nBgBuzzer)) return false;
    }
    break;
    case MODE_WARNING:
    {
        nBgRed = System->getParam("tower_light_warn_red").toInt();
        nBgYellow = System->getParam("tower_light_warn_yellow").toInt();
        nBgGreen = System->getParam("tower_light_warn_green").toInt();
        nBgBuzzer = System->getParam("tower_light_warn_buzzer").toInt();
        if (!triggerLight(nBgRed, nBgYellow, nBgGreen, nBgBuzzer)) return false;
    }
    break;
    case MODE_ERROR:
    {
        nBgRed = System->getParam("tower_light_error_red").toInt();
        nBgYellow = System->getParam("tower_light_error_yellow").toInt();
        nBgGreen = System->getParam("tower_light_error_green").toInt();
        nBgBuzzer = System->getParam("tower_light_error_buzzer").toInt();
        if (!triggerLight(nBgRed, nBgYellow, nBgGreen, nBgBuzzer)) return false;
    }
    break;
    case MODE_NULL:
        break;
    default:
        break;
    }

    return true;
}

bool TowerCtrl::stopLight()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);

    QVector<int> nPorts;

    nPorts.push_back(DO_RED_LIGHT);
    nPorts.push_back(DO_YELLOW_LIGHT);
    nPorts.push_back(DO_GREEN_LIGHT);
    nPorts.push_back(DO_BUZZER);

    return pMotion->setDOs(nPorts, 0);
}
