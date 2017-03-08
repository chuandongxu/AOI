#ifndef _BAR_CODE_READER_H_
#define _BAR_CODE_READER_H_

#include <QString>

class IBarCodeReader
{
public:
	virtual QString readBarcode(int devIdx) = 0;
};

#endif