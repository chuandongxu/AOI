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

	ui.lineEdit->setText("operator");
	ui.lineEdit_2->setText("123");

	connect(ui.pushButton,SIGNAL(clicked()),SLOT(onOkBtn()));
	connect(ui.pushButton_2,SIGNAL(clicked()),SLOT(onExitBtn()));
}

QLoginEdit::~QLoginEdit()
{

}

void QLoginEdit::onOkBtn()
{
	if (!System->checkRuntimeAuthRight())
	{
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("系统没有授权！"));
		return;
	}

	QString user = ui.lineEdit->text();
	QString pwd = ui.lineEdit_2->text();

	QString targPwd;
	int level;
	if(System->getUserPwd(user,targPwd,level))
	{
		if ( (level > USER_LEVEL_OPT) && PWD_INVENTOR)
		{
			QDateTime dtm = QDateTime::currentDateTime();
			QString dateTM = dtm.toString("yyyyMM");
			QByteArray pwdArr;
			if (USER_LEVEL_MANAGER == level)
			{
				targPwd = "admin";
			}
			else if (USER_LEVEL_TECH == level)
			{
				targPwd = "root";
			}
			pwdArr.append(targPwd);
			pwdArr.append(dateTM);
			QString pwdHash = QCryptographicHash::hash(pwdArr, QCryptographicHash::Md5).toBase64().data();
			QString pwdCacl = pwdHash.left(6);
			if (pwd == pwdCacl)
			{
				System->setUser(user, level);
				emit ok();
				return;
			}
		}
		else
		{
			QByteArray pwdArr;
			pwdArr.append(pwd);
			QString pwdHash = QCryptographicHash::hash(pwdArr, QCryptographicHash::Md5).toBase64().data();
			if (pwdHash == targPwd)
			{
				System->setUser(user, level);
				emit ok();
				return;
			}
		}
	}
	
	QMessageBox::warning(this,QStringLiteral("错误"),QStringLiteral("用户名或密码错误"));
}

void QLoginEdit::onExitBtn()
{
	emit exit();
}