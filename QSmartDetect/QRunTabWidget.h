#pragma once

#include <QWidget>
#include "ui_QRunTabWidget.h"

class QRunTabWidget : public QWidget
{
	Q_OBJECT

public:
	QRunTabWidget(QWidget *parent = Q_NULLPTR);
	~QRunTabWidget();

protected slots:
	void onSet();
	void onRun();

public:
	void showBtn(bool bRunning);

private:
	Ui::QRunTabWidget ui;
};
