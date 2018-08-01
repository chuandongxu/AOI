#include "WorkCtrl.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IMotion.h"
#include "../include/constants.h"

InputProcessThread::InputProcessThread(WorkCtrl* pCtrl, QThreadPool *pThreadPool)
    : m_pCtrl(pCtrl), m_pThreadPool(pThreadPool)
{
    m_bQuit = false;  
}

void InputProcessThread::run()
{        
    if (m_bQuit) return;  
    m_pThreadPool->waitForDone();

    if (m_bQuit) return;
    // Wait Input Board Signal
    int nTimeOut = 100;
    while (nTimeOut-- > 0)
    {
        if (m_bQuit) break;

        if (m_pCtrl->IsBoardArrived())
        {
            break;
        }
        QThread::msleep(100);
    }

    if (nTimeOut <= 0)  return;
  
    if (m_bQuit) return;
    // Search Stop Sensor to move input board
    if (!m_pCtrl->searchSensor(WorkCtrl::Enum_Arrived, WorkCtrl::Trigger_Heigh, 0, 5000))
    {
        return;
    }

    if (m_bQuit) return;
    // Up the cylinder
    if (!m_pCtrl->upDownCylinder(true))
    {
        return;
    }
   
    if (m_bQuit) return;
    // Search Ready Sensor to move input board
    if (!m_pCtrl->IsBoardReady())
    {       
        if (!m_pCtrl->searchSensor(WorkCtrl::Enum_Ready, WorkCtrl::Trigger_Heigh, 1, 5000))
        {
            return;
        }
    }
}

OutputProcessThread::OutputProcessThread(WorkCtrl* pCtrl, QThreadPool *pThreadPool)
    : m_pCtrl(pCtrl), m_pThreadPool(pThreadPool)
{
    m_bQuit = false;
}

void OutputProcessThread::run()
{
    if (m_bQuit) return;
    m_pThreadPool->waitForDone();

    if (m_bQuit) return;
    // Down the cylinder
    if (!m_pCtrl->upDownCylinder(false))
    {
        return;
    }


    if (m_bQuit) return;
    // Search Delivered Sensor to move input board
    if (!m_pCtrl->searchSensor(WorkCtrl::Enum_Delivered, WorkCtrl::Trigger_Heigh, 0, 5000))
    {
        return;
    }

    if (m_bQuit) return;
    // Search Delivered Sensor to move input board
    if (!m_pCtrl->searchSensor(WorkCtrl::Enum_Arrived, WorkCtrl::Trigger_Low, 0, 5000))
    {
        return;
    }    
}

WorkCtrl::WorkCtrl(QObject *parent)
    : QObject(parent)
{
    m_threadPool.setMaxThreadCount(2);
}

WorkCtrl::~WorkCtrl()
{
}

bool WorkCtrl::IsBoardArrived()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);

    int iState = 0;
    bool bOK = pMotion->getDI(DI_TRACK_ARRIVED, iState);
    if (bOK)
    {
        return iState > 0;
    }
    return false;   
}

bool WorkCtrl::IsBoardDelivered()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);

    int iState = 0;
    bool bOK = pMotion->getDI(DI_TRACK_DELIVERED, iState);
    if (bOK)
    {
        return iState > 0;
    }
    return false;
}

bool WorkCtrl::IsBoardReady()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);

    int iState = 0;
    bool bOK = pMotion->getDI(DI_TRACK_READY, iState);
    if (bOK)
    {
        return iState > 0;
    }
    return false;
}

bool WorkCtrl::IsBoardStop()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);

    int iState = 0;
    bool bOK = pMotion->getDI(DI_TRACK_STOP, iState);
    if (bOK)
    {
        return iState > 0;
    }
    return false;
}

bool WorkCtrl::upDownCylinder(bool bUp)
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);

    int iState = bUp ? 1 : 0;
    return pMotion->setDO(DO_TRACK_CYLINDER, iState);
}

bool WorkCtrl::searchSensor(PortEnum port, TriggerEnum trig, int nProfile, double dMoveLimit)
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);

    if (!pMotion->move(AXIS_MOTOR_TRACKING, nProfile, dMoveLimit, false)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("移动进料马达错误."));
        return false;
    }

    bool bSearched = false;
    while (!pMotion->isMoveDone(AXIS_MOTOR_TRACKING))
    {
        int iState = 0;
        switch (port)
        {
        case WorkCtrl::Enum_Arrived:
            pMotion->getDI(DI_TRACK_ARRIVED, iState);
            break;
        case WorkCtrl::Enum_Delivered:
            pMotion->getDI(DI_TRACK_DELIVERED, iState);
            break;
        case WorkCtrl::Enum_Stop:
            pMotion->getDI(DI_TRACK_STOP, iState);
            break;
        case WorkCtrl::Enum_Ready:
            pMotion->getDI(DI_TRACK_READY, iState);
            break;
        default:
            break;
        }

        if (0 != iState && Trigger_Heigh == trig)
        {
            bSearched = true;
            break;
        }
        else  if (0 == iState && Trigger_Low == trig)
        {
            bSearched = true;
            break;
        }
    }
    
    if (!pMotion->isMoveDone(AXIS_MOTOR_TRACKING))
    {
        pMotion->stopMove(AXIS_MOTOR_TRACKING);
    }

    return bSearched;
}

bool WorkCtrl::moveInput(bool bWait)
{
    auto pInputProcessRunnable = std::make_shared<InputProcessThread>(this, &m_threadPool);
    pInputProcessRunnable->setAutoDelete(false);
    m_threadPool.start(pInputProcessRunnable.get());

    if(bWait) m_threadPool.waitForDone();

    return true;
}

bool WorkCtrl::moveOutput(bool bWait)
{
    auto pOutputProcessRunnable = std::make_shared<OutputProcessThread>(this, &m_threadPool);
    pOutputProcessRunnable->setAutoDelete(false);
    m_threadPool.start(pOutputProcessRunnable.get());

    if (bWait) m_threadPool.waitForDone();

    return true;
}
