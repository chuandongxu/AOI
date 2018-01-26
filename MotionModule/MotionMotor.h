#pragma once

#include <QWidget>
#include "ui_MotionMotor.h"
#include "MotionControl.h"
#include <QThread>
#include <qstandarditemmodel>

class MotionMotor;
class MotionMotorOnLive : public QThread
{
	Q_OBJECT

public:
	MotionMotorOnLive(MotionMotor* pMotor);
	~MotionMotorOnLive(){};

public:
	void setQuitFlag(){ m_bQuit = true; }
	bool isRuning(){ return m_bRuning; };

private:
	void run();

private:
	MotionMotor*  m_pMotor;
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
	//void saveConfig();

private:
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

public:
	void updataStatus();

private slots:
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

private:
	void getAxisSelected(bool* axisIDs, int axisNum);

private:
	Ui::MotionMotor ui;
	MotionControl* m_pCtrl;
	MotionMotorOnLive *m_pThreadOnLive;
	QStandardItemModel m_moveProfModel;
	QStandardItemModel m_movePointModel;
	QStandardItemModel m_movePtGroupModel;
};
