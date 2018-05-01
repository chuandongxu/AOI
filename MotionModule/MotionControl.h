#pragma once

#include <QObject>
#include <QMap>
#include <QVector>

#include "../include/IMotion.h"
#include "../include/IdDefine.h"

struct QMtrVelocityProfile
{
    QMtrVelocityProfile()
    {
        _vel = 0;
        _acc = 0;
        _dec = 0;
    }

    double _vel;
    double _acc;
    double _dec;
};

struct QMtrHomeProfile
{
    enum HomeDir
    {
        HOME_DIR_POSITIVE = 0,
        HOME_DIR_NEGATIVE
    };

    enum HomeMode
    {
        HOME_MODE_HOME = 0,
        HOME_MODE_HOME_INDEX
    };

    QMtrHomeProfile() {
        _dir = HOME_DIR_NEGATIVE;
        _mode = HOME_MODE_HOME_INDEX;
    }

    QMtrVelocityProfile _velPf;
    HomeDir _dir;
    HomeMode _mode;
};

struct QMotorParam
{
    QMotorParam()
    {
        _res = 1.0;
        _name = "";
        _ID = -1;
    }

    double _res;
    int _ID;
    QString _name;
    QMtrHomeProfile _homeProf;
};

struct QMtrMoveProfile
{
    QMtrMoveProfile()
    {
        _name = "";
        _ID = -1;
        _smooth = 0;
    }

    QMtrVelocityProfile _velPf;
    QString _name;
    int _ID;
    int _smooth;
};

struct QMtrMovePoint
{
    QMtrMovePoint()
    {
        _AxisID = 0;
        _ProfID = 0;

        _name = "";
        _ID = -1;
        _posn = 0;
    }

    int _AxisID;
    int _ProfID;

    QString _name;
    int _ID;
    double _posn;
};

struct QMtrMovePointGroup
{
    QMtrMovePointGroup()
    {
        _name = "";
        _ID = -1;
    }

    std::vector<int> _movePointIDs;
    QString _name;
    int _ID;
};

class MotionControl : public QObject
{
    Q_OBJECT

public:
    MotionControl(QObject *parent = NULL);
    ~MotionControl();

public:
    enum AxisEnum
    {
        MTR_AXIS_Z = 0,        
        MTR_AXIS_X,
        MTR_AXIS_Y,
    };

    void loadConfig();

    // General Functions:
    bool init();
    void unInit();
    bool reset();
    void clearError(int AxisID);
    void clearAllError();
    bool IsPowerError();

    // IO Functions:
    bool setDOs(QVector<int>& nPorts, int iState);
    bool setDO(int nPort, int iState);
    bool getDO(int nPort, int &iState);
    bool getDI(int nPort, int &iState);

    // 3D Functions:
    bool triggerCapturing(IMotion::TRIGGER emTrig, bool bWaitDone, bool bClearSetupConfig);

    // Motor Functions:
    bool enable(int AxisID);
    bool disable(int AxisID);
    bool enableAllAxis();
    bool isEnabled(int AxisID);
    bool IsError(int AxisID);
    bool IsEMStopError(int AxisID);
    bool IsLimit(int AxisID);

    bool homeAll(bool bSyn);
    bool home(int AxisID, bool bSyn);
    bool homeLimit(int AxisID, bool bSyn);
    bool move(int AxisID, int nProfile, double dDist, bool bSyn);
    bool moveTo(int AxisID, int nProfile, double dPos, bool bSyn);

    bool movePos(int nPointTable, bool bSyn);
    bool moveToPos(int nPointTable, bool bSyn);

    bool movePosGroup(int nPtGroup, bool bSyn);
    bool moveToPosGroup(int nPtGroup, bool bSyn);

    bool moveToGroup(std::vector<int>& axis, std::vector<double>& pos, std::vector<int>& profiles, bool bSyn);
    bool moveGroup(std::vector<int>& axis, std::vector<double>& dists, std::vector<int>& profiles, bool bSyn);

