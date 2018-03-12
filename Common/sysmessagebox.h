#ifndef SYSMESSAGEBOX_H
#define SYSMESSAGEBOX_H

#include <QDialog>
#include "ui_sysmessagebox.h"

class QSysMessageBox : public QDialog
{
	Q_OBJECT

public:
	QSysMessageBox(QWidget *parent = 0, bool bInteract = false);
	~QSysMessageBox();

	void setTitle(const QString &str);
	void setText(const QString &msg);
	void enableCloseBtn(bool flag);

protected slots:
	void onBtnClick();

private:
	Ui::QSysMessageBox ui;
};

#endif // SYSMESSAGEBOX_H
