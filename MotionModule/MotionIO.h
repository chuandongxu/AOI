#pragma once

#include <QWidget>
#include "ui_MotionIO.h"
#include "MotionControl.h"
#include <QThread>

class MotionIO;
class MotionIOOnLive : public QThread
{
    Q_OBJECT

public:
    MotionIOOnLive(MotionIO* pMotor);
    ~MotionIOOnLive(){};

public:
    void setQuitFlag(){ m_bQuit = true; }
    bool isRuning(){ return m_bRuning; };

private:
    void run();

private:
    MotionIO*      m_pMotor;
    bool          m_bQuit;
    bool          m_bRuning;
};

class MotionIO : public QWidget
{
    Q_OBJECT

public:
    MotionIO(MotionControl* pCtrl, QWidget *parent = Q_NULLPTR);
    ~MotionIO();

public:
	void updataIO();

private slots:
    void updataOutPort0(int index);
    void updataOutPort1(int index);
    void updataOutPort2(int index);
    void updataOutPort3(int index);
    void updataOutPort4(int index);
    void updataOutPort5(int index);
    void updataOutPort6(int index);
    void updataOutPort7(int index);
    void updataOutPort8(int index);
    void updataOutPort9(int index);
    void updataOutPort10(int index);
    void updataOutPort11(int index);
    void updataOutPort12(int index);
    void updataOutPort13(int index);

    void onAutoLive();
    void onAutoStop();

    void onCombTriggerOn();
    void onCombTriggerOff();

protected:
    void initDI();
    void initDO();

    void updataDI();
    void updataDO();

private:
    MotionControl* m_pCtrl;
    MotionIOOnLive *m_pThreadOnLive;

    int m_diIO[16];
    int m_doIO[16];

private:
    Ui::MotionIO ui;
};
