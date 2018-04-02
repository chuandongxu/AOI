#include "MotionModule.h"
#include "../common/SystemData.h"
#include "MotionSetting.h"

MotionModule::MotionModule(int id, const QString &name)
	:QModuleBase(id, name)
{
}

MotionModule::~MotionModule()
{
}

void MotionModule::addSettingWiddget(QTabWidget * tabWidget)
{
	QString user;
	int level = 0;
	System->getUser(user, level);
	if (USER_LEVEL_MANAGER > level) return;
	if (tabWidget)
	{
		tabWidget->addTab(new MotionSetting(&m_ctrl), QStringLiteral("Motion设置"));
	}
}

bool MotionModule::init()
{
	return m_ctrl.init();
}

void MotionModule::unInit()
{
	m_ctrl.unInit();
}

bool MotionModule::reset()
{
	return m_ctrl.reset();
}

void MotionModule::clearAllError()
{
	m_ctrl.clearAllError();
}

bool MotionModule::IsPowerError()
{
	return m_ctrl.IsPowerError();
}

bool MotionModule::setDOs(QVector<int>& nPorts, int iState)
{
	return m_ctrl.setDOs(nPorts, iState);
}

bool MotionModule::setDO(int nPort, int iState)
{
	return m_ctrl.setDO(nPort, iState);
}

bool MotionModule::getDO(int nPort, int &iState)
{
	return m_ctrl.getDO(nPort, iState);
}

bool MotionModule::getDI(int nPort, int &iState)
{
	return m_ctrl.getDI(nPort, iState);
}

bool MotionModule::triggerCapturing(TRIGGER emTrig, bool bWaitDone, bool bClearSetupConfig)
{
	return m_ctrl.triggerCapturing(emTrig, bWaitDone, bClearSetupConfig);
}

bool MotionModule::enable(int AxisID)
{
	return m_ctrl.enable(AxisID);
}

bool MotionModule::disable(int AxisID)
{
	return m_ctrl.disable(AxisID);
}

bool MotionModule::enableAllAxis()
{
	return m_ctrl.enableAllAxis();
}

bool MotionModule::isEnabled(int AxisID)
{
	return m_ctrl.isEnabled(AxisID);
}

bool MotionModule::IsError(int AxisID)
{
	return m_ctrl.IsError(AxisID);
}

bool MotionModule::IsEMStopError(int AxisID)
{
	return m_ctrl.IsEMStopError(AxisID);
}

bool MotionModule::IsLimit(int AxisID)
{
	return m_ctrl.IsLimit(AxisID);
}

bool MotionModule::homeAll(bool bSyn)
{
	return m_ctrl.homeAll(bSyn);
}

bool MotionModule::home(int AxisID, bool bSyn)
{
	return m_ctrl.homeLimit(AxisID, bSyn);
}

bool MotionModule::move(int AxisID, int nProfile, double dDist, bool bSyn)
{
	return m_ctrl.move(AxisID, nProfile, dDist, bSyn);
}

bool MotionModule::moveTo(int AxisID, int nProfile, double dPos, bool bSyn)
{
	return m_ctrl.moveTo(AxisID, nProfile, dPos, bSyn);
}

bool MotionModule::movePos(int nPointTable, bool bSyn)
{
	return m_ctrl.movePos(nPointTable, bSyn);
}

bool MotionModule::moveToPos(int nPointTable, bool bSyn)
{
	return m_ctrl.moveToPos(nPointTable, bSyn);
}

bool MotionModule::movePosGroup(int nPtGroup, bool bSyn)
{
	return m_ctrl.movePosGroup(nPtGroup, bSyn);
}

bool MotionModule::moveToPosGroup(int nPtGroup, bool bSyn)
{
	return m_ctrl.movePosGroup(nPtGroup, bSyn);
}

bool MotionModule::moveToGroup(std::vector<int>& axis, std::vector<double>& pos, std::vector<int>& profiles, bool bSyn)
{
	return m_ctrl.moveToGroup(axis, pos, profiles, bSyn);
}

bool MotionModule::moveGroup(std::vector<int>& axis, std::vector<double>& dists, std::vector<int>& profiles, bool bSyn)
{
	return m_ctrl.moveGroup(axis, dists, profiles, bSyn);
}

bool MotionModule::waitDone()
{
	return m_ctrl.waitDone();
}

bool MotionModule::isHomed(int AxisID)
{
	return m_ctrl.isHomed(AxisID);
}

bool MotionModule::isMoveDone(int AxisID)
{
	return m_ctrl.isMoveDone(AxisID);
}

bool MotionModule::stopMove(int AxisID)
{
	return m_ctrl.stopMove(AxisID);
}

bool MotionModule::EmStop(int AxisID)
{
	return m_ctrl.EmStop(AxisID);
}

bool MotionModule::getCurrentPos(int AxisID, double *pos)
{
	return m_ctrl.getCurrentPos(AxisID, pos);
}

QMOUDLE_INSTANCE(MotionModule)
