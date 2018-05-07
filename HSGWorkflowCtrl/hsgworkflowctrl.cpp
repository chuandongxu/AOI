#include "hsgworkflowctrl.h"
#include "hsgworksetting.h"
#include "hsgWorkflowDiagions.h"
#include "hsgWorkFlowWidget.h"
#include "../Common/SystemData.h"
#include "hsgworkflowctrl_global.h"
#include "../include/IdDefine.h"

HSGWorkflowCtrl::HSGWorkflowCtrl(int id,const QString &name)
	:QModuleBase(id,name)
{
	initErrorCode();

	m_ctrl = new QFlowCtrl(NULL);
	m_workflowWidget = new QWorkFlowWidget;
}

HSGWorkflowCtrl::~HSGWorkflowCtrl()
{
	if(m_ctrl)delete m_ctrl;
	//if(m_workflowWidget)delete m_workflowWidget;
}


void HSGWorkflowCtrl::addSettingWiddget(QTabWidget *tabWidget)
{
	tabWidget->addTab(new QWorkSetting(m_ctrl),QStringLiteral("工艺设置"));
}

void HSGWorkflowCtrl::addDiagonseWidget(QTabWidget * tabWidget)
{
	tabWidget->addTab(new QWorkflowDiagions,QStringLiteral("工艺调试"));
}

void HSGWorkflowCtrl::preStartUp()
{
	if (m_ctrl) m_ctrl->initStartUp();
}

bool HSGWorkflowCtrl::isRuning()
{
	return m_ctrl->isRuning();
}

QWidget * HSGWorkflowCtrl::getFlowCtrlWidget()
{
	return m_workflowWidget;
}

void HSGWorkflowCtrl::InitResoultItemModel(QStandardItemModel * pModel)
{
	QStringList ls;
	ls << QStringLiteral("序号") << QStringLiteral("名称") << QStringLiteral("条码") << QStringLiteral("数据1") << QStringLiteral("数据2") << QStringLiteral("数据3");

	pModel->setHorizontalHeaderLabels(ls);

	pModel->insertRows(0, 2);
	for (int i = 0; i < 2; i++)
	{
		pModel->setData(pModel->index(i, 0), QString(QStringLiteral("工位%0")).arg(i + 1));
		pModel->setData(pModel->index(i, 1), QStringLiteral("检测"));
	}
}

QList<int> HSGWorkflowCtrl::getResoultItemHeaderWidth()
{
	QList<int> ls;
	ls << 60 << 85 << 105 << 60 << 60 << 60 << -1;
	return ls;
}

void HSGWorkflowCtrl::initErrorCode()
{
	System->addErrorMap(ERROR_ATHU_NORIGHT_WARRING, MSG_ATH_NORIGHT_WARRING);
	System->addErrorMap(ERROR_SAFE_DOOR,MSG_SAFE_DOOR);
	System->addErrorMap(ERROR_MOTOR_ALM,MSG_MOTOR_ALM);
	System->addErrorMap(ERROR_ZHOME_ALM,MSG_ZHOME_ALM);
	System->addErrorMap(ERROR_XHOME_ALM,MSG_XHOME_ALM);
	System->addErrorMap(ERROR_SHOME_ALM,MSG_SHOME_ALM);
	System->addErrorMap(ERROR_HOME_MOTION_ALM,MSG_HOME_NOMOTION_ALM);
	System->addErrorMap(ERROR_MOTION_POS_WARRING,MSG_MOTION_POS_WARRING);
	System->addErrorMap(ERROR_STATION_SAFE_GRATING_ALRM,MSG_STTATION_SAFE_GRATING_ALRM);
}

QMOUDLE_INSTANCE(HSGWorkflowCtrl)