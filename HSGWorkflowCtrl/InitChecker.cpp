#include "InitChecker.h"
#include <qmessagebox.h>
#include "workflowctrl_global.h"

QInitChecker::QInitChecker(QList<QRunnerContext> *pContextList,QWidget *parent)
	: QDialog(parent),m_pContextList(pContextList)
{
	ui.setupUi(this);

	connect(ui.pushButton,SIGNAL(clicked()),SLOT(onOK()));
	connect(ui.pushButton_2,SIGNAL(clicked()),SLOT(onCancel()));
}

QInitChecker::~QInitChecker()
{

}

void QInitChecker::setInitMaterialState(int n,bool s)
{
	int iState = 0;
	if(s)iState = 1;

	(*m_pContextList)[n].setParam(CHECK_INIT_MATERIAL,iState);
}

void QInitChecker::onOK()
{
	if(m_pContextList->size() < 6)
	{
		QMessageBox::warning(NULL,"",QStringLiteral("环境参数个数错误"));
		return;
	}

	bool s = ui.checkBox->isChecked();
	setInitMaterialState(0,s);

	s = ui.checkBox_2->isChecked();
	setInitMaterialState(1,s);

	s = ui.checkBox_3->isChecked();
	setInitMaterialState(2,s);

	s = ui.checkBox_4->isChecked();
	setInitMaterialState(3,s);

	s = ui.checkBox_5->isChecked();
	setInitMaterialState(4,s);

	s = ui.checkBox_6->isChecked();
	setInitMaterialState(5,s);

	QDialog::accept();
}

void QInitChecker::onCancel()
{
	if(m_pContextList->size() < 6)
	{
		QMessageBox::warning(NULL,"",QStringLiteral("环境参数个数错误"));
		QDialog::reject();
		return;
	}

	setInitMaterialState(0,false);
	setInitMaterialState(1,false);
	setInitMaterialState(2,false);
	setInitMaterialState(3,false);
	setInitMaterialState(4,false);
	setInitMaterialState(5,false);

	QDialog::reject();
}