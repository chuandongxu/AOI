#pragma once

#include "motionmodule_global.h"

#include "../Common/modulebase.h"
#include "../include/IMotion.h"
#include "Motioncontrol.h"

#include <memory>

class JoystickWidget;
class MotionModule : public QModuleBase, public IMotion
{
public:
    MotionModule(int id, const QString &name);
    ~MotionModule();

    virtual void addSettingWiddget(QTabWidget * tabWidget);

    // General Functions:
    virtual bool init();
    virtual void unInit();
    virtual bool reset();
    virtual void clearAllError();
    virtual bool IsPowerError();

    // IO Functions:
    virtual bool setDOs(QVector<int>& nPorts, int iState);
    virtual bool setDO(int nPort, int iState);
    virtual bool getDO(int nPort, int &iState);
    virtual bool getDI(int nPort, int &iState);

    virtual bool setDOs(QVector<QString>& szPorts, int iState);
    virtual bool setDO(const QString& szPort, int iState);
    virtual bool getDO(const QString& szPort, int &iState);
    virtual bool getDI(const QString& szPort, int &iState);

    // Motor Functions:
    virtual bool enable(int AxisID);
    virtual bool disable(int AxisID);
    virtual bool enableAllAxis();
    virtual bool isEnabled(int AxisID);
    virtual bool IsError(int AxisID);
    virtual bool IsEMStopError(int AxisID);
    virtual bool IsLimit(int AxisID);

    virtual bool enable(const QString& AxisID);
    virtual bool disable(const QString& AxisID);
    virtual bool isEnabled(const QString& AxisID);
    virtual bool IsError(const QString& AxisID);
    virtual bool IsEMStopError(const QString& AxisID);
    virtual bool IsLimit(const QString& AxisID);

    virtual bool homeAll(bool bSyn);
    virtual bool home(int AxisID, bool bSyn);
    virtual bool move(int AxisID, int nProfile, double dDist, bool bSyn);
    virtual bool moveTo(int AxisID, int nProfile, double dPos, bool bSyn);

    virtual bool home(const QString& AxisID, bool bSyn);
    virtual bool move(const QString& AxisID, int nProfile, double dDist, bool bSyn);
    virtual bool moveTo(const QString& AxisID, int nProfile, double dPos, bool bSyn);

    virtual bool movePos(int nPointTable, bool bSyn);
    virtual bool moveToPos(int nPointTable, bool bSyn);

    virtual bool movePosGroup(int nPtGroup, bool bSyn);
    virtual bool moveToPosGroup(int nPtGroup, bool bSyn);

    virtual bool moveToGroup(std::vector<int>& axis, std::vector<double>& pos, std::vector<int>& profiles, bool bSyn);
    virtual bool moveGroup(std::vector<int>& axis, std::vector<double>& dists, std::vector<int>& profiles, bool bSyn);

    virtual bool moveToGroup(std::vector<QString>& axis, std::vector<double>& pos, std::vector<int>& profiles, bool bSyn);
    virtual bool moveGroup(std::vector<QString>& axis, std::vector<double>& dists, std::vector<int>& profiles, bool bSyn);

    virtual bool waitDone();

    virtual bool isHomed(int AxisID);
    virtual bool isMoveDone(int AxisID);
    virtual bool stopMove(int AxisID);
    virtual bool EmStop(int AxisID);

    virtual bool isHomed(const QString& AxisID);
    virtual bool isMoveDone(const QString& AxisID);
    virtual bool stopMove(const QString& AxisID);
    virtual bool EmStop(const QString& AxisID);

    virtual bool getCurrentPos(int AxisID, double *pos);
    virtual bool getCurrentPos(const QString& AxisID, double *pos);

    virtual void startJoystick() override;
    virtual void setJoystickXMotor(int AxisID, double dStep, QLineEdit *pEdit) override;
    virtual void setJoystickYMotor(int AxisID, double dStep, QLineEdit *pEdit) override;
    virtual void setJoystickXMotor(const QString& AxisID, double dStep, QLineEdit *pEdit) override;
    virtual void setJoystickYMotor(const QString& AxisID, double dStep, QLineEdit *pEdit) override;
    virtual QWidget *getJoyStickWidget() override;
private:
    MotionControl m_ctrl;
    std::unique_ptr<JoystickWidget> m_pJoystickWidget;
};
