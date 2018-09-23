#include "MotionControl.h"
#include "../common/SystemData.h"
#include "../include/IdDefine.h"
#include "gts.h"

#include <QApplication>
#include <QThread>
#include <QDebug>
#include "../include/constants.h"

void setupTriggerConfig(TMPGenPara* pPara, int nDLP_ID)
{
    double dPatternExposure = System->getParam("motion_trigger_pattern_exposure").toDouble();
    double dPatternPeriod = System->getParam("motion_trigger_pattern_period").toDouble();
    int nPatternNum = System->getParam("motion_trigger_pattern_num").toInt();

    pPara->DoChnJoinNo = 1;
    pPara->DoChnMap[0] = nDLP_ID; //should confirm id start from 0 or 1?    
    pPara->DoOutCfg[0] = 0xfff; //111111111111    
    pPara->firstLevel = 1; //该group首先出现的电平状态,0:低电平;1:高电平
    pPara->highLevelTime = dPatternExposure; //高电平持续时间[ms]
    pPara->lowLevelTime = (dPatternPeriod > dPatternExposure) ? (dPatternPeriod - dPatternExposure) : 0; //低电平持续时间[ms]
    pPara->pulseNum = nPatternNum; //该group输出脉冲的个数,取值范围:非负数,当为0时,表示无限输出脉冲,直到遇到关闭指令
}

////////////////////////////////////////
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif
/////////////////////////////////

// 定义home回零搜索距离
#define SEARCH_HOME 400000
// 定义到home捕获位置的偏移量
#define HOME_OFFSET 2000
// 定义index回零搜索距离
#define SEARCH_INDEX 150000
// 定义到index捕获位置的偏移量
#define INDEX_OFFSET 1000

MotionControl::MotionControl(QObject *parent)
    : QObject(parent)
{
    m_bSetupTriggerConfig = false;

    m_nMoveProfID = 0;
    m_nMovePointID = 0;
    m_nMovePointGroupID = 0;

    for (int i = 0; i < AXIS_MOTOR_NUM; i++)
    {
        m_bHome[i] = false;
    }

    if (!init())
    {
        System->setTrackInfo(QStringLiteral("控制系统初始化出问题！"));
    }

    loadConfig();
}

MotionControl::~MotionControl()
{
}

void MotionControl::loadConfig()
{
    m_mapMtrID.clear();    
    m_mapMtrID.insert(AxisEnum::MTR_AXIS_Z, AXIS_MOTOR_Z);
    m_mapMtrID.insert(AxisEnum::MTR_AXIS_X, AXIS_MOTOR_X);
    //m_mapMtrID.insert(AxisEnum::MTR_AXIS_Y, AXIS_MOTOR_Y);    
}

bool MotionControl::init()
{
    short rtn = 0;

    rtn = GT_Open();
    commandhandler("GT_Open", rtn);
    if (rtn)
    {
        rtn = GT_Close();
        return false;
    }

    QString path = QApplication::applicationDirPath();
    path += "/config/motion/";
    std::string fileName = QString(path + "GTS800_InHouse.cfg").toStdString();
    const char *expr = fileName.c_str();
    char *buf = new char[strlen(expr) + 1];
    strcpy(buf, expr);
    rtn = GT_Reset();
    commandhandler("GT_Reset", rtn);
    rtn = GT_LoadConfig(buf);
    commandhandler("GT_LoadConfig", rtn);
    delete[] buf;

    if (rtn)
    {
        System->setTrackInfo(QStringLiteral("控制系统Load配置文件出问题！"));
        return false;
    }

    //获取系统 Version
    char* version = new char[200];
    rtn = GT_GetVersion(&version);
    commandhandler("GT_GetVersion", rtn);
    System->setTrackInfo(QString("Motion Version:%1").arg(version));
    //delete[] version;

    if (rtn)
    {
        System->setTrackInfo(QStringLiteral("控制系统获取Version出问题！"));
        return false;
    }

    for (int j = 0; j < 16; j++)
    {
        rtn += GT_SetDoBit(MC_GPO, j, 1);
    }
    commandhandler("GT_SetDoBit", rtn);

    return true;
}

void MotionControl::unInit()
{
    for (int i = 0; i < getMotorAxisNum(); i++)
    {
        m_bHome[i] = false;
    }
    GT_Close();
}

bool MotionControl::reset()
{
    for (int i = 0; i < getMotorAxisNum(); i++)
    {
        m_bHome[i] = false;
    }

    short rtn = 0;

    rtn = GT_Reset();
    commandhandler("GT_Reset", rtn);

    return 0 == rtn;
}

void MotionControl::clearError(int AxisID)
{
    // 指令返回值
    short sRtn = 0;

    // 清除轴状态
    sRtn += GT_ClrSts(AxisID, 8);

    commandhandler("GT_ClrSts", sRtn);
}

void MotionControl::clearAllError()
{
    // 指令返回值
    short sRtn = 0;

    // 清除轴状态
    for (int i = 0; i < getMotorAxisNum(); i++)
    {
        sRtn += GT_ClrSts(m_mapMtrID.values().at(i), 8);
    }

    commandhandler("GT_ClrSts", sRtn);
}

bool MotionControl::IsPowerError()
{
    return true;
}

bool MotionControl::setDOs(QVector<int>& nPorts, int iState)
{
    // 指令返回值
    short sRtn = 0;

    long value = 0;
    ushort mask = 0;

    int nSet = iState > 0 ? 1 : 0;
    for (int i = 0; i < nPorts.size(); i++)
    {
        value = value | (nSet << (nPorts[i] -1));
        mask = mask | (1 << (nPorts[i] - 1));
    }

    // EXO6输出高电平，使指示灯灭
    sRtn = GT_SetDoMask(MC_GPO, mask, value);
    //sRtn = GT_SetDo(MC_GPO, value);    
    //sRtn = GT_SetDoBit(MC_GPO, nPort, iState);

    commandhandler("setDOs", sRtn);

    return true;
}

bool MotionControl::setDO(int nPort, int iState)
{
    // 指令返回值
    short sRtn = 0;

    // EXO6输出高电平，使指示灯灭
    //sRtn = GT_SetDo(MC_GPO, 1 << nPort);
    sRtn = GT_SetDoBit(MC_GPO, nPort, iState);
    commandhandler("GT_SetDo", sRtn);

    return true;
}

bool MotionControl::getDO(int nPort, int &iState)
{
    // 指令返回值
    short sRtn = 0;

    // 通用输入读取值
    long lGpiValue = 0;

    // 读取EXI3输入值
    sRtn = GT_GetDo(MC_GPO, &lGpiValue);

    //commandhandler("GT_GetDo", sRtn);

    iState = 0;
    if (lGpiValue & (1 << nPort))
    {
        iState = 1;
    }

    return true;
}

bool MotionControl::getDI(int nPort, int &iState)
{
    // 指令返回值
    short sRtn = 0;

    // 通用输入读取值
    long lGpiValue = 0;

    // 读取EXI3输入值
    sRtn = GT_GetDi(MC_GPI, &lGpiValue);

    //commandhandler("GT_GetDi", sRtn);

    iState = 0;
    if (lGpiValue & (1 << nPort))
    {
        iState = 1;
    }

    return true;
}

