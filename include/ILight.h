#ifndef _I_LIGHT_H_
#define _I_LIGHT_H_

class ILight
{
public:
    enum TRIGGER
    {
        TRIGGER_ALL,
        TRIGGER_DLP,
        TRIGGER_DLP1,
        TRIGGER_DLP2,
        TRIGGER_DLP3,
        TRIGGER_DLP4,
        TRIGGER_LIGHT,
        TRIGGER_ONE_CH1,
        TRIGGER_ONE_CH2,
        TRIGGER_ONE_CH3,
        TRIGGER_ONE_CH4,
        TRIGGER_ONE_CH5,
        TRIGGER_ONE_CH6
    };

public:
    virtual void init() = 0;

	virtual QWidget* getLightWidget() = 0;
    virtual void startUpLight() = 0;
    virtual void endUpLight() = 0;

	virtual void setLuminance(int nChannel, int nLum) = 0;
	virtual QString getChName(int nChannel) = 0;

	virtual int getChLuminace(int nChannel) = 0;
	virtual void saveLuminance(int nChannel) = 0;

    // light card trigger functions
    virtual bool triggerCapturing(TRIGGER emTrig, bool bWaitDone, bool bClearSetupConfig = false) = 0;
};

#endif