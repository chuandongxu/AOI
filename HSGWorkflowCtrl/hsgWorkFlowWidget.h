#ifndef WORKFLOWWIDGET_H
#define WORKFLOWWIDGET_H

#include <QWidget>
#include "ui_WorkFlowWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

class QWorkFlowWidget : public QWidget
{
	Q_OBJECT

public:
	QWorkFlowWidget(QWidget *parent = 0);
	~QWorkFlowWidget();

protected slots:
	void onCheckerStateChange(const QVariantList &vars);
	void onBarCodeChange(const QVariantList &vars);
	void onChangeUser(const QVariantList &vars);
	
	
protected:
	void initStationState();

	void setCheckerState1(int iEvent,double data,double data2);	

	void offset(QWidget *w,int x,int y);
	
	void timerEvent(QTimerEvent *event);
private:
	Ui::QWorkFlowWidget ui;
	int m_nTimerID;
	QHBoxLayout * m_subVLayout;
	QVBoxLayout * m_mainLayout;
};

#endif // WORKFLOWWIDGET_H