// 3D Functions:
bool MotionControl::triggerCapturing(IMotion::TRIGGER emTrig, bool bWaitDone, bool bClearSetupConfig)
{
    // 指令返回值
    short rtn = 0;

    short sMultPulseSts = 0;
    rtn = GT_GetPulseWaveGenStatus(&sMultPulseSts);
    commandhandler("GT_GetPulseWaveGenStatus", rtn);

    if (1 == sMultPulseSts)
    {
        System->setTrackInfo(QStringLiteral("motion trigger signal not ready yet"));
        return false;
    }

    if (bClearSetupConfig) m_bSetupTriggerConfig = false;

    if (!m_bSetupTriggerConfig)
    {
        m_bSetupTriggerConfig = true;

        setupTrigger(emTrig);        
    }

    rtn = GT_TrigPulseWaveGen();
    commandhandler("GT_TrigPulseWaveGen", rtn);

    rtn = GT_GetPulseWaveGenStatus(&sMultPulseSts);
    commandhandler("GT_GetPulseWaveGenStatus", rtn);

    if (bWaitDone)
    {
        int nWaitTime = 30 * 100;// 10 seconds
        while (1 == sMultPulseSts && nWaitTime-- > 0)
        {
            GT_GetPulseWaveGenStatus(&sMultPulseSts);
            QThread::msleep(10);
        }
        if (nWaitTime <= 0) return false;
    }    

    return true;
}

void MotionControl::setupTrigger(IMotion::TRIGGER emTrig)
{
    // 指令返回值
    short rtn = 0;

    short sTotalSerial = 0;

    TMPGenPara MGenStrPrm[MAX_WAVE_SERIALS];
    memset(&MGenStrPrm, 0, sizeof(MGenStrPrm));

    switch (emTrig)
    {
    case IMotion::TRIGGER_ALL:
    {        
        sTotalSerial = 5;
        setupTriggerConfig(MGenStrPrm + 0, DO_TRIGGER_DLP1);
        setupTriggerConfig(MGenStrPrm + 1, DO_TRIGGER_DLP2);
        setupTriggerConfig(MGenStrPrm + 2, DO_TRIGGER_DLP3);
        setupTriggerConfig(MGenStrPrm + 3, DO_TRIGGER_DLP4);

        double dLightExposure = System->getParam("motion_trigger_light_exposure").toDouble() / 1000.0;
        double dLightPeriod = System->getParam("motion_trigger_light_period").toDouble();
        MGenStrPrm[4].DoChnJoinNo = 7;
        MGenStrPrm[4].DoChnMap[0] = DO_CAMERA_TRIGGER2; //should confirm id start from 0 or 1?
        MGenStrPrm[4].DoChnMap[1] = DO_LIGHT1_CH1;
        MGenStrPrm[4].DoChnMap[2] = DO_LIGHT1_CH2;
        MGenStrPrm[4].DoChnMap[3] = DO_LIGHT1_CH3;
        MGenStrPrm[4].DoChnMap[4] = DO_LIGHT1_CH4;
        MGenStrPrm[4].DoChnMap[5] = DO_LIGHT2_CH1;
        MGenStrPrm[4].DoChnMap[6] = DO_LIGHT2_CH2;
        MGenStrPrm[4].DoOutCfg[0] = 0x3F; //111111
        MGenStrPrm[4].DoOutCfg[1] = 0x4;  //000100; 信号出现的顺序是相反的，然后转成16进制
        MGenStrPrm[4].DoOutCfg[2] = 0x21; //100001;
        MGenStrPrm[4].DoOutCfg[3] = 0x8;  //001000;
        MGenStrPrm[4].DoOutCfg[4] = 0x10; //010000;
        MGenStrPrm[4].DoOutCfg[5] = 0x22; //100010;
        MGenStrPrm[4].DoOutCfg[6] = 0x10; //010000;
        MGenStrPrm[4].firstLevel = 1;
        MGenStrPrm[4].highLevelTime = dLightExposure;
        MGenStrPrm[4].lowLevelTime = (dLightPeriod > dLightExposure) ? (dLightPeriod - dLightExposure) : 0;
        MGenStrPrm[4].pulseNum = 6;            
    }
    break;
    case IMotion::TRIGGER_DLP:
    {
        int nDlpNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
        int nDlpNumIndex = System->getParam("motion_trigger_dlp_num_index").toInt();
        if (0 == nDlpNumIndex)
        {
            sTotalSerial = nDlpNum;
            setupTriggerConfig(MGenStrPrm + 0, DO_TRIGGER_DLP1);
            setupTriggerConfig(MGenStrPrm + 1, DO_TRIGGER_DLP2);        
        }
        else if (1 == nDlpNumIndex)
        {
            sTotalSerial = nDlpNum;
            setupTriggerConfig(MGenStrPrm + 0, DO_TRIGGER_DLP1);
            setupTriggerConfig(MGenStrPrm + 1, DO_TRIGGER_DLP2);
            setupTriggerConfig(MGenStrPrm + 2, DO_TRIGGER_DLP3);
            setupTriggerConfig(MGenStrPrm + 3, DO_TRIGGER_DLP4);
        }
    }
    break;
    case IMotion::TRIGGER_DLP1:
        sTotalSerial = 1;
        setupTriggerConfig(MGenStrPrm + 0, DO_TRIGGER_DLP1);
        break;
    case IMotion::TRIGGER_DLP2:
        sTotalSerial = 1;
        setupTriggerConfig(MGenStrPrm + 0, DO_TRIGGER_DLP2);
        break;
    case IMotion::TRIGGER_DLP3:
        sTotalSerial = 1;
        setupTriggerConfig(MGenStrPrm + 0, DO_TRIGGER_DLP3);
        break;
    case IMotion::TRIGGER_DLP4:
        sTotalSerial = 1;
        setupTriggerConfig(MGenStrPrm + 0, DO_TRIGGER_DLP4);
        break;
    case IMotion::TRIGGER_LIGHT:
    {
        double dLightExposure = System->getParam("motion_trigger_light_exposure").toDouble() / 1000.0;
        double dLightPeriod = System->getParam("motion_trigger_light_period").toDouble();

        sTotalSerial = 1;
        MGenStrPrm[0].DoChnJoinNo = 7;
        MGenStrPrm[0].DoChnMap[0] = DO_CAMERA_TRIGGER2; //should confirm id start from 0 or 1?
        MGenStrPrm[0].DoChnMap[1] = DO_LIGHT1_CH1;
        MGenStrPrm[0].DoChnMap[2] = DO_LIGHT1_CH2;
        MGenStrPrm[0].DoChnMap[3] = DO_LIGHT1_CH3;
        MGenStrPrm[0].DoChnMap[4] = DO_LIGHT1_CH4;
        MGenStrPrm[0].DoChnMap[5] = DO_LIGHT2_CH1;
        MGenStrPrm[0].DoChnMap[6] = DO_LIGHT2_CH2;
        MGenStrPrm[0].DoOutCfg[0] = 0x3F; //111111
        MGenStrPrm[0].DoOutCfg[1] = 0x4;  //000100; 信号出现的顺序是相反的，然后转成16进制
        MGenStrPrm[0].DoOutCfg[2] = 0x21; //100001;
        MGenStrPrm[0].DoOutCfg[3] = 0x8;  //001000;
        MGenStrPrm[0].DoOutCfg[4] = 0x10; //010000;
        MGenStrPrm[0].DoOutCfg[5] = 0x22; //100010;
        MGenStrPrm[0].DoOutCfg[6] = 0x10; //010000;
        MGenStrPrm[0].firstLevel = 1;
        MGenStrPrm[0].highLevelTime = dLightExposure;
        MGenStrPrm[0].lowLevelTime = (dLightPeriod > dLightExposure) ? (dLightPeriod - dLightExposure) : 0;
        MGenStrPrm[0].pulseNum = 6;
    }
    break;
    default:
        break;
    }

    //加载用户配置
    rtn = GT_PreLoadPulseWaveGenParm(sTotalSerial, MGenStrPrm);
    commandhandler("GT_PreLoadPulseWaveGenParm", rtn);
}

