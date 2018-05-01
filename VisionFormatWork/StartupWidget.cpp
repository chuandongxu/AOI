#include "StartupWidget.h"

QStartupWidget::QStartupWidget(QWidget *parent)
	: QDialog(parent)
{
	this->setObjectName("app_startup_widget");
	this->setFixedSize(841,614);

	m_layout = new QVBoxLayout;
	m_bkWidget = new QWidget(this);
	m_loginEdit = new QLoginEdit;

	m_bkWidget->setObjectName("loginEdit");
	m_bkWidget->setFixedSize(250,190);
	m_layout->addWidget(m_loginEdit);

	m_bkWidget->setLayout(m_layout);
	m_bkWidget->move(550,300);

	connect(m_loginEdit,SIGNAL(ok()),SLOT(onOk()));
	connect(m_loginEdit,SIGNAL(exit()),SLOT(onExit()));
}

QStartupWidget::~QStartupWidget()
{
}

void QStartupWidget::onOk()
{
	QDialog::accept();
}

void QStartupWidget::onExit()
{
	QDialog::reject();
}