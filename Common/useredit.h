#ifndef USEREDIT_H
#define USEREDIT_H

#include <QDialog>
#include "ui_useredit.h"

#define USER_OPT QStringLiteral("操作员")
#define USER_MANAGER QStringLiteral("设备管理员")
#define USER_TECH QStringLiteral("设备技术人员")
#define USER_DEBUGER QStringLiteral("设备调测人员")

class QUserEdit : public QDialog
{
	Q_OBJECT

public:
	QUserEdit(QWidget *parent = 0);
	~QUserEdit();

	void initUIData(bool isEdit);

protected slots:
	void onOk();
	void onCancel();

protected:
	bool isExistUser(const QString &user);

private:
	Ui::QUserEdit ui;

public:
	QString m_userName;
	QString m_password;
	int m_level;
	QString m_remark;
};

#endif // USEREDIT_H
