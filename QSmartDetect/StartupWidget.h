#ifndef STARTUPWIDGET_H
#define STARTUPWIDGET_H

#include <QObject>
#include <QDialog>
#include <qlabel.h>
#include <qlineedit.h>
#include <QVBoxLayout>
#include "LoginEdit.h"

class QStartupWidget : public QDialog
{
	Q_OBJECT

public:
	QStartupWidget(QWidget *parent = NULL);
	~QStartupWidget();

protected slots:
	void onOk();
	void onExit();

private:
	QLoginEdit * m_loginEdit;
	QWidget * m_bkWidget;
	QVBoxLayout * m_layout;
};

#endif // STARTUPWIDGET_H