void MotionControl::commandhandler(char *command, short error)
{
    // 如果指令执行返回值为非0，说明指令执行错误，向屏幕输出错误结果
    if (error)
    {
        System->setTrackInfo(QStringLiteral("%1 error, error code = %2").arg(command).arg(error));
    }
}

double MotionControl::convertToMm(AxisEnum emAxis, long lPulse)
{
    double dRes = _getMotorRes(emAxis);
    if (dRes <= 0.f)
        return 0.;
    return (double)lPulse / dRes;
}

long MotionControl::convertMmToPulse(AxisEnum emAxis, double dDist)
{
    double dRes = _getMotorRes(emAxis);
    if (dRes <= 0.f)
        return 0;
    return (long)(dDist * dRes);
}

double MotionControl::convertVelToMm(AxisEnum emAxis, double dVelPulse)
{
    double dRes = _getMotorRes(emAxis);
    if (dRes <= 0.f)
        return 0.;

    return dVelPulse / dRes *m_nMotionControlFreq; // pulse/sample -> mm/s
}

double MotionControl::convertVelToPulse(AxisEnum emAxis, double dVelDist)
{
    double dRes = _getMotorRes(emAxis);
    if (dRes <= 0.f)
        return 0.;
    return dVelDist * dRes / m_nMotionControlFreq; // mm/s -> pulse/sample
}

double MotionControl::convertAccToMm(AxisEnum emAxis, double dAccPulse)
{
    double dRes = _getMotorRes(emAxis);
    if (dRes <= 0.f)
        return 0.;
    return dAccPulse / dRes * m_nMotionControlFreq * m_nMotionControlFreq; // pulse/sample^2 -> mm/s^2
}

double MotionControl::convertAccToPulse(AxisEnum emAxis, double dAccDist)
{
    double dRes = _getMotorRes(emAxis);
    if (dRes <= 0.f)
        return 0.;
    return dAccDist * dRes / m_nMotionControlFreq / m_nMotionControlFreq; // mm/s^2 -> pulse/sample^2
}

double MotionControl::_getMotorRes(AxisEnum emAxis)
{
    auto motorIndex = getMotorAxisIndex(changeToMtrID(emAxis));
    if (motorIndex < 0) {
        System->setTrackInfo(QStringLiteral("Motor index %1 not defined.").arg(emAxis));
        return 0.f;
    }

    return m_mtrParams[motorIndex]._res;
}

MotionControl::AxisEnum MotionControl::changeToMtrEnum(int AxisID)
{
    return (AxisEnum)m_mapMtrID.key(AxisID);
}

int MotionControl::changeToMtrID(AxisEnum emAxis)
{
    return m_mapMtrID.value((int)emAxis);
}

int MotionControl::getMotorAxisNum()
{
    return m_mapMtrID.size();
}

int MotionControl::getMotorAxisID(int nIndex)
{
    if (nIndex < 0 || nIndex >= m_mapMtrID.size()) return 0;
    return m_mapMtrID.values().at(nIndex);
}

int MotionControl::getMotorAxisIndex(int AxisID)
{
    int nIndex = m_mapMtrID.values().indexOf(AxisID, 0);
    return nIndex >= 0 ? nIndex : -1;
}

bool MotionControl::enable(int AxisID)
{
    short sRtn = 0; // 指令返回值变量
    sRtn = GT_AxisOn(AxisID);
    commandhandler("GT_AxisOn", sRtn);

    return 0 == sRtn;
}

bool MotionControl::disable(int AxisID)
{
    short sRtn = 0; // 指令返回值变量
    sRtn = GT_AxisOff(AxisID);
    commandhandler("GT_AxisOff", sRtn);

    return 0 == sRtn;
}

bool MotionControl::enableAllAxis()
{
    short sRtn = 0; // 指令返回值变量
    for (int i = 0; i < getMotorAxisNum(); i++)
    {
        sRtn += GT_AxisOn(m_mapMtrID.values().at(i));        
        commandhandler("GT_AxisOn", sRtn);
    }

    return 0 == sRtn;
}

bool MotionControl::isEnabled(int AxisID)
{
    short sRtn = 0; // 指令返回值变量
    long lAxisStatus = 0; // 轴状态

    short bFlagServoOn = FALSE; // 伺服使能标志

    // 读取轴状态
    sRtn = GT_GetSts(AxisID, &lAxisStatus);
    commandhandler("GT_GetSts", sRtn);

    // 伺服使能标志
    if (lAxisStatus & 0x200)
    {
        bFlagServoOn = TRUE;        
    }
    else
    {
        bFlagServoOn = FALSE;
    }

    return bFlagServoOn;
}

bool MotionControl::IsError(int AxisID)
{
    short sRtn = 0; // 指令返回值变量
    long lAxisStatus = 0; // 轴状态

    short bFlagAlarm = FALSE; // 伺服报警标志
    short bFlagMError = FALSE; // 跟随误差越限标志

    // 读取轴状态
    sRtn = GT_GetSts(AxisID, &lAxisStatus);
    commandhandler("GT_GetSts", sRtn);

    // 伺服报警标志
    if (lAxisStatus & 0x2)
    {
        bFlagAlarm = TRUE;        
    }
    else
    {
        bFlagAlarm = FALSE;        
    }

    // 跟随误差越限标志
    if (lAxisStatus & 0x10)
    {
        bFlagMError = TRUE;        
    }
    else
    {
        bFlagMError = FALSE;    
    }

    return bFlagAlarm || bFlagMError;
}

bool MotionControl::IsEMStopError(int AxisID)
{
    short sRtn = 0; // 指令返回值变量
    long lAxisStatus = 0; // 轴状态

    short bFlagAbruptStop = FALSE; // 急停标志

    // 读取轴状态
    sRtn = GT_GetSts(AxisID, &lAxisStatus);
    commandhandler("GT_GetSts", sRtn);

    // 急停标志
    if (lAxisStatus & 0x100)
    {
        bFlagAbruptStop = TRUE;        
    }
    else
    {
        bFlagAbruptStop = FALSE;    
    }

    return bFlagAbruptStop;
}

