#include "CenterWidget.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../include/IFlowCtrl.h"

QCenterWidget::QCenterWidget(QWidget *parent)
    :QFrame(parent)
{
	IFlowCtrl * p = getModule<IFlowCtrl>(CTRL_MODEL);
	if(!p)return;

	m_widget = p->getFlowCtrlWidget();

	m_subVLayout = new QHBoxLayout;
	m_subVLayout->addWidget(m_widget);

	m_mainLayout = new QVBoxLayout;
	m_mainLayout->setContentsMargins(0,0,0,0);
	m_mainLayout->addLayout(m_subVLayout);
    this->setLayout(m_mainLayout);
}

void QCenterWidget::setCenterWidget(QWidget * w)
{
	
}