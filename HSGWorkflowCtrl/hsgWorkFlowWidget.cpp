#include "hsgWorkFlowWidget.h"
#include "../include/workflowDefine.h"
#include "../Common/eos.h"
#include "../include/IFlowCtrl.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../Common/SystemData.h"
#include "../include/ICamera.h"
#include "../include/IVisionUI.h"
#include "hsgworkflowctrl_global.h"
#include <QMessagebox>
#include <qdesktopwidget.h>


#define _NORMAL_WIN_WIDTH_ 1440
#define ALARMTEST 200

QWorkFlowWidget::QWorkFlowWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	initStationState();

	QEos::Attach(EVENT_CHECK_STATE,this,SLOT(onCheckerStateChange(const QVariantList &)));
	QEos::Attach(EVENT_BARCODE_CHANGE,this,SLOT(onBarCodeChange(const QVariantList &)));
	QEos::Attach(EVENT_AI_STATE,this,SLOT(onAIStateChange(const QVariantList &)));	
	QEos::Attach(EVENT_CHANGE_USER,this,SLOT(onChangeUser(const QVariantList &)));

	IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);

	m_subVLayout = new QHBoxLayout;
	QWidget* pWidget = pUI->getVisionView();
	m_subVLayout->addWidget(pWidget);;
	pWidget->setFixedSize(1200, 800);
	m_subVLayout->setAlignment(Qt::AlignTop);
	
	
	m_mainLayout = new QVBoxLayout;
	m_mainLayout->setContentsMargins(0, 0, 0, 0);
	m_mainLayout->addLayout(m_subVLayout);
	this->setLayout(m_mainLayout);

	
	QPalette Pal(palette());
	// set black background
	Pal.setColor(QPalette::Background, QColor(245,245,245));	
	setAutoFillBackground(true);
	setPalette(Pal);

	m_nTimerID = this->startTimer(800);	

	if(USER_LEVEL_TECH > System->getUserLevel())
	{
		
	}		
}

QWorkFlowWidget::~QWorkFlowWidget()
{
	this->killTimer(m_nTimerID);
}

void QWorkFlowWidget::onCheckerStateChange(const QVariantList &vars)
{
	if(vars.size() < 3)return;

	int iStation = vars[0].toInt();
	int iEvent = vars[1].toInt();
	int iState = vars[2].toInt();
	double dValue1 = vars.size()>3 ? vars[3].toDouble():0;	

	if (0 == iStation)setCheckerState1(iEvent, iState, dValue1);	
}

void QWorkFlowWidget::onBarCodeChange(const QVariantList &vars)
{
	if(vars.size() < 3)return;

	int iStation = vars[0].toInt();
	QString barcode = vars[2].toString();	
}

void QWorkFlowWidget::onChangeUser(const QVariantList &vars)
{
	if(USER_LEVEL_TECH > System->getUserLevel())
	{
	
	}else
	{
		
	}
}

void QWorkFlowWidget::setCheckerState1(int iEvent,double data,double data2)
{
	//bool bBinDisp = System->getParam("sys_run_bin_display").toBool();	
	QString colorstr;
	if (STATION_STATE_WAIT_START == iEvent)
	{

	}
}

void QWorkFlowWidget::initStationState()
{
	IFlowCtrl * p = getModule<IFlowCtrl>(CTRL_MODEL);
	if(p)
	{		
	}
}

void QWorkFlowWidget::timerEvent(QTimerEvent *event)
{
	IFlowCtrl * p = getModule<IFlowCtrl>(CTRL_MODEL);
	if(p)
	{
		if(p->isRuning())
		{			
		}
		else
		{			
		}
	}	
}