#ifndef INITCHECKER_H
#define INITCHECKER_H

#include <QDialog>
#include "ui_InitChecker.h"
#include "runnercontext.h"
#include <qlist.h>

class QInitChecker : public QDialog
{
	Q_OBJECT

public:
	QInitChecker(QList<QRunnerContext> *pContextList,QWidget *parent = 0);
	~QInitChecker();

protected slots:
	void onOK();
	void onCancel();
protected:
	void setInitMaterialState(int n,bool s);

private:
	Ui::QInitChecker ui;
	QList<QRunnerContext> * m_pContextList;
};

#endif // INITCHECKER_H
