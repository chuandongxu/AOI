#include "DLPControlModule.h"
#include "../common/SystemData.h"
#include "DLPControlModuleSetting.h"

DLPControlModule::DLPControlModule(int id, const QString &name)
	:QModuleBase(id, name)
{
	
}

DLPControlModule::~DLPControlModule()
{
	
}

void DLPControlModule::addSettingWiddget(QTabWidget * tabWidget)
{
	if (tabWidget)
	{
		tabWidget->addTab(new DLPControlModuleSetting(&m_ctrl), QStringLiteral("DLP设置"));
	}

	QString user;
	int level = 0;
	System->getUser(user, level);
	if (USER_LEVEL_MANAGER > level)
	{
		//tabWidget->setEnabled(false);
	}
}

int DLPControlModule::getDLPNum()
{
	return m_ctrl.getDLPNum();
}

bool DLPControlModule::startUpCapture(int indDLP)
{
	return m_ctrl.startUpCapture(indDLP);
}

bool DLPControlModule::endUpCapture(int indDLP)
{
	return m_ctrl.endUpCapture(indDLP);
}

bool DLPControlModule::isConnected(int indDLP)
{
	return m_ctrl.isConnected(indDLP);
}

bool DLPControlModule::trigger(int indDLP)
{
	return m_ctrl.trigger(indDLP);
}
	

QMOUDLE_INSTANCE(DLPControlModule)
