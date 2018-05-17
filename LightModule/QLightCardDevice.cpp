#include "QLightCardDevice.h"
#include "../Common/SystemData.h"
#include "LightDefine.h"
#include <qvariant>
#include <QtSerialPort\\QSerialPortInfo>
#include <qdebug.h>
#include <qthread.h>
#include <qmutex.h>

#include "CommPort.h"

#define ToInt(value)                (static_cast<int>(value))

QLightCardDevice::QLightCardDevice(const QString & devName, int nChnNum, QObject *parent)
    : QLightDevice(devName, nChnNum, parent)
{
}

QLightCardDevice::~QLightCardDevice()
{

}

void QLightCardDevice::setChLuminance(int ch, int luminance)
{
    m_data[ch].iLuminance = luminance;
 
}

void QLightCardDevice::setupTrigger(ILight::TRIGGER emTrig)
{
    m_emTrig = emTrig;

    QString szCmd;
   
    //设置Light曝光参数
    const double dLightExposure = System->getParam("motion_trigger_light_exposure").toDouble();
    const double dLightPeriod = System->getParam("motion_trigger_light_period").toDouble() * 100;

    int nCaptureLightNum = _CHN_NUM;
    int nLightPeriod = ToInt(dLightPeriod);
    int lums[_CHN_NUM];
    for (int i = 0; i < _CHN_NUM; i++)
    {
        lums[i] = ToInt(getChLuminance(i) / 10.0);
    }

    int plus[_CHN_NUM] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20 };

    //设置DLP曝光参数
    const double dPatternExposure = System->getParam("motion_trigger_pattern_exposure").toDouble() * 100;
    const double dPatternPeriod = System->getParam("motion_trigger_pattern_period").toDouble() * 100;
    const int nPatternNumConst = System->getParam("motion_trigger_pattern_num").toInt();
    const int nStationNumConst = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;

    int nPatternNum = nPatternNumConst;
    int nStationNum = nStationNumConst;
    int nPatternExposure = ToInt(dPatternExposure);
    int nPatternPeriod = ToInt(dPatternPeriod);

    //根据触发模式，设置不同的参数
    switch (m_emTrig)
    {
    case ILight::TRIGGER_ALL:
       break;
    case ILight::TRIGGER_DLP:
    {  
        nCaptureLightNum = 0;
    }
    break;
    case ILight::TRIGGER_DLP1:
    {
    }
    break;
    case ILight::TRIGGER_DLP2:
    {
    }
    break;
    case ILight::TRIGGER_DLP3:
    {
    }
    break;
    case ILight::TRIGGER_DLP4:
    {        
    }
    break;
    case ILight::TRIGGER_LIGHT:
    { 
        nStationNum = 0;
    }
    break;
    case ILight::TRIGGER_ONE_CH1:
    {
        nCaptureLightNum = 1;
        nStationNum = 0;
        for (int i = 0; i < _CHN_NUM; i++)
        {
            plus[i] = 0;
        }
        plus[0] = 0x01;
    }
    break;
    case ILight::TRIGGER_ONE_CH2:
    {
        nCaptureLightNum = 1;
        nStationNum = 0;
        for (int i = 0; i < _CHN_NUM; i++)
        {
            plus[i] = 0;
        }
        plus[0] = 0x02;
    }
    break;
    case ILight::TRIGGER_ONE_CH3:
    {
        nCaptureLightNum = 1;
        nStationNum = 0;
        for (int i = 0; i < _CHN_NUM; i++)
        {
            plus[i] = 0;
        }
        plus[0] = 0x04;
    }
    break;
    case ILight::TRIGGER_ONE_CH4:
    {
        nCaptureLightNum = 1;
        nStationNum = 0;
        for (int i = 0; i < _CHN_NUM; i++)
        {
            plus[i] = 0;
        }
        plus[0] = 0x08;
    }
    break;
    case ILight::TRIGGER_ONE_CH5:
    {
        nCaptureLightNum = 1;
        nStationNum = 0;
        for (int i = 0; i < _CHN_NUM; i++)
        {
            plus[i] = 0;
        }
        plus[0] = 0x10;
    }
    break;
    case ILight::TRIGGER_ONE_CH6:
    { 
        nCaptureLightNum = 1;
        nStationNum = 0;
        for (int i = 0; i < _CHN_NUM; i++)
        {
            plus[i] = 0;
        }
        plus[0] = 0x20;
    }
    break;
    default:
        break;
    }

    //灯光参数设定
    szCmd = "SetT" + QString::number(0) + " " + QString::number(nLightPeriod) + "\r\n";
    writeCmd(szCmd);

    //szCmd = "Set" + QString("t") + " " + QString::number(1000) + "\r\n";
    //writeCmd(szCmd);

    szCmd = "SetN" + QString::number(0) + " " + QString::number(nCaptureLightNum) + "\r\n";
    writeCmd(szCmd);

    //设置灯光亮度电流值
    for (int i = 0; i < _CHN_NUM; i++)
    {
        szCmd = "SetPWM" + QString::number(i) + " " + QString::number(lums[i]) + "\r\n";
        writeCmd(szCmd);
        QThread::msleep(100);
    }   

    //设置Light灯光组合    
    for (int i = 0; i < _CHN_NUM; i++)
    {
        szCmd = "SetPlus" + QString::number(i) + " " + QString::number(plus[i]) + "\r\n";
        writeCmd(szCmd);
        QThread::msleep(100);
    }

    //DLP参数设定
    szCmd = "SetPWM" + QString::number(9) + " " + QString::number(nPatternExposure) + "\r\n";
    writeCmd(szCmd);

    szCmd = "SetT" + QString::number(1) + " " + QString::number(nPatternPeriod) + "\r\n";
    writeCmd(szCmd);
    
    for (int i = 0; i < nStationNumConst; i++)
    {
        szCmd = "SetOut" + QString::number(i) + " " + QString::number(nPatternNum) + "\r\n";
        writeCmd(szCmd);
        QThread::msleep(100);
    }

    szCmd = "SetN" + QString::number(1) + " " + QString::number(nStationNum) + "\r\n";
    writeCmd(szCmd);  
  
    //设置控制参数
    szCmd = "SetIdle" + QString("") + " " + QString::number(200) + "\r\n";
    writeCmd(szCmd);

    szCmd = "SetCh" + QString::number(8) + " " + QString::number(1) + "\r\n";
    writeCmd(szCmd);
}

