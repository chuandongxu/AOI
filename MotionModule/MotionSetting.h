#pragma once

#include <QWidget>
#include "ui_MotionSetting.h"
#include "MotionControl.h"
#include <QTimer>

#include "MotionIO.h"
#include "MotionMotor.h"

class MotionSetting : public QWidget
{
	Q_OBJECT

public:
	MotionSetting(MotionControl* pCtrl, QWidget *parent = Q_NULLPTR);
	~MotionSetting();

private:
	void initUI();

private slots:
	void onTriggerNumIndexChanged(int iIndex);
	void onTriggerSave();

private:
	Ui::MotionSetting ui;
	//QTimer *m_timer;
	MotionControl* m_pCtrl;

	MotionIO* m_pMotionIO;
	MotionMotor* m_pMotionMtr;
};