bool MotionControl::IsLimit(int AxisID)
{
    short sRtn = 0; // 指令返回值变量
    long lAxisStatus = 0; // 轴状态

    short bFlagPosLimit = FALSE; // 正限位触发标志
    short bFlagNegLimit = FALSE; // 负限位触发标志

    // 读取轴状态
    sRtn = GT_GetSts(AxisID, &lAxisStatus);
    commandhandler("GT_GetSts", sRtn);

    // 正向限位
    if (lAxisStatus & 0x20)
    {
        bFlagPosLimit = TRUE;    
    }
    else
    {
        bFlagPosLimit = FALSE;
    }

    // 负向限位
    if (lAxisStatus & 0x40)
    {
        bFlagNegLimit = TRUE;        
    }
    else
    {
        bFlagNegLimit = FALSE;        
    }

    return bFlagPosLimit || bFlagNegLimit;
}

bool MotionControl::homeAll(bool bSyn)
{
    int nMtrNum = getMotorAxisNum();
    for (int i = 0; i < nMtrNum; i++)
    {
        if (!homeLimit(getMotorAxisID(i), false))
        {
            return false;
        }
    }

    if (bSyn) return waitDone();
    return true;
}

bool MotionControl::home(int AxisID, bool bSyn)
{
    if (System->isRunOffline()) {
        m_dRunOfflinePos[AxisID] = 0.;
        return true;
    }

    auto nAxisIndex = getMotorAxisIndex(AxisID);
    if (nAxisIndex < 0) {
        System->setTrackInfo(QStringLiteral("Motor ID %1 not defined.").arg(AxisID));
        return false;
    }

    short sRtn = 0; // 指令返回值变量

    int nHomeDir = m_mtrParams[nAxisIndex]._homeProf._dir == 0 ? 1 : -1;

    // 清状态
    sRtn = GT_ClrSts(AxisID);
    commandhandler("GT_ClrSts", sRtn);

    if (!isEnabled(AxisID))
    {
        if (!enable(AxisID)) return false;        
    }

    // 开启轴的home捕获功能
    sRtn = GT_SetCaptureMode(AxisID, CAPTURE_HOME);
    commandhandler("GT_SetCaptureMode", sRtn);
    // 设置轴为点位运动模式
    sRtn = GT_PrfTrap(AxisID);
    commandhandler("GT_PrfTrap", sRtn);
    // 读取点位运动参数
    TTrapPrm trapPrm;
    sRtn = GT_GetTrapPrm(AxisID, &trapPrm);
    commandhandler("GT_GetTrapPrm", sRtn);
    trapPrm.acc = m_mtrParams[nAxisIndex]._homeProf._velPf._acc;
    trapPrm.dec = m_mtrParams[nAxisIndex]._homeProf._velPf._dec;
    trapPrm.smoothTime = 40;
    // 设置点位运动参数
    sRtn = GT_SetTrapPrm(AxisID, &trapPrm);
    commandhandler("GT_SetTrapPrm", sRtn);
    // 设置目标速度
    sRtn = GT_SetVel(AxisID, m_mtrParams[nAxisIndex]._homeProf._velPf._vel);
    commandhandler("GT_SetVel", sRtn);
    // 设置目标位置
    sRtn = GT_SetPos(AxisID, SEARCH_HOME * nHomeDir);
    commandhandler("GT_SetPos", sRtn);
    // 启动运动，等待home信号触发
    sRtn = GT_Update(1 << (AxisID - 1));
    commandhandler("GT_Update", sRtn);

    // 捕获状态
    short capture = 0;
    // 捕获位置
    long pos = 0;

    // 分别是规划位置，编码器位
    double prfPos = 0, encPos = 0;

    int nTimeOut = 30 * 100;// 30 seconds
    do
    {
        // 读取捕获状态
        sRtn = GT_GetCaptureStatus(AxisID, &capture, &pos);
        // 读取规划位置
        sRtn = GT_GetPrfPos(AxisID, &prfPos);
        // 读取编码器位置
        sRtn = GT_GetEncPos(AxisID, &encPos);
    
        // 电机已经停止，说明整个搜索过程中home信号一直没有触发
        if (isMoveDone(AxisID))
        {
            System->setTrackInfo(QStringLiteral("电机已经停止, Home失败！"));
            return false;
        }

        QThread::msleep(10);
        // 如果home信号已经触发，则退出循环，捕获位置已经在pos变量中保存
    } while (0 == capture && nTimeOut-- > 0);

    if (nTimeOut <= 0)
    {
        System->setTrackInfo(QStringLiteral("电机回零TimeOut！"));
        return false;
    }

    // 设定目标位置为捕获位置+偏移量
    sRtn = GT_SetPos(AxisID, pos + HOME_OFFSET*nHomeDir);
    commandhandler("GT_SetPos", sRtn);
    // 启动运动
    sRtn = GT_Update(1 << (AxisID - 1));
    commandhandler("GT_Update", sRtn);

    nTimeOut = 30 * 100;// 30 seconds
    do 
    {
        // 读取规划位置
        sRtn = GT_GetPrfPos(AxisID, &prfPos);

        QThread::msleep(10);
    } while (!isMoveDone(AxisID) && nTimeOut-- > 0);


    if (nTimeOut <= 0)
    {
        System->setTrackInfo(QStringLiteral("电机回零TimeOut！"));
        return false;
    }

    if (qAbs(prfPos - (pos + HOME_OFFSET)) > 10)
    {
        System->setTrackInfo(QStringLiteral("电机回零位置偏移问题！"));
        return false;
    }

    QThread::msleep(200);

    // 启动index捕获
    sRtn = GT_SetCaptureMode(AxisID, CAPTURE_INDEX);
    commandhandler("GT_SetCaptureMode", sRtn);

    // 设置当前位置+index搜索距离为目标位置
    sRtn = GT_SetPos(AxisID, (long)(prfPos + SEARCH_INDEX * (-nHomeDir)));
    commandhandler("GT_SetPos", sRtn);

    // 启动运动
    sRtn = GT_Update(1 << (AxisID - 1));
    commandhandler("GT_Update", sRtn);

    nTimeOut = 30 * 100;// 30 seconds
    do
    {
        // 读取捕获状态
        sRtn = GT_GetCaptureStatus(AxisID, &capture, &pos);
        // 读取规划位置
        sRtn = GT_GetPrfPos(AxisID, &prfPos);
        // 读取编码器位置
        sRtn = GT_GetEncPos(AxisID, &encPos);
    
        // 电机已经停止，说明整个搜索过程中index信号一直没有触发
        if (isMoveDone(AxisID))
        {
            System->setTrackInfo(QStringLiteral("电机已经停止, Index失败！"));
            return false;
        }

        QThread::msleep(10);

        // 如果index信号已经触发，则退出循环，捕获位置已经在pos变量中保存
    } while (0 == capture && nTimeOut-- > 0);

    if (nTimeOut <= 0)
    {
        System->setTrackInfo(QStringLiteral("电机回零TimeOut！"));
        return false;
    }

    // 设置捕获位置+index偏移量为目标位置
    sRtn = GT_SetPos(AxisID, pos + INDEX_OFFSET * (-nHomeDir));
    commandhandler("GT_SetPos", sRtn);
    // 启动运动
    sRtn = GT_Update(1 << (AxisID - 1));
    commandhandler("GT_Update", sRtn);

    nTimeOut = 30 * 100;// 30 seconds
    do
    {
        // 读取规划位置
        sRtn = GT_GetPrfPos(AxisID, &prfPos);
        // 读取编码器位置
        sRtn = GT_GetEncPos(AxisID, &encPos);

        QThread::msleep(10);
        
    } while (!isMoveDone(AxisID) && nTimeOut-- > 0);

    if (nTimeOut <= 0)
    {
        System->setTrackInfo(QStringLiteral("电机回零TimeOut！"));
        return false;
    }
    if (qAbs(prfPos - (pos + INDEX_OFFSET * (-nHomeDir))) > 10)
    {
        System->setTrackInfo(QStringLiteral("电机Index位置偏移问题！"));
        return false;
    }

    QThread::msleep(200);

    // 更新原点位置
    sRtn = GT_ZeroPos(AxisID);
    commandhandler("GT_ZeroPos", sRtn);

    // AXIS轴规划位置清零
    sRtn = GT_SetPrfPos(AxisID, 0);
    commandhandler("GT_SetPrfPos", sRtn);

    m_bHome[nAxisIndex] = true;

    return true;
}

