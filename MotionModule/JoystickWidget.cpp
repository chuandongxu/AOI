#include "JoystickWidget.h"
#include "../common/SystemData.h"
#include "../include/IdDefine.h"
#include "../Common/ThreadPrioc.h"
#include "../include/constants.h"
#include "MotionControl.h"

#include <QDebug>
#include <QTimerEvent>

#define TIMER_TIMEOUT (500)

JoystickWidget::JoystickWidget(MotionControl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	m_nJoystick = 0;
	m_emLastDir = MOVE_DIR_LEFT;

	connect(ui.pushButton_enable, SIGNAL(clicked()), SLOT(onEnable()));
	connect(ui.pushButton_home, SIGNAL(clicked()), SLOT(onHome()));
	connect(ui.pushButton_left, SIGNAL(clicked()), SLOT(onLeft()));
	connect(ui.pushButton_right, SIGNAL(clicked()), SLOT(onRight()));
	connect(ui.pushButton_up, SIGNAL(clicked()), SLOT(onUp()));
	connect(ui.pushButton_down, SIGNAL(clicked()), SLOT(onDown()));

	connect(ui.pushButton_ok, SIGNAL(clicked()), SLOT(onOk()));
	connect(ui.pushButton_cancel, SIGNAL(clicked()), SLOT(onCancel()));

	ui.pushButton_left->setAutoRepeat(true);
	ui.pushButton_left->setAutoRepeatDelay(50);
	ui.pushButton_left->setAutoRepeatInterval(200);

	ui.pushButton_right->setAutoRepeat(true);
	ui.pushButton_right->setAutoRepeatDelay(50);
	ui.pushButton_right->setAutoRepeatInterval(200);

	ui.pushButton_up->setAutoRepeat(true);
	ui.pushButton_up->setAutoRepeatDelay(50);
	ui.pushButton_up->setAutoRepeatInterval(200);

	ui.pushButton_down->setAutoRepeat(true);
	ui.pushButton_down->setAutoRepeatDelay(50);
	ui.pushButton_down->setAutoRepeatInterval(200);	

	clearJoystick();

	m_nTimerID = this->startTimer(TIMER_TIMEOUT);
}

JoystickWidget::~JoystickWidget()
{
}

void JoystickWidget::closeEvent(QCloseEvent *e) {
	clearJoystick();
	this->hide();
}

void JoystickWidget::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_nTimerID)
	{
		updateMtrStatus();

		if (m_nJoystick > 0) m_nJoystick--;
	}
}

void JoystickWidget::clearJoystick()
{
	m_AxisID_X = 0;
	m_dStepX = 0;
	m_pEditX = NULL;

	m_AxisID_Y = 0;
	m_dStepY = 0;
	m_pEditY = NULL;

	ui.pushButton_left->setEnabled(false);
	ui.pushButton_right->setEnabled(false);
	ui.pushButton_up->setEnabled(false);
	ui.pushButton_down->setEnabled(false);
}

void JoystickWidget::SetXMotor(int AxisID, double dStep, QLineEdit *pEdit)
{
	m_AxisID_X = AxisID;
	m_dStepX = dStep;
	m_pEditX = pEdit;

	ui.pushButton_left->setEnabled(true);
	ui.pushButton_right->setEnabled(true);
}

void JoystickWidget::SetYMotor(int AxisID, double dStep, QLineEdit *pEdit)
{
	m_AxisID_Y = AxisID;
	m_dStepY = dStep;
	m_pEditY = pEdit;

	ui.pushButton_up->setEnabled(true);
	ui.pushButton_down->setEnabled(true);
}

void JoystickWidget::UpdateResult()
{
	if (m_pEditX) m_pEditX->setText(ui.lineEdit_encX->text());
	if (m_pEditY) m_pEditY->setText(ui.lineEdit_encY->text());
}

