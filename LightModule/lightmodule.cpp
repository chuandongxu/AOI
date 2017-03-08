#include "lightmodule.h"
#include "lightsetting.h"
#include "lightctrl.h"
#include "LightDefine.h"
#include "..\Common\SystemData.h"
QLightModule::QLightModule(int id,const QString &name)
	:QModuleBase(id,name)
	,m_devCtrl(NULL)
{
	QLightCtrl *ctrl = new QLightCtrl(NULL);
	if(ctrl)
	{
		m_devCtrl = (void*)ctrl;
	}
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
}

void QLightModule::setLuminance(int nDevice, int nChannel, int nLum)
{
	QLightCtrl *ctrl = (QLightCtrl*)m_devCtrl;
	
	if (ctrl)
	{
		QLightDevice *device = ctrl->getLightDevice(nDevice);
		if(device)
		{
			device->setChLuminance(nChannel,nLum);
		}
		
	}
}

QString QLightModule::getChName(int nDevice, int nChannel)
{
	QLightCtrl *ctrl = (QLightCtrl*)m_devCtrl;
	
	if (ctrl)
	{
		
		QString key;
		QVariant data;
		QLightDevice *device = ctrl->getLightDevice(nDevice);
		if(device)
		{
			key = QString("%0-%1%2").arg(device->getDeviceName()).arg(NAMED_CH).arg(nChannel);
			data = System->getParam(key);
			return data.toString();
		}
		
	}

	return "";
}
int QLightModule::getChLuminace(int nDevice, int nChannel)
{
	QLightCtrl *ctrl = (QLightCtrl*)m_devCtrl;
	
	if (ctrl)
	{
		
		QString key;
		QVariant data;
		QLightDevice *device = ctrl->getLightDevice(nDevice);
		if(device)
		{
			key = QString("%0-%1%2").arg(device->getDeviceName()).arg(LUM_CH).arg(nChannel);
			data = System->getParam(key);
			return data.toInt();
		}
		
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
		tabWidget->addTab(new QLightSetting(ctrl),QStringLiteral("µÆ¹â¿ØÖÆ"));
	}
}

QMOUDLE_INSTANCE(QLightModule)