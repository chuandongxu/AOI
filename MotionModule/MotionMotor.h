#pragma once

#include <QWidget>
#include "ui_MotionMotor.h"
#include "MotionControl.h"
#include <QThread>
#include <qstandarditemmodel>

class MotionMotorOnLive : public QThread
{
    Q_OBJECT

public:
    MotionMotorOnLive();
    ~MotionMotorOnLive(){};

signals:
    void UpdateMsg();

public:
    void setQuitFlag(){ m_bQuit = true; }
    bool isRuning(){ return m_bRuning; };

private:
    void run();

private:
    bool          m_bQuit;
    bool          m_bRuning;
};

class MotionMotor : public QWidget
{
    Q_OBJECT

public:
    MotionMotor(MotionControl* pCtrl, QWidget *parent = Q_NULLPTR);
    ~MotionMotor();    

private:
    void initUI();
    void loadConfig();
    void updateUI();

    void loadMotorConfig();
    void updateMotorUI();
    void saveMotorConfig();

    void loadMtrProfConfig();
    void updateMtrProfData();
    void saveMtrProfConfig();

    void loadMtrPointConfig();
    void updateMtrPointData();
    void saveMtrPointConfig();

    void loadMtrPointGroupConfig();
    void updateMtrPointGroupData();
    void updateMtrPointGroupListData(int nIndex);
    void saveMtrPointGroupConfig();

    void getAxisSelected(bool* axisIDs, int axisNum);
	QMtrHomeProfile::HomeMode getAxisHomeMode(int nAxisID);

private slots:
	void updataStatus();

    void onEnable();
    void onHome();
    void onDisable();
    void onImStop();
    void onClearError();
    void onMoveRel();
    void onMoveAbs();
    void onStop();
    void onParamSave();

    void onAutoLive();
    void onAutoStop();

    void onAddProf();
    void onDelProf();
    void onProfSave();

    void onAddPoint();
    void onDelPoint();
    void onPointMove();
    void onPointSave();

    void onPointGroupIndexChanged(int nIndex);
    void onAddPointGroup();
    void onDelPointGroup();    
    void onPointGroupSave();

    void onAddPointGroupPt();
    void onDelPointGroupPt();

    void on_btnConvertVelocityToMotorUnit_clicked();
    void on_btnConvertVelocityToStandardUnit_clicked();
    void on_btnConvertAccelerationToMotorUnit_clicked();
    void on_btnConvertAccelerationToStandardUnit_clicked();

private:
    Ui::MotionMotor ui;
    MotionControl* m_pCtrl;
    MotionMotorOnLive *m_pThreadOnLive;
    QStandardItemModel m_moveProfModel;
    QStandardItemModel m_movePointModel;
    QStandardItemModel m_movePtGroupModel;
};