    bool move(int AxisID, double dVec, double acc, double dec, int smooth, double dPos, bool bSyn);

    bool waitDone();

    bool isHomed(int AxisID);
    bool isMoveDone(int AxisID);
    bool isMoveLimit(int AxisID);
    bool stopMove(int AxisID);
    bool EmStop(int AxisID);

    bool getCurrentPos(int AxisID, double *posMm);

    QString getCurrentStatus(int AxisID);

    int getMotorAxisNum();
    int getMotorAxisID(int nIndex);
    int getMotorAxisIndex(int AxisID);

    void clearMotorParams();
    void addMotorParam(QMotorParam& mtrParam);
    void updateMotorParam(int nID, QMotorParam& mtrParam);
    int getMotorParamsNum();
    QMotorParam& getMotorParam(int nID);
    QMotorParam& getMotorParamByIndex(int nIndex);

    void clearMotorProfiles();
    void addMotorProfile(QMtrMoveProfile& mtrMoveProf);
    void updateMotorProfile(int nID, QMtrMoveProfile& mtrMoveProf);
    int getMotorProfilesNum();
    QMtrMoveProfile& getMotorProfile(int nID);
    QMtrMoveProfile& getMotorProfileByIndex(int nIndex);
    void removeMotorProfile(int nID);
    int incrementMotorProfileID();
    void setMotorProfileID(int nID);

    void clearMotorPoints();
    void addMotorPoint(QMtrMovePoint& mtrMovePoint);
    void updateMotorPoint(int nID, QMtrMovePoint& mtrMovePoint);
    int getMotorPointsNum();
    QMtrMovePoint& getMotorPoint(int nID);
    QMtrMovePoint& getMotorPointByIndex(int nIndex);
    void removeMotorPoint(int nID);
    int incrementMotorPointID();
    void setMotorPointID(int nID);

    void clearMotorPointGroups();
    void addMotorPointGroup(QMtrMovePointGroup& mtrMovePointGroup);
    void updateMotorPointGroup(int nID, QMtrMovePointGroup& mtrMovePointGroup);
    int getMotorPointGroupNum();
    QMtrMovePointGroup& getMotorPointGroup(int nID);
    QMtrMovePointGroup& getMotorPointGroupByIndex(int nIndex);
    void removeMotorPointGroup(int nID);
    int incrementMotorPointGroupID();
    void setMotorPointGroupID(int nID);

    double convertToMm(AxisEnum emAxis, long lPulse);
    long convertMmToPulse(AxisEnum emAxis, double dDist);

    double convertVelToMm(AxisEnum emAxis, double dVelPulse);
    double convertVelToPulse(AxisEnum emAxis, double dVelDist);

    double convertAccToMm(AxisEnum emAxis, double dAccPulse);
    double convertAccToPulse(AxisEnum emAxis, double dAccDist);

    AxisEnum changeToMtrEnum(int AxisID);
    int		 changeToMtrID(AxisEnum emAxis);

private:
    void commandhandler(char *command, short error);
    void setupTrigger(IMotion::TRIGGER emTrig);
    double _getMotorRes(AxisEnum emAxis);

private:
    bool                        m_bHome[AXIS_MOTOR_NUM];
    double                      m_dRunOfflinePos[AXIS_MOTOR_NUM];

    QMap<int, int>              m_mapMtrID;

    bool                        m_bSetupTriggerConfig;

    QVector<QMotorParam>        m_mtrParams;
    QVector<QMtrMoveProfile>    m_mtrMoveProfs;
    int                         m_nMoveProfID;
    QVector<QMtrMovePoint>      m_mtrMovePoints;
    int                         m_nMovePointID;
    QVector<QMtrMovePointGroup> m_mtrMovePointGroups;
    int                         m_nMovePointGroupID;
    int                         m_nMotionControlFreq = 1000;       // Currently the motion controller frequency is 8000
};
