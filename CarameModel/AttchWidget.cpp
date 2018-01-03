#include "AttchWidget.h"

QAttchWidget::QAttchWidget(QWidget *w,QWidget *parent)
	: QWidget(parent),m_attWidget(w)
{
	m_pLayout = new QHBoxLayout;
	m_pLayout->addWidget(m_attWidget);
	this->setLayout(m_pLayout);
}

QAttchWidget::~QAttchWidget()
{
	m_attWidget->setParent(NULL);
	m_pLayout->removeWidget(m_attWidget);
}
