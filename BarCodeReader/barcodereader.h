#ifndef BARCODEREADER_H
#define BARCODEREADER_H

#include "../Common/modulebase.h"
#include "../include/IBarCodeReader.h"
#include "BarCodeDevMgr.h"

class BarCodeReader : public QModuleBase , public IBarCodeReader
{
public:
	BarCodeReader(int id,const QString &name);
	~BarCodeReader();

	virtual QString readBarcode(int devIdx);
	virtual void addSettingWiddget(QTabWidget * tabWidget);

private:
	QBarCodeDevMgr * m_pDevMgr;
};

#endif // BARCODEREADER_H
