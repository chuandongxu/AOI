#ifndef BARCODEDEVMGR_H
#define BARCODEDEVMGR_H

#include <QObject>
#include <qlist.h>
#include "BarCodeDevice.h"

class QBarCodeDevMgr : public QObject
{
	Q_OBJECT
		
	typedef QList<QBarCodeDevice*> QBarCodeDevList;

public:
	QBarCodeDevMgr(QObject *parent);
	~QBarCodeDevMgr();

	QBarCodeDevice * getDevice(int n);
	QBarCodeDevice * addDevice(const QString &name);

	void removeDevice(int n);

	int getDevcount();

protected:
	void loadDevice();

private:
	QBarCodeDevList m_devList;
};

#endif // BARCODEDEVMGR_H
