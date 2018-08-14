#pragma once

#include <QObject>

#include "../include/ITowerLight.h"

enum FlashMode
{   
    MODE_ON,
    MODE_OFF,
    MODE_FLASHING
};

class TowerCtrl : public QObject
{
    Q_OBJECT

public:
    TowerCtrl(QObject *parent = NULL);
    ~TowerCtrl();

    bool triggerLight(int nRed, int nYellow, int nGreen, int nBuzzer);
    bool setLightMode(LightMode mode);
    bool stopLight();
};
