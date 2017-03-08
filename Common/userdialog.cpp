#include "userdialog.h"
#include <QCryptographicHash>
#include <qmessagebox>
#include <QSqlQuery>
#include "SystemData.h"

const QString softOpt = "SupperMan";
const QString softPwd = "supperPwd.com";

QUserDialog::QUserDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.pushButton,SIGNAL(clicked()),this,SLOT(onOk()));
	connect(ui.pushButton_2,SIGNAL(clicked()),this,SLOT(onCancel()));
}

QUserDialog::~QUserDialog()
{

}

void QUserDialog::onOk()
{
	QString user = ui.lineEdit->text();
	QString pwd = ui.lineEdit_2->text();

	//if(user == softOpt && pwd == softPwd)
	//{
	//	System->setUser(QStringLiteral("调试员"),10);
	//	QDialog::accept();
	//	return;
	//}

	QString targPwd;
	int level;
	if(System->getUserPwd(user,targPwd,level))
	//if(getUserPwd(user,targPwd,level))
	{
		QByteArray pwdArr;
		pwdArr.append(pwd);
		QString pwdHash = QCryptographicHash::hash(pwdArr,QCryptographicHash::Md5).toBase64().data();
		if(pwdHash == targPwd)
		{
			System->setUser(user,level);
			QDialog::accept();
			return;
		}
	}
	
	QMessageBox::warning(this,QStringLiteral("提示"),QStringLiteral("用户名或密码错误，请核对后重新输入"));
}

void QUserDialog::onCancel()
{
	QDialog::reject();
}

bool QUserDialog::getUserPwd(const QString user,QString &pwd,int &level)
{
	QString queryStr = QString("SELECT password,level FROM T_userManager where username = '%0'").arg(user);
	QSqlQuery query(queryStr);
    while (query.next()) 
	{
		pwd = query.value(0).toString();
		level = query.value(1).toInt();

		return true;
	}

	return false;
}