bool MotionControl::homeLimit(int AxisID, bool bSyn)
{
    if (System->isRunOffline()) {
        m_dRunOfflinePos[AxisID] = 0.f;
        return true;
    }

    short sRtn = 0; // 指令返回值变量

    int nHomeDir = m_mtrParams[getMotorAxisIndex(AxisID)]._homeProf._dir == 0 ? 1 : -1;

    // 清状态
    sRtn = GT_ClrSts(AxisID);
    commandhandler("GT_ClrSts", sRtn);

    if (!isEnabled(AxisID))
    {
        if (!enable(AxisID)) return false;
    }

    // 开启轴的home捕获功能
    sRtn = GT_SetCaptureMode(AxisID, CAPTURE_HOME);
    commandhandler("GT_SetCaptureMode", sRtn);
    // 设置轴为点位运动模式
    sRtn = GT_PrfTrap(AxisID);
    commandhandler("GT_PrfTrap", sRtn);
    // 读取点位运动参数
    TTrapPrm trapPrm;
    sRtn = GT_GetTrapPrm(AxisID, &trapPrm);
    commandhandler("GT_GetTrapPrm", sRtn);
    trapPrm.acc = m_mtrParams[getMotorAxisIndex(AxisID)]._homeProf._velPf._acc;
    trapPrm.dec = m_mtrParams[getMotorAxisIndex(AxisID)]._homeProf._velPf._dec;
    trapPrm.smoothTime = 40;
    // 设置点位运动参数
    sRtn = GT_SetTrapPrm(AxisID, &trapPrm);
    commandhandler("GT_SetTrapPrm", sRtn);
    // 设置目标速度
    sRtn = GT_SetVel(AxisID, m_mtrParams[getMotorAxisIndex(AxisID)]._homeProf._velPf._vel);
    commandhandler("GT_SetVel", sRtn);
    // 设置目标位置
    sRtn = GT_SetPos(AxisID, SEARCH_HOME * nHomeDir);
    commandhandler("GT_SetPos", sRtn);
    // 启动运动，等待home信号触发
    sRtn = GT_Update(1 << (AxisID - 1));
    commandhandler("GT_Update", sRtn);

    // 捕获状态
    short capture = 0;
    // 捕获位置
    long pos = 0;

    // 分别是规划位置，编码器位
    double prfPos = 0, encPos = 0;

    int nTimeOut = 120 * 100;// 30 seconds
    do
    {
        // 读取捕获状态
        sRtn = GT_GetCaptureStatus(AxisID, &capture, &pos);
        // 读取规划位置
        sRtn = GT_GetPrfPos(AxisID, &prfPos);
        // 读取编码器位置
        sRtn = GT_GetEncPos(AxisID, &encPos);

        // 电机已经停止，说明整个搜索过程中home信号一直没有触发
        if (isMoveDone(AxisID))
        {
            if (!isMoveLimit(AxisID))
            {
                System->setTrackInfo(QStringLiteral("电机已经停止, Home失败！"));
                return false;
            }

            break;
        }

        QThread::msleep(10);
        // 如果home信号已经触发，则退出循环，捕获位置已经在pos变量中保存
    } while (0 == capture && nTimeOut-- > 0);

    if (nTimeOut <= 0)
    {
        System->setTrackInfo(QStringLiteral("电机回零TimeOut！"));
        return false;
    }

    //// 设定目标位置为捕获位置+偏移量
    //sRtn = GT_SetPos(AxisID, pos + HOME_OFFSET*nHomeDir);
    //commandhandler("GT_SetPos", sRtn);
    //// 启动运动
    //sRtn = GT_Update(1 << (AxisID - 1));
    //commandhandler("GT_Update", sRtn);

    //nTimeOut = 30 * 100;// 30 seconds
    //do
    //{
    //    // 读取规划位置
    //    sRtn = GT_GetPrfPos(AxisID, &prfPos);

    //    QThread::msleep(10);
    //} while (!isMoveDone(AxisID) && nTimeOut-- > 0);


    //if (nTimeOut <= 0)
    //{
    //    System->setTrackInfo(QStringLiteral("电机回零TimeOut！"));
    //    return false;
    //}

    //if (qAbs(prfPos - (pos + HOME_OFFSET)) > 10)
    //{
    //    System->setTrackInfo(QStringLiteral("电机回零位置偏移问题！"));
    //    return false;
    //}

    QThread::msleep(200);

    // 清状态
    sRtn = GT_ClrSts(AxisID);
    commandhandler("GT_ClrSts", sRtn);

    // 清状态
    sRtn = GT_ClearCaptureStatus(AxisID);
    commandhandler("GT_ClearCaptureStatus", sRtn);


    // 启动index捕获
    sRtn = GT_SetCaptureMode(AxisID, CAPTURE_INDEX);
    commandhandler("GT_SetCaptureMode", sRtn);

    // 设置当前位置+index搜索距离为目标位置
    sRtn = GT_SetPos(AxisID, (long)(prfPos + SEARCH_INDEX * (-nHomeDir)));
    commandhandler("GT_SetPos", sRtn);

    // 启动运动
    sRtn = GT_Update(1 << (AxisID - 1));
    commandhandler("GT_Update", sRtn);

    nTimeOut = 30 * 100;// 30 seconds
    do
    {
        // 读取捕获状态
        sRtn = GT_GetCaptureStatus(AxisID, &capture, &pos);
        // 读取规划位置
        sRtn = GT_GetPrfPos(AxisID, &prfPos);
        // 读取编码器位置
        sRtn = GT_GetEncPos(AxisID, &encPos);

        // 电机已经停止，说明整个搜索过程中index信号一直没有触发
        if (isMoveDone(AxisID))
        {
            System->setTrackInfo(QStringLiteral("电机已经停止, Index失败！"));
            return false;
        }

        QThread::msleep(10);

        // 如果index信号已经触发，则退出循环，捕获位置已经在pos变量中保存
    } while (0 == capture && nTimeOut-- > 0);

    if (nTimeOut <= 0)
    {
        System->setTrackInfo(QStringLiteral("电机回零TimeOut！"));
        return false;
    }

    //// 设置捕获位置+index偏移量为目标位置
    //sRtn = GT_SetPos(AxisID, pos + INDEX_OFFSET * (-nHomeDir));
    //commandhandler("GT_SetPos", sRtn);
    //// 启动运动
    //sRtn = GT_Update(1 << (AxisID - 1));
    //commandhandler("GT_Update", sRtn);

    //nTimeOut = 30 * 100;// 30 seconds
    //do
    //{
    //    // 读取规划位置
    //    sRtn = GT_GetPrfPos(AxisID, &prfPos);
    //    // 读取编码器位置
    //    sRtn = GT_GetEncPos(AxisID, &encPos);

    //    QThread::msleep(10);

    //} while (!isMoveDone(AxisID) && nTimeOut-- > 0);

    //if (nTimeOut <= 0)
    //{
    //    System->setTrackInfo(QStringLiteral("电机回零TimeOut！"));
    //    return false;
    //}
    //if (qAbs(prfPos - (pos + INDEX_OFFSET * (-nHomeDir))) > 10)
    //{
    //    System->setTrackInfo(QStringLiteral("电机Index位置偏移问题！"));
    //    return false;
    //}

    // 清状态
    sRtn = GT_ClrSts(AxisID);
    commandhandler("GT_ClrSts", sRtn);

    // 清状态
    sRtn = GT_ClearCaptureStatus(AxisID);
    commandhandler("GT_ClearCaptureStatus", sRtn);

    // 停止
    stopMove(AxisID);

    QThread::msleep(200);

    // 更新原点位置
    sRtn = GT_ZeroPos(AxisID);
    commandhandler("GT_ZeroPos", sRtn);

    // AXIS轴规划位置清零
    sRtn = GT_SetPrfPos(AxisID, 0);
    commandhandler("GT_SetPrfPos", sRtn);

    //sRtn = GT_SetEncPos(AxisID, 0);
    //commandhandler("GT_SetEncPos", sRtn);    

    m_bHome[changeToMtrEnum(AxisID)] = true;

    return true;
}

