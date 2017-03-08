#ifndef LOGINEDIT_H
#define LOGINEDIT_H

#include <QWidget>
#include "ui_LoginEdit.h"

class QLoginEdit : public QWidget
{
	Q_OBJECT

public:
	QLoginEdit(QWidget *parent = 0);
	~QLoginEdit();

signals:
	void ok();
	void exit();

protected slots:
	void onOkBtn();
	void onExitBtn();

private:
	Ui::QLoginEdit ui;
};

#endif // LOGINEDIT_H
