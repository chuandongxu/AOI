#include "MotionIO.h"
#include "../common/SystemData.h"
#include "../include/IdDefine.h"

MotionIOOnLive::MotionIOOnLive(MotionIO* pMotor)
	: m_pMotor(pMotor)
{
	m_bQuit = false;
	m_bRuning = false;
}

void MotionIOOnLive::run()
{
	m_bRuning = true;

	while (!m_bQuit)
	{
		m_pMotor->updataIO();

		if (m_bQuit)break;

		QThread::msleep(200);
	}

	m_bRuning = false;
}

MotionIO::MotionIO(MotionControl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	m_pThreadOnLive = NULL;

	this->initDI();
	this->initDO();

	updataIO();

	connect(ui.checkBox_25, SIGNAL(stateChanged(int)), SLOT(updataOutPort4(int)));
	connect(ui.checkBox_26, SIGNAL(stateChanged(int)), SLOT(updataOutPort5(int)));

	connect(ui.pushButton_onLive, SIGNAL(clicked()), SLOT(onAutoLive()));
	connect(ui.pushButton_onStop, SIGNAL(clicked()), SLOT(onAutoStop()));
	ui.pushButton_onLive->setEnabled(true);
	ui.pushButton_onStop->setEnabled(false);
}

MotionIO::~MotionIO()
{
	if (m_pThreadOnLive)
	{
		delete m_pThreadOnLive;
		m_pThreadOnLive = NULL;
	}
}

void MotionIO::initDI()
{
	m_diIO[0] = DI_IM_STOP;
	m_diIO[1] = DI_RESET;
	m_diIO[2] = DI_START;
	m_diIO[3] = DI_STOP;
	m_diIO[4] = 0;
	m_diIO[5] = 0;
	m_diIO[6] = 0;
	m_diIO[7] = 0;
	m_diIO[8] = 0;
	m_diIO[9] = 0;
	m_diIO[10] = 0;
	m_diIO[11] = 0;
	m_diIO[12] = 0;
	m_diIO[13] = 0;
	m_diIO[14] = 0;
	m_diIO[15] = 0;
}

void MotionIO::initDO()
{
	m_doIO[0] = DO_YELLOW_LIGHT;
	m_doIO[1] = DO_GREEN_LIGHT;
	m_doIO[2] = DO_RED_LIGHT;
	m_doIO[3] = DO_BUZZER;
	m_doIO[4] = DO_TRIGGER_DLP1;
	m_doIO[5] = DO_TRIGGER_DLP2;
	m_doIO[6] = DO_TRIGGER_DLP3;
	m_doIO[7] = DO_TRIGGER_DLP4;
	m_doIO[8] = 0;
	m_doIO[9] = 0;
	m_doIO[10] = 0;
	m_doIO[11] = 0;
	m_doIO[12] = 0;
	m_doIO[13] = 0;
	m_doIO[14] = 0;
	m_doIO[15] = 0;
}

void MotionIO::updataDI()
{
	bool val[16];

	for (int i = 0; i < 16; i++)
	{
		int iState = 0;
		m_pCtrl->getDI(m_diIO[i], iState);

		val[i] = iState > 0 ? true : false;
	}

	ui.checkBox_5->setChecked(val[0]);
	ui.checkBox_6->setChecked(val[1]);
	ui.checkBox_7->setChecked(val[2]);
	ui.checkBox_8->setChecked(val[3]);
	ui.checkBox_9->setChecked(val[4]);
	ui.checkBox_10->setChecked(val[5]);
	ui.checkBox_11->setChecked(val[6]);
	ui.checkBox_12->setChecked(val[7]);
	ui.checkBox_13->setChecked(val[8]);
	ui.checkBox_14->setChecked(val[9]);
	ui.checkBox_15->setChecked(val[10]);
	ui.checkBox_16->setChecked(val[11]);
	ui.checkBox_17->setChecked(val[12]);
	ui.checkBox_18->setChecked(val[13]);
	ui.checkBox_19->setChecked(val[14]);
	ui.checkBox_20->setChecked(val[15]);
}

void MotionIO::updataDO()
{
	bool val[16];

	for (int i = 0; i < 16; i++)
	{
		int iState = 0;
		m_pCtrl->getDO(m_doIO[i], iState);

		val[i] = iState > 0 ? true : false;
	}

	ui.checkBox_21->setChecked(val[0]);
	ui.checkBox_22->setChecked(val[1]);
	ui.checkBox_23->setChecked(val[2]);
	ui.checkBox_24->setChecked(val[3]);
	ui.checkBox_25->setChecked(val[4]);
	ui.checkBox_26->setChecked(val[5]);
	ui.checkBox_27->setChecked(val[6]);
	ui.checkBox_28->setChecked(val[7]);
	ui.checkBox_29->setChecked(val[8]);
	ui.checkBox_30->setChecked(val[9]);
	ui.checkBox_31->setChecked(val[10]);
	ui.checkBox_32->setChecked(val[11]);
	ui.checkBox_33->setChecked(val[12]);
	ui.checkBox_34->setChecked(val[13]);
	ui.checkBox_35->setChecked(val[14]);
	ui.checkBox_36->setChecked(val[15]);
}

void MotionIO::updataIO()
{
	this->updataDI();
	//this->updataDO();
}

void MotionIO::updataOutPort4(int index)
{
	int portNO = m_doIO[4];
	int portvalue = 0;
	if (ui.checkBox_25->isChecked())portvalue = 1;

	m_pCtrl->setDO(portNO, portvalue);
}

void MotionIO::updataOutPort5(int index)
{
	int portNO = m_doIO[5];
	int portvalue = 0;
	if (ui.checkBox_26->isChecked())portvalue = 1;

	m_pCtrl->setDO(portNO, portvalue);
}

void MotionIO::onAutoLive()
{
	if (!m_pThreadOnLive)
	{
		ui.pushButton_onLive->setEnabled(false);
		ui.pushButton_onStop->setEnabled(true);

		m_pThreadOnLive = new MotionIOOnLive(this);
		m_pThreadOnLive->start();
	}
}

void MotionIO::onAutoStop()
{
	if (m_pThreadOnLive)
	{
		m_pThreadOnLive->setQuitFlag();
		while (m_pThreadOnLive->isRuning())
		{
			QThread::msleep(10);
			QApplication::processEvents();
		}
		QThread::msleep(200);
		delete m_pThreadOnLive;
		m_pThreadOnLive = NULL;

		ui.pushButton_onLive->setEnabled(true);
		ui.pushButton_onStop->setEnabled(false);
	}
}