bool MotionControl::move(int AxisID, int nProfile, double dDist, bool bSyn)
{
    QMtrMoveProfile mtrProf = getMotorProfile(nProfile);
    double dVec = mtrProf._velPf._vel;
    double acc = mtrProf._velPf._acc;
    double dec = mtrProf._velPf._dec;
    int smooth = mtrProf._smooth;

    double dCurPos = 0;
    if (!getCurrentPos(AxisID, &dCurPos))
    {
        return false;
    }
  
    return move(AxisID, dVec, acc, dec, smooth, dCurPos + dDist, bSyn);
}

bool MotionControl::moveTo(int AxisID, int nProfile, double dPos, bool bSyn)
{
    QMtrMoveProfile mtrProf = getMotorProfile(nProfile);
    if (mtrProf._ID < 0) return false;

    double dVec = mtrProf._velPf._vel;
    double acc = mtrProf._velPf._acc;
    double dec = mtrProf._velPf._dec;
    int smooth = mtrProf._smooth;

    return move(AxisID, dVec, acc, dec, smooth, dPos, bSyn);
}

bool MotionControl::movePos(int nPointTable, bool bSyn)
{
    QMtrMovePoint mtrPt = getMotorPoint(nPointTable);
    QMtrMoveProfile mtrProf = getMotorProfile(mtrPt._ProfID);
    double dVec = mtrProf._velPf._vel;
    double acc = mtrProf._velPf._acc;
    double dec = mtrProf._velPf._dec;
    int smooth = mtrProf._smooth;

    double dCurPos = 0;
    if (!getCurrentPos(mtrPt._AxisID, &dCurPos))
    {
        return false;
    }

    return move(mtrPt._AxisID, dVec, acc, dec, smooth, dCurPos + mtrPt._posn, bSyn);
}

bool MotionControl::moveToPos(int nPointTable, bool bSyn)
{
    QMtrMovePoint mtrPt = getMotorPoint(nPointTable);
    QMtrMoveProfile mtrProf = getMotorProfile(mtrPt._ProfID);
    double dVec = mtrProf._velPf._vel;
    double acc = mtrProf._velPf._acc;
    double dec = mtrProf._velPf._dec;
    int smooth = mtrProf._smooth;

    return move(mtrPt._AxisID, dVec, acc, dec, smooth, mtrPt._posn, bSyn);
}

bool MotionControl::movePosGroup(int nPtGroup, bool bSyn)
{
    QMtrMovePointGroup mtPointGroup = getMotorPointGroup(nPtGroup);
    if (mtPointGroup._ID > -1)
    {
        int nMtrNum = mtPointGroup._movePointIDs.size();
        for (int i = 0; i < nMtrNum; i++)
        {
            if (!moveToPos(mtPointGroup._movePointIDs[i], false))
            {
                return false;
            }
        }

        if (bSyn) return waitDone();
        return true;
    }

    return false;
}

bool MotionControl::moveToPosGroup(int nPtGroup, bool bSyn)
{
    QMtrMovePointGroup mtPointGroup = getMotorPointGroup(nPtGroup);
    if (mtPointGroup._ID > -1)
    {
        int nMtrNum = mtPointGroup._movePointIDs.size();
        for (int i = 0; i < nMtrNum; i++)
        {
            if (!moveToPos(mtPointGroup._movePointIDs[i], false))
            {
                return false;
            }
        }

        if (bSyn) return waitDone();
        return true;
    }

    return false;
}

bool MotionControl::moveToGroup(std::vector<int>& axis, std::vector<double>& pos, std::vector<int>& profiles, bool bSyn)
{
    if ((axis.size() != pos.size()) || (pos.size() != profiles.size())) return false;

    if (System->isRunOffline()) {
        for (int i = 0; i < axis.size(); ++ i)
            m_dRunOfflinePos[axis[i]] = pos[i];
        return true;
    }

    for (int i = 0; i < axis.size(); i++)
    {
        bool bRet = moveTo(axis[i], profiles[i], pos[i], false);
        if (!bRet) return false;
    }

    if (bSyn)
    {
        int nTimeOut = 30 * 100;// 30 seconds
        do
        {
            bool bMoveDone = true;
            for (int i = 0; i < axis.size(); i++)
            {
                if (!isMoveDone(axis[i]))
                {
                    bMoveDone = false;
                }
            }
            if (bMoveDone) break;

            QThread::msleep(10);
        } while (nTimeOut-- > 0);

        if (nTimeOut <= 0)
        {
            System->setTrackInfo(QStringLiteral("电机运动TimeOut！"));
            return false;
        }
    }

    return true;
}

