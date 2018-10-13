#ifndef _MOTION_H_
#define _MOTION_H_

#define MOTION_STATE_PE 0x00001
#define MOTION_STATE_ORG 0x00010
#define MOTION_STATE_EML 0x00001
#define MOTION_STATE_ALM 0x00001
#define MOTION_STATE_EMG 0x00001

#define HOME_DIR_PDR  1
#define HOME_DIR_NDR  -1

#include <qlineedit.h>

class IMotion
{
public:
    enum STATE
    {
        STATE_NO_INIT,
        STATE_PE,
        STATE_ORG,
        STATE_EML,
        STATE_ALM,
        STATE_EMG
    };
   
public:
    // General Functions:
    virtual bool init() = 0;
    virtual void unInit() = 0;
    virtual bool reset() = 0;
    virtual void clearAllError() = 0;
    virtual bool IsPowerError() = 0;

    // IO Functions:
    virtual bool setDOs(QVector<int>& nPorts, int iState) = 0;
    virtual bool setDO(int nPort, int iState) = 0;// nPort 0 : n-1
    virtual bool getDO(int nPort, int &iState) = 0;
    virtual bool getDI(int nPort, int &iState) = 0;

    virtual bool setDOs(QVector<QString>& szPorts, int iState) = 0;
    virtual bool setDO(const QString& szPort, int iState) = 0;// nPort 0 : n-1
    virtual bool getDO(const QString& szPort, int &iState) = 0;
    virtual bool getDI(const QString& szPort, int &iState) = 0;   

    // Motor Functions:
    virtual bool enable(int AxisID) = 0;
    virtual bool disable(int AxisID) = 0;
    virtual bool enableAllAxis() = 0;
    virtual bool isEnabled(int AxisID) = 0;
    virtual bool IsError(int AxisID) = 0;
    virtual bool IsEMStopError(int AxisID) = 0;
    virtual bool IsLimit(int AxisID) = 0;

    virtual bool enable(const QString& AxisID) = 0;
    virtual bool disable(const QString& AxisID) = 0;
    virtual bool isEnabled(const QString& AxisID) = 0;
    virtual bool IsError(const QString& AxisID) = 0;
    virtual bool IsEMStopError(const QString& AxisID) = 0;
    virtual bool IsLimit(const QString& AxisID) = 0;

    virtual bool homeAll(bool bSyn) = 0;
    virtual bool home(int AxisID, bool bSyn) = 0;
    virtual bool move(int AxisID, int nProfile, double dDist, bool bSyn) = 0;
    virtual bool moveTo(int AxisID, int nProfile, double dPos, bool bSyn) = 0;

    virtual bool home(const QString& AxisID, bool bSyn) = 0;
    virtual bool move(const QString& AxisID, int nProfile, double dDist, bool bSyn) = 0;
    virtual bool moveTo(const QString& AxisID, int nProfile, double dPos, bool bSyn) = 0;

    virtual bool movePos(int nPointTable, bool bSyn) = 0;
    virtual bool moveToPos(int nPointTable, bool bSyn) = 0;

    virtual bool movePosGroup(int nPtGroup, bool bSyn) = 0;
    virtual bool moveToPosGroup(int nPtGroup, bool bSyn) = 0;

    virtual bool moveToGroup(std::vector<int>& axis, std::vector<double>& pos, std::vector<int>& profiles, bool bSyn) = 0;
    virtual bool moveGroup(std::vector<int>& axis, std::vector<double>& dists, std::vector<int>& profiles, bool bSyn) = 0;

    virtual bool moveToGroup(std::vector<QString>& axis, std::vector<double>& pos, std::vector<int>& profiles, bool bSyn) = 0;
    virtual bool moveGroup(std::vector<QString>& axis, std::vector<double>& dists, std::vector<int>& profiles, bool bSyn) = 0;

    virtual bool waitDone() = 0;

    virtual bool isHomed(int AxisID) = 0;
    virtual bool isMoveDone(int AxisID) = 0;
    virtual bool stopMove(int AxisID) = 0;
    virtual bool EmStop(int AxisID) = 0;

    virtual bool isHomed(const QString& AxisID) = 0;
    virtual bool isMoveDone(const QString& AxisID) = 0;
    virtual bool stopMove(const QString& AxisID) = 0;
    virtual bool EmStop(const QString& AxisID) = 0;

    virtual bool getCurrentPos(int AxisID, double *pos) = 0;
    virtual bool getCurrentPos(const QString& AxisID, double *pos) = 0;
    
    virtual void setJoystickXMotor(int AxisID, double dStep, QLineEdit *pEdit = NULL) = 0;
    virtual void setJoystickYMotor(int AxisID, double dStep, QLineEdit *pEdit = NULL) = 0;

    virtual void setJoystickXMotor(const QString& AxisID, double dStep, QLineEdit *pEdit = NULL) = 0;
    virtual void setJoystickYMotor(const QString& AxisID, double dStep, QLineEdit *pEdit = NULL) = 0;

    virtual void startJoystick() = 0;

    virtual QWidget *getJoyStickWidget() = 0;   
};


#endif