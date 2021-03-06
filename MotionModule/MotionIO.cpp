﻿#include "MotionIO.h"
#include "../common/SystemData.h"
#include "../include/IdDefine.h"
#include "ConfigData.h"

MotionIOOnLive::MotionIOOnLive()    
{
    m_bQuit = false;
    m_bRuning = false;
}

void MotionIOOnLive::run()
{
    m_bRuning = true;

    while (!m_bQuit)
    {
        emit UpdateMsg();

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

    connect(ui.checkBox_21, SIGNAL(stateChanged(int)), SLOT(updataOutPort0(int)));
    connect(ui.checkBox_22, SIGNAL(stateChanged(int)), SLOT(updataOutPort1(int)));
    connect(ui.checkBox_23, SIGNAL(stateChanged(int)), SLOT(updataOutPort2(int)));
    connect(ui.checkBox_24, SIGNAL(stateChanged(int)), SLOT(updataOutPort3(int)));
    connect(ui.checkBox_25, SIGNAL(stateChanged(int)), SLOT(updataOutPort4(int)));
    connect(ui.checkBox_26, SIGNAL(stateChanged(int)), SLOT(updataOutPort5(int)));
    connect(ui.checkBox_27, SIGNAL(stateChanged(int)), SLOT(updataOutPort6(int)));
    connect(ui.checkBox_28, SIGNAL(stateChanged(int)), SLOT(updataOutPort7(int)));
    connect(ui.checkBox_29, SIGNAL(stateChanged(int)), SLOT(updataOutPort8(int)));
    connect(ui.checkBox_30, SIGNAL(stateChanged(int)), SLOT(updataOutPort9(int)));
    connect(ui.checkBox_31, SIGNAL(stateChanged(int)), SLOT(updataOutPort10(int)));
    connect(ui.checkBox_32, SIGNAL(stateChanged(int)), SLOT(updataOutPort11(int)));
    connect(ui.checkBox_33, SIGNAL(stateChanged(int)), SLOT(updataOutPort12(int)));
    connect(ui.checkBox_34, SIGNAL(stateChanged(int)), SLOT(updataOutPort13(int)));

    connect(ui.pushButton_onLive, SIGNAL(clicked()), SLOT(onAutoLive()));
    connect(ui.pushButton_onStop, SIGNAL(clicked()), SLOT(onAutoStop()));
    ui.pushButton_onLive->setEnabled(true);
    ui.pushButton_onStop->setEnabled(false);

    connect(ui.pushButton_onCombTriggerOn, SIGNAL(clicked()), SLOT(onCombTriggerOn()));
    connect(ui.pushButton_onCombTriggerOff, SIGNAL(clicked()), SLOT(onCombTriggerOff()));

    //QVector<int> nPorts;    
    //nPorts.push_back(DO_LIGHT1_ENABLE);
    //nPorts.push_back(DO_LIGHT2_ENABLE);    
    //m_pCtrl->setDOs(nPorts, 1);
    //QThread::msleep(1000);
    //m_pCtrl->setDOs(nPorts, 0);

    //QThread::msleep(1000);
    //m_pCtrl->setDO(DO_LIGHT1_ENABLE, 1);
    //m_pCtrl->setDO(DO_LIGHT2_ENABLE, 1);
    //QThread::msleep(1000);
    //m_pCtrl->setDO(DO_LIGHT1_ENABLE, 0);
    //m_pCtrl->setDO(DO_LIGHT2_ENABLE, 0);
    //QThread::msleep(1000);
    //m_pCtrl->setDO(DO_LIGHT1_ENABLE, 0);
}

MotionIO::~MotionIO()
{
	onAutoStop();
}

void MotionIO::initDI()
{
    m_diIO[0] = Config->ID(DI_TRACK_ARRIVED);
    m_diIO[1] = Config->ID(DI_TRACK_READY);
    m_diIO[2] = Config->ID(DI_TRACK_STOP);
    m_diIO[3] = Config->ID(DI_TRACK_DELIVERED);
    m_diIO[4] = Config->ID(DI_IM_STOP);
    m_diIO[5] = Config->ID(DI_START);
    m_diIO[6] = Config->ID(DI_RESET);
    m_diIO[7] = Config->ID(DI_STOP);
    m_diIO[8] = 0;
    m_diIO[9] = 0;
    m_diIO[10] = 0;
    m_diIO[11] = 0;
    m_diIO[12] = 0;
    m_diIO[13] = 0;
    m_diIO[14] = 0;
    m_diIO[15] = 0;

    QCheckBox * checkBoxInput[16] = { ui.checkBox_5, ui.checkBox_6, ui.checkBox_7, ui.checkBox_8,
        ui.checkBox_9, ui.checkBox_10, ui.checkBox_11, ui.checkBox_12,
        ui.checkBox_13, ui.checkBox_14, ui.checkBox_15, ui.checkBox_16,
        ui.checkBox_17, ui.checkBox_18, ui.checkBox_19, ui.checkBox_20 };
    for (int i = 0; i < 16; i++)
    {
        QString name = Config->getInpName(m_diIO[i]);
        if (!name.isEmpty()) checkBoxInput[i]->setText(name);
    }
}

void MotionIO::initDO()
{   
    m_doIO[0] = Config->ID(DO_CAMERA_TRIGGER1);
    m_doIO[1] = Config->ID(DO_CAMERA_TRIGGER2);
    m_doIO[2] = Config->ID(DO_TRACK_CYLINDER);
    m_doIO[3] = 0;
    m_doIO[4] = Config->ID(DO_RED_LIGHT);
    m_doIO[5] = Config->ID(DO_YELLOW_LIGHT);
    m_doIO[6] = Config->ID(DO_GREEN_LIGHT);
    m_doIO[7] = Config->ID(DO_BUZZER);
    m_doIO[8] = Config->ID(DO_START);
    m_doIO[9] = Config->ID(DO_STOP);
    m_doIO[10] = Config->ID(DO_RESET);
    m_doIO[11] = Config->ID(DO_Z_BRAKE);
    m_doIO[12] = Config->ID(DO_STEPPER_RUN);
    m_doIO[13] = 0;
    m_doIO[14] = 0;
    m_doIO[15] = 0;

    QCheckBox * checkBoxOutput[16] = { ui.checkBox_21, ui.checkBox_22, ui.checkBox_23, ui.checkBox_24,
        ui.checkBox_25, ui.checkBox_26, ui.checkBox_27, ui.checkBox_28,
        ui.checkBox_29, ui.checkBox_30, ui.checkBox_31, ui.checkBox_32,
        ui.checkBox_33, ui.checkBox_34, ui.checkBox_35, ui.checkBox_36 };
    for (int i = 0; i < 16; i++)
    {
        QString name = Config->getOupName(m_doIO[i]);
        if (!name.isEmpty()) checkBoxOutput[i]->setText(name);
    }
}

void MotionIO::updataDI()
{
    bool val[16] = {false};

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

void MotionIO::updataOutPort0(int index)
{
    int portNO = m_doIO[0];
    int portvalue = 0;
    if (ui.checkBox_21->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
}


void MotionIO::updataOutPort1(int index)
{
    int portNO = m_doIO[1];
    int portvalue = 0;
    if (ui.checkBox_22->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
}


void MotionIO::updataOutPort2(int index)
{
    int portNO = m_doIO[2];
    int portvalue = 0;
    if (ui.checkBox_23->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
}


void MotionIO::updataOutPort3(int index)
{
    int portNO = m_doIO[3];
    int portvalue = 0;
    if (ui.checkBox_24->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
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

void MotionIO::updataOutPort6(int index)
{
    int portNO = m_doIO[6];
    int portvalue = 0;
    if (ui.checkBox_27->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
}


void MotionIO::updataOutPort7(int index)
{
    int portNO = m_doIO[7];
    int portvalue = 0;
    if (ui.checkBox_28->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
}


void MotionIO::updataOutPort8(int index)
{
    int portNO = m_doIO[8];
    int portvalue = 0;
    if (ui.checkBox_29->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
}


void MotionIO::updataOutPort9(int index)
{
    int portNO = m_doIO[9];
    int portvalue = 0;
    if (ui.checkBox_30->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
}


void MotionIO::updataOutPort10(int index)
{
    int portNO = m_doIO[10];
    int portvalue = 0;
    if (ui.checkBox_31->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
}

void MotionIO::updataOutPort11(int index)
{
    int portNO = m_doIO[11];
    int portvalue = 0;
    if (ui.checkBox_32->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
}

void MotionIO::updataOutPort12(int index)
{
    int portNO = m_doIO[12];
    int portvalue = 0;
    if (ui.checkBox_33->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
}

void MotionIO::updataOutPort13(int index)
{
    int portNO = m_doIO[13];
    int portvalue = 0;
    if (ui.checkBox_34->isChecked())portvalue = 1;

    m_pCtrl->setDO(portNO, portvalue);
}

void MotionIO::onAutoLive()
{
    if (!m_pThreadOnLive)
    {
        ui.pushButton_onLive->setEnabled(false);
        ui.pushButton_onStop->setEnabled(true);

        m_pThreadOnLive = new MotionIOOnLive();
        connect(m_pThreadOnLive, SIGNAL(UpdateMsg()), this, SLOT(updataIO()));
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

void MotionIO::onCombTriggerOn()
{
    QCheckBox * checkBoxTrigger[16] = { ui.checkBox_21, ui.checkBox_22, ui.checkBox_23, ui.checkBox_24,
        ui.checkBox_25, ui.checkBox_26, ui.checkBox_27, ui.checkBox_28, 
        ui.checkBox_29, ui.checkBox_30, ui.checkBox_31, ui.checkBox_32, 
        ui.checkBox_33, ui.checkBox_34, ui.checkBox_35, ui.checkBox_36 };

    QVector<int> nPorts;
    for (int i = 0; i < 16; i++)
    {    
        if (checkBoxTrigger[i]->isChecked())
        {
            int nPort = m_doIO[i];
            nPorts.push_back(nPort);
        }
    }

    m_pCtrl->setDOs(nPorts, 1);
}

void MotionIO::onCombTriggerOff()
{
    QCheckBox * checkBoxTrigger[16] = { ui.checkBox_21, ui.checkBox_22, ui.checkBox_23, ui.checkBox_24,
        ui.checkBox_25, ui.checkBox_26, ui.checkBox_27, ui.checkBox_28,
        ui.checkBox_29, ui.checkBox_30, ui.checkBox_31, ui.checkBox_32,
        ui.checkBox_33, ui.checkBox_34, ui.checkBox_35, ui.checkBox_36 };

    QVector<int> nPorts;
    for (int i = 0; i < 16; i++)
    {
        if (checkBoxTrigger[i]->isChecked())
        {
            int nPort = m_doIO[i];
            nPorts.push_back(nPort);
        }
    }

    m_pCtrl->setDOs(nPorts, 0);
}