bool MotionControl::moveGroup(std::vector<int>& axis, std::vector<double>& dists, std::vector<int>& profiles, bool bSyn)
{
    if ((axis.size() != dists.size()) || (dists.size() != profiles.size())) return false;

    for (int i = 0; i < axis.size(); i++)
    {
        bool bRet = move(axis[i], profiles[i], dists[i], false);
        if (!bRet) return false;
    }

    if (bSyn)
    {
        int nTimeOut = 30 * 100;// 30 seconds
        do
        {
            bool bMoveDone = true;
            for (int i = 0; i < axis.size(); i++)
            {
                if (!isMoveDone(axis[i]))
                {
                    bMoveDone = false;
                }
            }
            if (bMoveDone) break;

            QThread::msleep(10);
        } while (nTimeOut-- > 0);

        if (nTimeOut <= 0)
        {
            System->setTrackInfo(QStringLiteral("电机运动TimeOut！"));
            return false;
        }
    }

    return true;
}

bool MotionControl::move(int AxisID, double dVec, double acc, double dec, int smooth, double dPos, bool bSyn)
{
    if (System->isRunOffline()) {
        m_dRunOfflinePos[AxisID] = dPos * MM_TO_UM;
        return true;
    }

    short sRtn = 0; // 指令返回值变量

    // 将AXIS轴设为点位模式
    sRtn = GT_PrfTrap(AxisID);
    commandhandler("GT_PrfTrap", sRtn);

    TTrapPrm trap;
    // 读取点位运动参数
    sRtn = GT_GetTrapPrm(AxisID, &trap);
    commandhandler("GT_GetTrapPrm", sRtn);

    trap.acc = convertAccToPulse(changeToMtrEnum(AxisID), acc);
    trap.dec = convertAccToPulse(changeToMtrEnum(AxisID), dec);
    trap.smoothTime = smooth;

    // 设置点位运动参数
    sRtn = GT_SetTrapPrm(AxisID, &trap);
    commandhandler("GT_SetTrapPrm", sRtn);
    
    //long lPulse = convertToPulse(changeToMtrEnum(AxisID), dPos);
    //qDebug() << "move to Pos: " << AxisID << " : " << lPulse;

    // 设置AXIS轴的目标位置
    sRtn = GT_SetPos(AxisID, convertMmToPulse(changeToMtrEnum(AxisID), dPos));
    commandhandler("GT_SetPos", sRtn);

    // 设置AXIS轴的目标速度
    sRtn = GT_SetVel(AxisID, convertVelToPulse(changeToMtrEnum(AxisID), dVec));
    commandhandler("GT_SetVel", sRtn);

    // 启动AXIS轴的运动
    sRtn = GT_Update(1 << (AxisID - 1));
    commandhandler("GT_Update", sRtn);

    if (bSyn)
    {
        // 分别是规划位置，编码器位
        //double prfPos = 0, encPos = 0;
        int nTimeOut = 30 * 100;// 30 seconds
        do
        {
            // 读取规划位置
            //sRtn = GT_GetPrfPos(AxisID, &prfPos);
            // 读取编码器位置
            //sRtn = GT_GetEncPos(AxisID, &encPos);

            QThread::msleep(10);

        } while (!isMoveDone(AxisID) && nTimeOut-- > 0);

        if (nTimeOut <= 0)
        {
            System->setTrackInfo(QStringLiteral("电机运动TimeOut！"));
            return false;
        }
    }

    return true;
}

bool MotionControl::waitDone()
{
    int nTimeOut = 30 * 100;// 30 seconds
    int nMtrNum = getMotorAxisNum();
    do
    {
        bool bAllMotorDone = true;
        for (int i = 0; i < nMtrNum; i++)
        {
            if (!isMoveDone(getMotorAxisID(i)))
            {
                bAllMotorDone = false;
                break;
            }
        }

        if (bAllMotorDone)
        {
            break;
        }

        QThread::msleep(10);

    } while (nTimeOut-- > 0);

    if (nTimeOut <= 0)
    {
        System->setTrackInfo(QStringLiteral("电机运动TimeOut！"));
        return false;
    }

    return true;
}

bool MotionControl::isHomed(int AxisID)
{
    return m_bHome[changeToMtrEnum(AxisID)];
}

bool MotionControl::isMoveDone(int AxisID)
{
    short sRtn = 0; // 指令返回值变量
    long lAxisStatus = 0; // 轴状态

    short bFlagMotion = FALSE; // 规划器运动标志

    // 读取轴状态
    sRtn = GT_GetSts(AxisID, &lAxisStatus);
    commandhandler("GT_GetSts", sRtn);

    // 规划器正在运动标志
    if (lAxisStatus & 0x400)
    {
        bFlagMotion = TRUE;        
    }
    else
    {
        bFlagMotion = FALSE;        
    }

    return !bFlagMotion;
}

bool MotionControl::isMoveLimit(int AxisID)
{
    short sRtn = 0; // 指令返回值变量
    long lAxisStatus = 0; // 轴状态

    bool bHitLimit = false;

    // 读取轴状态
    sRtn = GT_GetSts(AxisID, &lAxisStatus);
    commandhandler("GT_GetSts", sRtn);

    // 规划器正在运动标志
    if ((lAxisStatus & 0x20) == 0x20)
    {
        bHitLimit = true;
        //printf("Pos lmt is hit!\n");
    }
    if ((lAxisStatus & 0x40) == 0x40)
    {
        bHitLimit = true;
        //printf("Neg lmt is hit!\n");
    }
    if ((lAxisStatus & 0x10) == 0x10)
    {
        bHitLimit = true;
        //printf("PosErr OverLmt!\n");
    }

    return bHitLimit;
}

bool MotionControl::stopMove(int AxisID)
{
    short sRtn = 0; // 指令返回值变量

    long mask = 0;

    mask = 1 << (AxisID - 1);
    sRtn = GT_Stop(mask, 0);
    commandhandler("GT_Stop", sRtn);

    return 0 == sRtn;
}

bool MotionControl::EmStop(int AxisID)
{
    short sRtn = 0; // 指令返回值变量

    long mask = 0;

    mask = 1 << (AxisID - 1);
    sRtn = GT_Stop(mask, mask);
    commandhandler("GT_Stop", sRtn);

    return 0 == sRtn;
}

bool MotionControl::getCurrentPos(int AxisID, double *posMm)
{
    if (System->isRunOffline()) {
        *posMm = m_dRunOfflinePos[AxisID] * UM_TO_MM;
        return true;
    }

    short sRtn = 0; // 指令返回值变量

    double dMtrPos = 0;

    // 读取axis编码器位置
    sRtn = GT_GetEncPos(AxisID, &dMtrPos);
    commandhandler("GT_GetAxisEncPos", sRtn);
    
    //qDebug() << "get enc Pos: " << AxisID << " : " << dMtrPos;

    if (sRtn)
    {
        *posMm = 0;
        return false;
    }
    else
    {
        *posMm = convertToMm(changeToMtrEnum(AxisID), dMtrPos) * 2.0; // Unknow GT issue, that encoder position is less double times then real one.
        return true;
    }
}

