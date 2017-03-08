#ifndef USERDIALOG_H
#define USERDIALOG_H

#include <QDialog>
#include "ui_userdialog.h"

class QUserDialog : public QDialog
{
	Q_OBJECT

public:
	QUserDialog(QWidget *parent = 0);
	~QUserDialog();

protected slots:
	void onOk();
	void onCancel();

protected:
	bool getUserPwd(const QString user,QString &pwd,int &level);

private:
	Ui::QUserDialog ui;
};

#endif // USERDIALOG_H
