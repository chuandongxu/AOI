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

	QWidget* getLightWidget();
    void startUpLight();
    void endUpLight();

	void setLuminance(int nChannel, int nLum);
	QString getChName(int nChannel);
	int getChLuminace(int nChannel);

	void saveLuminance(int nChannel);

	int getDeviceCount();
	virtual void addSettingWiddget(QTabWidget *tabWidget);
    //virtual void addDiagonseWidget(QTabWidget *tabWidget);


    virtual bool triggerCapturing(TRIGGER emTrig, bool bWaitDone, bool bClearSetupConfig) override;
    virtual void setupTrigger(ILight::TRIGGER emTrig) override;

private:
	void * m_devCtrl;
	LightWidget* m_pLightWidget;
};

#endif // LIGHTMODULE_H
