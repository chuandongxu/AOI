#include "authright.h"
#include "SystemData.h"

QAuthRight::QAuthRight(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.pushButton,SIGNAL(clicked()),this,SLOT(onOk()));
	connect(ui.pushButton_2,SIGNAL(clicked()),this,SLOT(onCancel()));
}

QAuthRight::~QAuthRight()
{

}

void QAuthRight::onOk()
{
	QString authCode = ui.lineEdit->text();
	if(System->doAuthright(authCode))
	{
		QDialog::accept();
	}
}

void QAuthRight::onCancel()
{
	QDialog::reject();
}