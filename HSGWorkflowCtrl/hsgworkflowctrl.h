#ifndef HSGWORKFLOWCTRL_H
#define HSGWORKFLOWCTRL_H

#include "hsgworkflowctrl_global.h"
#include "../Common/modulebase.h"
#include "../include/IFlowCtrl.h"
#include <qstandarditemmodel.h>
#include "hsgflowctrl.h"
#include "hsgWorkFlowWidget.h"

class HSGWorkflowCtrl : public QModuleBase , public IFlowCtrl
{
public:
	HSGWorkflowCtrl(int id,const QString &name);
	~HSGWorkflowCtrl();

	void addSettingWiddget(QTabWidget *tabWidget);
	void addDiagonseWidget(QTabWidget * tabWidget);
	
	virtual bool isRuning();

	virtual QWidget * getFlowCtrlWidget();
	virtual void InitResoultItemModel(QStandardItemModel * pModel);
	virtual QList<int> getResoultItemHeaderWidth();
private:
	void initErrorCode();

private:
	QFlowCtrl * m_ctrl;
	QWorkFlowWidget * m_workflowWidget;

};

#endif // HSGWORKFLOWCTRL_H
