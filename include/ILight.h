#ifndef _I_LIGHT_H_
#define _I_LIGHT_H_

class ILight
{
public:
	virtual void setLuminance(int nDevice, int nChannel, int nLum) = 0;
	virtual QString getChName(int nDevice, int nChannel) = 0;
	virtual int getChLuminace(int nDevice, int nChannel) = 0;
};

#endif