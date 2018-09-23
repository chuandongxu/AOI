#include "userdialog.h"
#include <QCryptographicHash>
#include <qmessagebox>
#include <QSqlQuery>
#include "SystemData.h"

QUserDialog::QUserDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    //ui.lineEdit->setText("operator");
    //ui.lineEdit_2->setText("123");
    ui.lineEditUser->setText("admin");
    ui.lineEditPassword->setText("Lvr/CB");

    connect(ui.pushButton,SIGNAL(clicked()),this,SLOT(onOk()));
    connect(ui.pushButton_2,SIGNAL(clicked()),this,SLOT(onCancel()));
}

QUserDialog::~QUserDialog()
{

}

void QUserDialog::onOk()
{
    QString user = ui.lineEditUser->text();
    QString pwd = ui.lineEditPassword->text();

    if (System->checkUserPwd(user, pwd))
    {
        QDialog::accept();
        return;
    }

    QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("用户名或密码错误"));    
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
