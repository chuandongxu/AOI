#include "rightwidget.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../include/IData.h"
#include <QStyleOption>
#include <QPainter>
#include "../common/SystemData.h"
#include "../Common/eos.h"

QRightWidget::QRightWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	this->setFixedWidth(410);

	IData * pData = getModule<IData>(DATA_MODEL);
	if (pData)
	{
		m_widgetEdit = pData->getToolWidget(true);
		m_widgetRun = pData->getToolWidget(false);
		m_widgetRun->setVisible(false);

		m_subVLayout = new QHBoxLayout;
		m_subVLayout->setContentsMargins(0, 0, 0, 0);
		m_subVLayout->addWidget(m_widgetEdit);
		m_subVLayout->addWidget(m_widgetRun);
		//m_subVLayout->setAlignment(Qt::AlignLeft);

		m_mainLayout = new QVBoxLayout;
		m_mainLayout->setContentsMargins(0, 0, 0, 0);
		m_mainLayout->addLayout(m_subVLayout);
		this->setLayout(m_mainLayout);
	}	

	QEos::Attach(EVENT_RUN_STATE, this, SLOT(onRunState(const QVariantList &)));

	QPalette Pal(palette());
	Pal.setColor(QPalette::Background, QColor(235, 235, 235));
	setAutoFillBackground(true);
	setPalette(Pal);	
}

QRightWidget::~QRightWidget()
{
}

void QRightWidget::paintEvent(QPaintEvent *event)
{
	QStyleOption option;
	option.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);

	QWidget::paintEvent(event);
}

void QRightWidget::onRunState(const QVariantList &data)
{
	if (data.size() == 1)
	{
		int iState = data[0].toInt();
		if (RUN_STATE_RUNING == iState)
		{
			m_widgetRun->setVisible(true);
			m_widgetEdit->setVisible(false);
		}
		else
		{
			m_widgetRun->setVisible(false);
			m_widgetEdit->setVisible(true);
		}
	}
}

