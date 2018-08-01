#include "LoginEdit.h"
#include "../common/SystemData.h"
#include <QCryptographicHash>
#include <qmessagebox.h>
#include <QDateTime>

#define PWD_INVENTOR 1

QLoginEdit::QLoginEdit(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	//ui.lineEdit->setText("operator");
	//ui.lineEdit_2->setText("123");
	ui.lineEditUser->setText("admin");
	ui.lineEditPassword->setText("jFgEMa");

	connect(ui.pushButton,SIGNAL(clicked()),SLOT(onOkBtn()));
	connect(ui.pushButton_2,SIGNAL(clicked()),SLOT(onExitBtn()));
}

QLoginEdit::~QLoginEdit()
{
}

void QLoginEdit::onOkBtn()
{
	QString user = ui.lineEditUser->text();
	QString pwd = ui.lineEditPassword->text();

	if (System->checkUserPwd(user, pwd))
	{
		emit ok();
		return;
	}

	QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("用户名或密码错误"));
}

void QLoginEdit::onExitBtn()
{
	emit exit();
}