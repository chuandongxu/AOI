#ifndef WORKFLOWDIAGIONS_H
#define WORKFLOWDIAGIONS_H

#include <QWidget>
#include "ui_WorkflowDiagions.h"

class QWorkflowDiagions : public QWidget
{
	Q_OBJECT

public:
	QWorkflowDiagions(QWidget *parent = 0);
	~QWorkflowDiagions();

protected slots:
	void onOutLine(int state);
	void onRunBtn();
	void onStopBtn();
	void onRestBtn();
	void onEmStopBtn();

	void onLeftStart();
	void onRightStart();
	void onPacketout();

private:
	Ui::QWorkflowDiagions ui;
};

#endif // WORKFLOWDIAGIONS_H
