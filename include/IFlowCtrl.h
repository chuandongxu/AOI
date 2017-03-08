#ifndef _I_FLOW_CTRL_H_
#define _I_FLOW_CTRL_H_

#include <qwidget.h>
#include <QList>
#include <QStandardItemModel>

class IFlowCtrl
{
public:
	virtual bool isRuning() = 0;

	virtual QWidget * getFlowCtrlWidget() = 0;
	virtual void InitResoultItemModel(QStandardItemModel * pModel) = 0;
	virtual QList<int> getResoultItemHeaderWidth() = 0;
};


#endif