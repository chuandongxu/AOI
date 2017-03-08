#include "hsgWorkflowDiagions.h"
#include "../Common/SystemData.h"
#include "../include/IdDefine.h"

QWorkflowDiagions::QWorkflowDiagions(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.checkBox,SIGNAL(stateChanged(int)),SLOT(onOutLine(int)));
	connect(ui.pushButton,SIGNAL(clicked()),SLOT(onRunBtn()));
	connect(ui.pushButton_2,SIGNAL(clicked()),SLOT(onStopBtn()));
	connect(ui.pushButton_3,SIGNAL(clicked()),SLOT(onRestBtn()));
	connect(ui.pushButton_4,SIGNAL(clicked()),SLOT(onEmStopBtn()));

	connect(ui.pushButton_5,SIGNAL(clicked()),SLOT(onLeftStart()));
	connect(ui.pushButton_6,SIGNAL(clicked()),SLOT(onRightStart()));
	connect(ui.pushButton_7,SIGNAL(clicked()),SLOT(onPacketout()));
}

QWorkflowDiagions::~QWorkflowDiagions()
{

}

void QWorkflowDiagions::onOutLine(int state)
{
	if(Qt::Checked == state)System->enableOutline(true);
	else System->enableOutline(false);
}

void QWorkflowDiagions::onRunBtn()
{
	int run = System->data(OUT_LINE_RUN).toInt();
	if(0 == run)System->setData(OUT_LINE_RUN,1);
	else System->setData(OUT_LINE_RUN,0);
}

void QWorkflowDiagions::onStopBtn()
{
	int stop = System->data(OUT_LINE_STOP).toInt();
	if(0 == stop)System->setData(OUT_LINE_STOP,1);
	else System->setData(OUT_LINE_STOP,0);
}

void QWorkflowDiagions::onRestBtn()
{
	
}

void QWorkflowDiagions::onEmStopBtn()
{
	
}

void QWorkflowDiagions::onLeftStart()
{
	
}

void QWorkflowDiagions::onRightStart()
{
	
}

void QWorkflowDiagions::onPacketout()
{
	
}