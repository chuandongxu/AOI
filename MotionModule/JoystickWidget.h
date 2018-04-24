#pragma once

#include <QWidget>
#include "ui_JoystickWidget.h"
#include <qmutex.h>

enum MoveDir
{
	MOVE_DIR_LEFT,
	MOVE_DIR_RIGHT,
	MOVE_DIR_UP,
	MOVE_DIR_DOWN,
};

class MotionControl;
class JoystickWidget : public QWidget
{
	Q_OBJECT

public:
	JoystickWidget(MotionControl* pCtrl, QWidget *parent = Q_NULLPTR);
	~JoystickWidget();

	void clearJoystick();
	void SetXMotor(int AxisID, double dStep, QLineEdit *pEdit);
	void SetYMotor(int AxisID, double dStep, QLineEdit *pEdit);
	void UpdateResult();

protected:
	void closeEvent(QCloseEvent *e);

private:
	virtual void timerEvent(QTimerEvent *event);

private slots:
	void onEnable();
	void onHome();

	void onLeft();
	void onRight();
	void onUp();
	void onDown();

	void onOk();
	void onCancel();

private:
	void updateMtrStatus();
	void setMoveDir(MoveDir dir);
	bool IsDirChanged(MoveDir dir);
	void preMove(MoveDir dir);
	void postMove(MoveDir dir);

private:
	Ui::JoystickWidget ui;
	MotionControl* m_pCtrl;
	int m_nTimerID;
	QMutex m_mutex;

	MoveDir m_emLastDir;
	int m_nJoystick;

	int m_AxisID_X;
	int m_AxisID_Y;

	double m_dStepX;
	double m_dStepY;

    int m_nProfileId = 0;

	QLineEdit *m_pEditX;
	QLineEdit *m_pEditY;
};
