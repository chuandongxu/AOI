#include "LoginEdit.h"
#include "../common/SystemData.h"
#include <QCryptographicHash>
#include <qmessagebox.h>

QLoginEdit::QLoginEdit(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	//ui.lineEdit->setText("operator");
	//ui.lineEdit_2->setText("123");
	ui.lineEdit->setText("aoiDebuger");
	ui.lineEdit_2->setText("aoi.com");

	connect(ui.pushButton,SIGNAL(clicked()),SLOT(onOkBtn()));
	connect(ui.pushButton_2,SIGNAL(clicked()),SLOT(onExitBtn()));
}

QLoginEdit::~QLoginEdit()
{

}

void QLoginEdit::onOkBtn()
{
	QString user = ui.lineEdit->text();
	QString pwd = ui.lineEdit_2->text();

	QString targPwd;
	int level;
	if(System->getUserPwd(user,targPwd,level))
	{
		QByteArray pwdArr;
		pwdArr.append(pwd);
		QString pwdHash = QCryptographicHash::hash(pwdArr,QCryptographicHash::Md5).toBase64().data();
		if(pwdHash == targPwd)
		{
			System->setUser(user,level);
			emit ok();
			return;
		}
	}
	
	QMessageBox::warning(this,QStringLiteral("错误"),QStringLiteral("用户名或密码错误"));
}

void QLoginEdit::onExitBtn()
{
	emit exit();
}