#ifndef AUTHRIGHT_H
#define AUTHRIGHT_H

#include <QDialog>
#include "ui_authright.h"

class QAuthRight : public QDialog
{
	Q_OBJECT

public:
	QAuthRight(QWidget *parent = 0);
	~QAuthRight();

protected slots:
	void onOk();
	void onCancel();

private:
	Ui::QAuthRight ui;
};

#endif // AUTHRIGHT_H