void JoystickWidget::updateMtrStatus()
{
	QAutoLocker loacker(&m_mutex);

	if (m_pCtrl)
	{
		double dMtrPosX = 0, dMtrPosY = 0;
		if (m_AxisID_X > 0) m_pCtrl->getCurrentPos(m_AxisID_X, &dMtrPosX);
		if (m_AxisID_Y > 0) m_pCtrl->getCurrentPos(m_AxisID_Y, &dMtrPosY);

		ui.lineEdit_encX->setText(QString::number(dMtrPosX, 'f', 2));
		ui.lineEdit_encY->setText(QString::number(dMtrPosY, 'f', 2));

		ui.lineEdit_stepX->setText(QString::number(m_dStepX, 'f', 2));
		ui.lineEdit_stepY->setText(QString::number(m_dStepY, 'f', 2));
	}
}

void JoystickWidget::setMoveDir(MoveDir dir)
{
	m_emLastDir = dir;
}

bool JoystickWidget::IsDirChanged(MoveDir dir)
{
	return m_emLastDir != dir;
}

void JoystickWidget::preMove(MoveDir dir)
{
	if (IsDirChanged(dir))
	{
		if (m_AxisID_X > 0) m_pCtrl->stopMove(m_AxisID_X);
		if (m_AxisID_Y > 0) m_pCtrl->stopMove(m_AxisID_Y);
		m_nJoystick = 0;
	}
	if (m_nJoystick < 100) m_nJoystick += 1;
}

void JoystickWidget::postMove(MoveDir dir)
{
	setMoveDir(dir);
}

void JoystickWidget::onEnable()
{
	QAutoLocker loacker(&m_mutex);

	if (m_AxisID_X > 0 && !m_pCtrl->enable(m_AxisID_X))
	{
		System->setTrackInfo(QStringLiteral("Enable Motor Error!, AxisID=%1").arg(m_AxisID_X));
	}

	if (m_AxisID_Y > 0 && !m_pCtrl->enable(m_AxisID_Y))
	{
		System->setTrackInfo(QStringLiteral("Enable Motor Error!, AxisID=%1").arg(m_AxisID_Y));
	}

	System->setTrackInfo(QStringLiteral("Motor Enabled!"));
}

void JoystickWidget::onHome()
{
	QAutoLocker loacker(&m_mutex);

	if (m_AxisID_X > 0 && !m_pCtrl->home(m_AxisID_X, true))
	{
		System->setTrackInfo(QStringLiteral("Home Motor Error!, AxisID=%1").arg(m_AxisID_X));
	}

	if (m_AxisID_Y > 0 && !m_pCtrl->home(m_AxisID_Y, true))
	{
		System->setTrackInfo(QStringLiteral("Home Motor Error!, AxisID=%1").arg(m_AxisID_Y));
	}

	System->setTrackInfo(QStringLiteral("Motor Homed!"));
}

void JoystickWidget::onLeft()
{
	QAutoLocker loacker(&m_mutex);

	preMove(MOVE_DIR_LEFT);

	m_pCtrl->move(m_AxisID_X, m_nProfileId, -m_dStepX * m_nJoystick, m_bSyncMotion);

	postMove(MOVE_DIR_LEFT);
}

void JoystickWidget::onRight()
{
	QAutoLocker loacker(&m_mutex);

	preMove(MOVE_DIR_RIGHT);

	m_pCtrl->move(m_AxisID_X, m_nProfileId, m_dStepX * m_nJoystick, m_bSyncMotion);

	postMove(MOVE_DIR_RIGHT);
}

void JoystickWidget::onUp()
{
	QAutoLocker loacker(&m_mutex);

	preMove(MOVE_DIR_UP);

	m_pCtrl->move(m_AxisID_Y, m_nProfileId, m_dStepY * m_nJoystick, m_bSyncMotion);

	postMove(MOVE_DIR_UP);
}

void JoystickWidget::onDown()
{
	QAutoLocker loacker(&m_mutex);

	preMove(MOVE_DIR_DOWN);

	m_pCtrl->move(m_AxisID_Y, m_nProfileId, -m_dStepY * m_nJoystick, m_bSyncMotion);

	postMove(MOVE_DIR_DOWN);
}

void JoystickWidget::onOk()
{
	QAutoLocker loacker(&m_mutex);

	this->UpdateResult();
	this->hide();
	clearJoystick();
}

void JoystickWidget::onCancel()
{
	QAutoLocker loacker(&m_mutex);

	this->hide();
	clearJoystick();
}
