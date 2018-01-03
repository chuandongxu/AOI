#include "QRightWidget.h"

QRightWidget::QRightWidget(DataCtrl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	ui.verticalLayout_tab->addWidget(m_pCtrl->getWidget());
}

QRightWidget::~QRightWidget()
{
}

void QRightWidget::setWidget(QWidget* pRunWidget, QWidget* pSetWidget)
{
	m_pRunWidget = pRunWidget;
	m_pSetWidget = pSetWidget;

	ui.verticalLayout_Content->addWidget(m_pRunWidget);
	ui.verticalLayout_Content->addWidget(m_pSetWidget);
}
