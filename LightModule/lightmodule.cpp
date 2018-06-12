#include "lightmodule.h"
#include "lightsetting.h"
#include "lightctrl.h"
#include "LightDefine.h"
#include "..\Common\SystemData.h"
#include "LightWidget.h"

QLightModule::QLightModule(int id,const QString &name)
	:QModuleBase(id,name)
	,m_devCtrl(NULL)
{
	QLightCtrl *ctrl = new QLightCtrl(NULL);
	if(ctrl)
	{
		m_devCtrl = (void*)ctrl;
	}
	m_pLightWidget = new LightWidget(ctrl);
}

QLightModule::~QLightModule()
{

}

void QLightModule::init()
{
	QLightCtrl *ctrl = (QLightCtrl*)m_devCtrl;
	if (ctrl)
	{
		ctrl->init();
	}

    if (m_pLightWidget)
    {
        m_pLightWidget->init();
    }
}

QWidget* QLightModule::getLightWidget()
{
	return m_pLightWidget;
}

void QLightModule::startUpLight()
{
    if (m_pLightWidget)
    {
        m_pLightWidget->startUpLight();
    }
}

void QLightModule::endUpLight()
{
    if (m_pLightWidget)
    {
        m_pLightWidget->endUpLight();
    }
}

void QLightModule::saveLuminance(int nChannel)
{
	QLightCtrl *ctrl = (QLightCtrl*)m_devCtrl;

	if (ctrl)
	{
        ctrl->saveLuminance(nChannel);
	}
}

void QLightModule::setLuminance(int nChannel, int nLum)
{
	QLightCtrl *ctrl = (QLightCtrl*)m_devCtrl;
	
	if (ctrl)
	{
        ctrl->setLuminance(nChannel, nLum);
	}
}

QString QLightModule::getChName(int nChannel)
{
	QLightCtrl *ctrl = (QLightCtrl*)m_devCtrl;
	
	if (ctrl)
	{		
        return ctrl->getChName(nChannel);
	}

	return "";
}

int QLightModule::getChLuminace(int nChannel)
{
	QLightCtrl *ctrl = (QLightCtrl*)m_devCtrl;
	
	if (ctrl)
	{	
        return ctrl->getChLuminace(nChannel);
	}

	return 0;
}

int QLightModule::getDeviceCount()
{
	QLightCtrl *ctrl = (QLightCtrl*)m_devCtrl;
	if (ctrl)
	{
		return ctrl->getDeviceCount();
	}

	return 0;
}

void QLightModule::addSettingWiddget(QTabWidget *tabWidget)
{
	QLightCtrl *ctrl = (QLightCtrl*)m_devCtrl;
	if(ctrl)
	{
		tabWidget->addTab(new QLightSetting(ctrl), QStringLiteral("灯光控制"));
	}
}

bool QLightModule::triggerCapturing(TRIGGER emTrig, bool bWaitDone, bool bClearSetupConfig)
{
    QLightCtrl *ctrl = (QLightCtrl*)m_devCtrl;
    if (ctrl)
    {
        return ctrl->triggerCapturing(emTrig, bWaitDone, bClearSetupConfig);
    }

    return false;
}


QMOUDLE_INSTANCE(QLightModule)