bool QLightCardDevice::trigger()
{
    QString szCmd;

    szCmd = "SetCh" + QString::number(8) + " " + QString::number(0) + "\r\n";
    writeCmd(szCmd);

    szCmd = "Start" + QString("") + " " + QString::number(getPatternNum()) + "\r\n";
    writeCmd(szCmd);

    return true;
}

void QLightCardDevice::writeCmd(const QString& szCmd)
{
    if (m_comPort)
    {
        m_comPort->write(szCmd.toLocal8Bit());

        QByteArray readLine;
        if (m_comPort->readSyn(readLine))
        {
            QString value = readLine;
            if (!value.trimmed().isEmpty())
            {                
               qDebug() << "LiCard, rev = " + value;
            }           
        }   
        else
        {
            System->setTrackInfo("read com error!");
        }
    }
}

int QLightCardDevice::getPatternNum()
{
    int nPatternNum = 0;

    switch (m_emTrig)
    {
    case ILight::TRIGGER_ALL:
    {
        nPatternNum = System->getParam("motion_trigger_pattern_num").toInt();
        int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
        nPatternNum *= nStationNum;
        nPatternNum += _CHN_NUM;
    }
    break;
    case ILight::TRIGGER_DLP:
    {
        nPatternNum = System->getParam("motion_trigger_pattern_num").toInt();
        int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
        nPatternNum *= nStationNum;
    }
    break;
    case ILight::TRIGGER_DLP1:
    case ILight::TRIGGER_DLP2:
    case ILight::TRIGGER_DLP3:
    case ILight::TRIGGER_DLP4:
    {
        nPatternNum = System->getParam("motion_trigger_pattern_num").toInt();
    }
    break;
    case ILight::TRIGGER_LIGHT:
    {

        nPatternNum = _CHN_NUM;
    }
    break;
    case ILight::TRIGGER_ONE_CH1:
    case ILight::TRIGGER_ONE_CH2:
    case ILight::TRIGGER_ONE_CH3:
    case ILight::TRIGGER_ONE_CH4:
    case ILight::TRIGGER_ONE_CH5:
    case ILight::TRIGGER_ONE_CH6:
    {
        nPatternNum = 1;
    }
    break;
    default:
        break;
    }

    return nPatternNum;
}