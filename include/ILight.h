#ifndef _I_LIGHT_H_
#define _I_LIGHT_H_

class ILight
{
public:
    virtual void init() = 0;

	virtual QWidget* getLightWidget() = 0;
    virtual void startUpLight() = 0;
    virtual void endUpLight() = 0;

	virtual void setLuminance(int nChannel, int nLum) = 0;
	virtual QString getChName(int nChannel) = 0;

	virtual int getChLuminace(int nChannel) = 0;
	virtual void saveLuminance(int nChannel) = 0;
};

#endif