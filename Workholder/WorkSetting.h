#pragma once

#include <QWidget>
#include "ui_WorkSetting.h"
#include "WorkCtrl.h"
#include <QThread>
#include <memory>

class WorkSetting;
class WorkSettingOnLive : public QThread
{
    Q_OBJECT

public:
    WorkSettingOnLive(WorkSetting* pSetting);
    ~WorkSettingOnLive(){};

public:
    void setQuitFlag(){ m_bQuit = true; }
    bool isRuning(){ return m_bRuning; };

private:
    void run();

private:
    WorkSetting*	  m_pSetting;
    bool          m_bQuit;
    bool          m_bRuning;
};

//using AutoUpdateIORunnablePtr = std::shared_ptr<WorkSettingOnLive>;

class WorkSetting : public QWidget
{
    Q_OBJECT

public:
    WorkSetting(WorkCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
    ~WorkSetting();

private:
    void initUI();

public:
    void updateIO();

private slots:
    void onAutoLive();
    void onAutoStop();

    void onEnable();   
    void onDisable();
    void onHome();
    void onImStop();
    void onClearError();

    void onCylinderUpDown(int iState);

    void onMove();
    void onStop();
    void onInputMove();
    void onOutputMove();

    void onTabWidgetChanged(int index);

    void onEnableWidth();
    void onDisableWidth();
    void onHomeWidth();
    void onImStopWidth();
    void onClearErrorWidth();

    void onJoystick();
    void onMoveToReady();
    void onMoveToWidth();

    void onTextChangedWidth(const QString &text);

private:
    Ui::WorkSetting ui;
    WorkCtrl* m_pCtrl;
    WorkSettingOnLive* m_pThreadOnLive;
};
