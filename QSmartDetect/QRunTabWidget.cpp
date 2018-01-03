#include "QRunTabWidget.h"
#include "../Common/eos.h"
#include "../include/IdDefine.h"

QRunTabWidget::QRunTabWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pushButton_setTab, SIGNAL(clicked()), SLOT(onSet()));
	connect(ui.pushButton_runTab, SIGNAL(clicked()), SLOT(onRun()));
	ui.pushButton_runTab->setEnabled(true);
	ui.pushButton_setTab->setEnabled(false);
}

QRunTabWidget::~QRunTabWidget()
{
}

void QRunTabWidget::onSet()
{
	QEos::Notify(EVENT_TAB_STATE, 0, RUN_TAB_SETTING, 0);	
}

void QRunTabWidget::onRun()
{
	QEos::Notify(EVENT_RESULT_DISPLAY, 0, STATION_RESULT_DISPLAY_CLEAR);
	QEos::Notify(EVENT_TAB_STATE, 0, RUN_TAB_RUNING, 0);	
}

void QRunTabWidget::showBtn(bool bRunning)
{
	ui.pushButton_runTab->setEnabled(bRunning);
	ui.pushButton_setTab->setEnabled(!bRunning);
}
