#pragma once

#include <QObject>
#include <QThreadPool>
#include <qthread.h>

#include <QRunnable>
#include <memory>

class WorkCtrl;
class InputProcessThread : public QRunnable
{
public:
    InputProcessThread(WorkCtrl* pCtrl, QThreadPool *pThreadPool);
    ~InputProcessThread(){};

protected:
    virtual void run() override;

public:
    void setQuitFlag(){ m_bQuit = true; }

private:
    WorkCtrl*	  m_pCtrl;
    QThreadPool*  m_pThreadPool;
    bool          m_bQuit;  
};

class OutputProcessThread : public QRunnable
{
public:
    OutputProcessThread(WorkCtrl* pCtrl, QThreadPool *pThreadPool);
    ~OutputProcessThread(){};

protected:
    virtual void run() override;

public:
    void setQuitFlag(){ m_bQuit = true; }

private:
    WorkCtrl*	  m_pCtrl;
    QThreadPool*  m_pThreadPool;
    bool          m_bQuit;
};

//using InputProcessRunnablePtr = std::shared_ptr<InputProcessThread>;
//using OutputProcessRunnablePtr = std::shared_ptr<OutputProcessThread>;

class WorkCtrl : public QObject
{
    Q_OBJECT

public:
    WorkCtrl(QObject *parent = NULL);
    ~WorkCtrl();

    enum PortEnum
    {
        Enum_Arrived,
        Enum_Delivered,
        Enum_Stop,
        Enum_Ready
    };

    enum TriggerEnum
    {
        Trigger_Low,
        Trigger_Heigh
    };

public:
    bool IsBoardArrived();
    bool IsBoardDelivered();
    bool IsBoardReady();
    bool IsBoardStop();

    bool upDownCylinder(bool bUp);

    bool moveInput(bool bWait);
    bool moveOutput(bool bWait);


    bool searchSensor(PortEnum port, TriggerEnum trig, int nProfile, double dMoveLimit = 10000);

private:
    QThreadPool m_threadPool;
};
