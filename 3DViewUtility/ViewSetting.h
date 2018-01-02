#pragma once

#include <QWidget>
#include "ui_QViewSetting.h"

class QViewSetting : public QWidget
{
	Q_OBJECT

public:
	QViewSetting(QWidget *parent = Q_NULLPTR);
	~QViewSetting();

protected slots:
	void onCSGIndexChanged(int index);
	void onSampFreqIndexChanged(int index);

protected:
	void closeEvent(QCloseEvent *e);

private:
	Ui::QViewSetting ui;
};
