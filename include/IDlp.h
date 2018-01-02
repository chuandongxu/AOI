#ifndef _DLP_INTERFACE_H_
#define _DLP_INTERFACE_H_

#include <QString>

class IDlp
{
public:
	virtual int getDLPNum() = 0;
	virtual bool startUpCapture(int indDLP) = 0;
	virtual bool endUpCapture(int indDLP) = 0;

	virtual bool isConnected(int indDLP) = 0;
	virtual bool trigger(int indDLP) = 0;
};

#endif