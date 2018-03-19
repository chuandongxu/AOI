#ifndef LIGHTMODULE_H
#define LIGHTMODULE_H

#include "lightmodule_global.h"
#include "../Common/modulebase.h"
#include "../include/ILight.h"

class LightWidget;
class QLightModule  : public QModuleBase,public ILight
{
public:
	QLightModule(int id,const QString &name);
	~QLightModule();

	void init();

	void setLight(int nLight);
	QWidget* getLightWidget();

	void setLuminance(int nDevice, int nChannel, int nLum);
	QString getChName(int nDevice, int nChannel);
	int getChLuminace(int nDevice, int nChannel);

	int getDeviceCount();
	virtual void addSettingWiddget(QTabWidget *tabWidget);
    //virtual void addDiagonseWidget(QTabWidget *tabWidget);

private:
	void * m_devCtrl;
	LightWidget* m_pLightWidget;
};

#endif // LIGHTMODULE_H