QString MotionControl::getCurrentStatus(int AxisID)
{
    QString szStatus = QStringLiteral("未初始化");
    if (isEnabled(AxisID))
    {        
        if (isHomed(AxisID))
        {
            szStatus = QStringLiteral("已回零");
        }
        else
        {
            szStatus = QStringLiteral("已使能");
        }
    }
    else if (IsError(AxisID))
    {
        szStatus = QStringLiteral("驱动错误");
    }
    else if (IsLimit(AxisID))
    {
        szStatus = QStringLiteral("限位报警");
    }
    else if (IsEMStopError(AxisID))
    {
        szStatus = QStringLiteral("急停报警");
    }

    return szStatus;
}

void MotionControl::clearMotorParams()
{
    m_mtrParams.clear();
}

void MotionControl::addMotorParam(QMotorParam& mtrParam)
{
    m_mtrParams.push_back(mtrParam);
}

void MotionControl::updateMotorParam(int nID, QMotorParam& mtrParam)
{
    for (int i = 0; i < m_mtrParams.size(); i++)
    {
        if (m_mtrParams[i]._ID == nID)
        {
            m_mtrParams[i] = mtrParam;
            m_mtrParams[i]._ID = nID;
            break;
        }
    }
}

int MotionControl::getMotorParamsNum()
{
    return m_mtrParams.size();
}

QMotorParam& MotionControl::getMotorParam(int nID)
{
    for (int i = 0; i < m_mtrParams.size(); i++)
    {
        if (m_mtrParams[i]._ID == nID)
        {
            return m_mtrParams[i];
        }
    }

    return QMotorParam();
}

QMotorParam& MotionControl::getMotorParamByIndex(int nIndex)
{
    if (nIndex < 0 || nIndex >= m_mtrParams.size()) return QMotorParam();
    return m_mtrParams[nIndex];
}

void MotionControl::clearMotorProfiles()
{
    m_mtrMoveProfs.clear();
}

void MotionControl::addMotorProfile(QMtrMoveProfile& mtrMoveProf)
{
    m_mtrMoveProfs.push_back(mtrMoveProf);
}

void MotionControl::updateMotorProfile(int nID, QMtrMoveProfile& mtrMoveProf)
{
    for (int i = 0; i < m_mtrMoveProfs.size(); i++)
    {
        if (m_mtrMoveProfs[i]._ID == nID)
        {
            m_mtrMoveProfs[i] = mtrMoveProf;
            m_mtrMoveProfs[i]._ID = nID;
            break;
        }
    }
}

int MotionControl::getMotorProfilesNum()
{
    return m_mtrMoveProfs.size();
}

QMtrMoveProfile& MotionControl::getMotorProfile(int nID)
{
    for (int i = 0; i < m_mtrMoveProfs.size(); i++)
    {
        if (m_mtrMoveProfs[i]._ID == nID)
        {
            return m_mtrMoveProfs[i];
        }
    }

    return QMtrMoveProfile();
}

QMtrMoveProfile& MotionControl::getMotorProfileByIndex(int nIndex)
{
    if (nIndex < 0 || nIndex >= m_mtrMoveProfs.size()) return QMtrMoveProfile();
    return m_mtrMoveProfs[nIndex];
}

void MotionControl::removeMotorProfile(int nID)
{
    for (int i = 0; i < m_mtrMoveProfs.size(); i++)
    {
        if (m_mtrMoveProfs[i]._ID == nID)
        {
            m_mtrMoveProfs.removeAt(i);
            break;
        }
    }
}

int MotionControl::incrementMotorProfileID()
{
    return m_nMoveProfID++;
}

void MotionControl::setMotorProfileID(int nID)
{
    m_nMoveProfID = nID;
}

void MotionControl::clearMotorPoints()
{
    m_mtrMovePoints.clear();
}

void MotionControl::addMotorPoint(QMtrMovePoint& mtrMovePoint)
{
    m_mtrMovePoints.push_back(mtrMovePoint);
}

void MotionControl::updateMotorPoint(int nID, QMtrMovePoint& mtrMovePoint)
{
    for (int i = 0; i < m_mtrMovePoints.size(); i++)
    {
        if (m_mtrMovePoints[i]._ID == nID)
        {
            m_mtrMovePoints[i] = mtrMovePoint;
            m_mtrMovePoints[i]._ID = nID;
            break;
        }
    }
}

int MotionControl::getMotorPointsNum()
{
    return m_mtrMovePoints.size();
}

QMtrMovePoint& MotionControl::getMotorPoint(int nID)
{
    for (int i = 0; i < m_mtrMovePoints.size(); i++)
    {
        if (m_mtrMovePoints[i]._ID == nID)
        {
            return m_mtrMovePoints[i];
        }
    }

    return QMtrMovePoint();
}

QMtrMovePoint& MotionControl::getMotorPointByIndex(int nIndex)
{
    if (nIndex < 0 || nIndex >= m_mtrMovePoints.size()) return QMtrMovePoint();
    return m_mtrMovePoints[nIndex];
}

void MotionControl::removeMotorPoint(int nID)
{
    for (int i = 0; i < m_mtrMovePoints.size(); i++)
    {
        if (m_mtrMovePoints[i]._ID == nID)
        {
            m_mtrMovePoints.removeAt(i);
            break;
        }
    }
}

int MotionControl::incrementMotorPointID()
{
    return m_nMovePointID++;
}

void MotionControl::setMotorPointID(int nID)
{
    m_nMovePointID = nID;
}

void MotionControl::clearMotorPointGroups()
{
    m_mtrMovePointGroups.clear();
}

void MotionControl::addMotorPointGroup(QMtrMovePointGroup& mtrMovePointGroup)
{
    m_mtrMovePointGroups.push_back(mtrMovePointGroup);
}

void MotionControl::updateMotorPointGroup(int nID, QMtrMovePointGroup& mtrMovePointGroup)
{
    for (int i = 0; i < m_mtrMovePointGroups.size(); i++)
    {
        if (m_mtrMovePointGroups[i]._ID == nID)
        {
            m_mtrMovePointGroups[i] = mtrMovePointGroup;
            m_mtrMovePointGroups[i]._ID = nID;
            break;
        }
    }
}

int MotionControl::getMotorPointGroupNum()
{
    return m_mtrMovePointGroups.size();
}

QMtrMovePointGroup& MotionControl::getMotorPointGroup(int nID)
{
    for (int i = 0; i < m_mtrMovePointGroups.size(); i++)
    {
        if (m_mtrMovePointGroups[i]._ID == nID)
        {
            return m_mtrMovePointGroups[i];
        }
    }

    return QMtrMovePointGroup();
}

QMtrMovePointGroup& MotionControl::getMotorPointGroupByIndex(int nIndex)
{
    if (nIndex < 0 || nIndex >= m_mtrMovePointGroups.size()) return QMtrMovePointGroup();
    return m_mtrMovePointGroups[nIndex];
}

void MotionControl::removeMotorPointGroup(int nID)
{
    for (int i = 0; i < m_mtrMovePointGroups.size(); i++)
    {
        if (m_mtrMovePointGroups[i]._ID == nID)
        {
            m_mtrMovePointGroups.removeAt(i);
            break;
        }
    }
}

int MotionControl::incrementMotorPointGroupID()
{
    return m_nMovePointGroupID++;
}

void MotionControl::setMotorPointGroupID(int nID)
{
    m_nMovePointGroupID = nID;
}
