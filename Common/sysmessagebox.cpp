#include "sysmessagebox.h"

QSysMessageBox::QSysMessageBox(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.pushButton,SIGNAL(clicked()),SLOT(onBtnClick()));
}

QSysMessageBox::~QSysMessageBox()
{

}

void QSysMessageBox::setTitle(const QString &str)
{
	ui.label->setText(str);
}

void QSysMessageBox::setText(const QString &msg)
{
	//ui.label->setText(msg);
	ui.plainTextEdit->setPlainText(msg);
}

void QSysMessageBox::enableCloseBtn(bool flag)
{
	if(flag)ui.pushButton->show();
	else ui.pushButton->hide();
}

void QSysMessageBox::onBtnClick()
{
	this->hide();
}