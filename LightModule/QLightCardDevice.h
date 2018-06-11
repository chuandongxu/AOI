#pragma once

#include <QObject>

#include "lightmodule_global.h"

#include "../include/ILight.h"

#include "lightdevice.h"

class QLightCardDevice : public QLightDevice
{
    Q_OBJECT
public:
    QLightCardDevice(const QString & devName, int nChnNum, QObject *parent);
    ~QLightCardDevice();

    virtual void setChLuminance(int ch, int luminance) override;
    virtual void setupTrigger(ILight::TRIGGER emTrig) override;
    virtual bool trigger() override;

private:
    void writeCmd(const QString& szCmd);
    int getPatternNum();

private:
    ILight::TRIGGER m_emTrig;

    bool m_bSetChn;
};
