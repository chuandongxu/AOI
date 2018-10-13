#include "MotionModule.h"
#include "../common/SystemData.h"
#include "MotionSetting.h"
#include "JoystickWidget.h"
#include "ConfigData.h"

MotionModule::MotionModule(int id, const QString &name)
    :QModuleBase(id, name)
{
    m_pJoystickWidget = std::make_unique<JoystickWidget>(&m_ctrl);
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

bool MotionModule::setDOs(QVector<QString>& szPorts, int iState)
{
    QVector<int> nPorts;
    for each (auto port in szPorts)
    {
        nPorts.push_back(Config->ID(port));
    }
    return m_ctrl.setDOs(nPorts, iState);
}

bool MotionModule::setDO(const QString& szPort, int iState)
{
    return m_ctrl.setDO(Config->ID(szPort), iState);
}

bool MotionModule::getDO(const QString& szPort, int &iState)
{
    return m_ctrl.getDO(Config->ID(szPort), iState);
}

bool MotionModule::getDI(const QString& szPort, int &iState)
{
    return m_ctrl.getDI(Config->ID(szPort), iState);
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

bool MotionModule::enable(const QString& AxisID)
{
    return m_ctrl.enable(Config->ID(AxisID));
}

bool MotionModule::disable(const QString& AxisID)
{
    return m_ctrl.disable(Config->ID(AxisID));
}

bool MotionModule::isEnabled(const QString& AxisID)
{
    return m_ctrl.isEnabled(Config->ID(AxisID));
}

bool MotionModule::IsError(const QString& AxisID)
{
    return m_ctrl.IsError(Config->ID(AxisID));
}

bool MotionModule::IsEMStopError(const QString& AxisID)
{
    return m_ctrl.IsEMStopError(Config->ID(AxisID));
}

bool MotionModule::IsLimit(const QString& AxisID)
{
    return m_ctrl.IsLimit(Config->ID(AxisID));
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

bool MotionModule::home(const QString& AxisID, bool bSyn)
{
    return m_ctrl.homeLimit(Config->ID(AxisID), bSyn);
}

bool MotionModule::move(const QString& AxisID, int nProfile, double dDist, bool bSyn)
{
    return m_ctrl.move(Config->ID(AxisID), nProfile, dDist, bSyn);
}

bool MotionModule::moveTo(const QString& AxisID, int nProfile, double dPos, bool bSyn)
{
    return m_ctrl.moveTo(Config->ID(AxisID), nProfile, dPos, bSyn);
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

bool MotionModule::moveToGroup(std::vector<QString>& axis, std::vector<double>& pos, std::vector<int>& profiles, bool bSyn)
{
    std::vector<int> axisIDs;
    for each (auto axItem in axis)
    {
        axisIDs.push_back(Config->ID(axItem));
    }
    return m_ctrl.moveToGroup(axisIDs, pos, profiles, bSyn);
}

bool MotionModule::moveGroup(std::vector<QString>& axis, std::vector<double>& dists, std::vector<int>& profiles, bool bSyn)
{
    std::vector<int> axisIDs;
    for each (auto axItem in axis)
    {
        axisIDs.push_back(Config->ID(axItem));
    }
    return m_ctrl.moveGroup(axisIDs, dists, profiles, bSyn);
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

bool MotionModule::isHomed(const QString& AxisID)
{
    return m_ctrl.isHomed(Config->ID(AxisID));
}

bool MotionModule::isMoveDone(const QString& AxisID)
{
    return m_ctrl.isMoveDone(Config->ID(AxisID));
}

bool MotionModule::stopMove(const QString& AxisID)
{
    return m_ctrl.stopMove(Config->ID(AxisID));
}

bool MotionModule::EmStop(const QString& AxisID)
{
    return m_ctrl.EmStop(Config->ID(AxisID));
}

bool MotionModule::getCurrentPos(const QString& AxisID, double *pos)
{
    return m_ctrl.getCurrentPos(Config->ID(AxisID), pos);
}

void MotionModule::startJoystick()
{
    m_pJoystickWidget->show();
}

void MotionModule::setJoystickXMotor(int AxisID, double dStep, QLineEdit *pEdit)
{
    m_pJoystickWidget->SetXMotor(AxisID, dStep, pEdit);
}

void MotionModule::setJoystickYMotor(int AxisID, double dStep, QLineEdit *pEdit)
{
    m_pJoystickWidget->SetYMotor(AxisID, dStep, pEdit);
}

void MotionModule::setJoystickXMotor(const QString& AxisID, double dStep, QLineEdit *pEdit)
{
    m_pJoystickWidget->SetXMotor(Config->ID(AxisID), dStep, pEdit);
}

void MotionModule::setJoystickYMotor(const QString& AxisID, double dStep, QLineEdit *pEdit)
{
    m_pJoystickWidget->SetYMotor(Config->ID(AxisID), dStep, pEdit);
}

QWidget *MotionModule::getJoyStickWidget()
{
    return m_pJoystickWidget.get();
}

QMOUDLE_INSTANCE(MotionModule)
