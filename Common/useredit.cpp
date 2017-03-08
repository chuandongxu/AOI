#include "useredit.h"
#include <qcryptographichash.h>
#include <QSqlQuery>
#include <qmessagebox>
#include "SystemData.h"



QUserEdit::QUserEdit(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QStringList ls;
	ls << USER_OPT << USER_MANAGER << USER_TECH << USER_DEBUGER;
	ui.comboBox->addItems(ls);

	connect(ui.pushButton_2,SIGNAL(clicked()),SLOT(onOk()));
	connect(ui.pushButton,SIGNAL(clicked()),SLOT(onCancel()));
}

QUserEdit::~QUserEdit()
{

}

void QUserEdit::initUIData(bool isEdit)
{
	if(isEdit)
	{
		ui.lineEdit->setText(m_userName);
		ui.lineEdit_2->setText(m_password);
		ui.lineEdit_3->setText(m_password);
		if(USER_LEVEL_OPT == m_level)ui.comboBox->setCurrentText(USER_OPT);
		else if(USER_LEVEL_MANAGER == m_level)ui.comboBox->setCurrentText(USER_OPT);
		else if(USER_LEVEL_TECH  == m_level)ui.comboBox->setCurrentText(USER_OPT);
		else if(USER_LEVEL_DEBUG  == m_level)ui.comboBox->setCurrentText(USER_DEBUGER);
		else ui.comboBox->setCurrentText(USER_OPT);

		ui.lineEdit->setReadOnly(true);
	}
	else 
	{
		ui.lineEdit->setReadOnly(false);
	}
}

void QUserEdit::onOk()
{
	QString pwd = ui.lineEdit_2->text();
	QString rePwd = ui.lineEdit_3->text();

	if(pwd == rePwd)
	{
		m_userName = ui.lineEdit->text();
		m_remark = ui.textEdit->toPlainText();

		if(isExistUser(m_userName))
		{
			QMessageBox::warning(NULL,QStringLiteral("错误"),QStringLiteral("用户名已存在，请重新输入用户名"));
			return;
		}

		QByteArray arr;
		arr.append(pwd);
		m_password = QCryptographicHash::hash(arr,QCryptographicHash::Md5).toBase64().data();

		QString str = ui.comboBox->currentText();
		
		if(str == USER_MANAGER) m_level = USER_LEVEL_MANAGER;
		else if(str == USER_TECH) m_level = USER_LEVEL_TECH;
		else if(str == USER_DEBUGER) m_level = USER_LEVEL_DEBUG;
		else m_level = USER_LEVEL_OPT;


		QDialog::accept();
	}
	else
	{
		QMessageBox::warning(NULL,QStringLiteral("错误"),QStringLiteral("两次输入密码不相同，请重新输入。"));
	}
}

void QUserEdit::onCancel()
{
	QDialog::reject();
}

bool QUserEdit::isExistUser(const QString &user)
{
	QSqlQuery query;
	QString strQuery = QString("select * id from T_userManager where userName = '%0").arg(user);
	if(query.exec(strQuery))
	{
		if(query.next()) 
		{
			return true;
		}
	}

	return false;
}