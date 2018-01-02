#pragma once

#include <QWidget>
#include "ui_QRightWidget.h"
#include "DataCtrl.h"

class QRightWidget : public QWidget
{
	Q_OBJECT

public:
	QRightWidget(DataCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~QRightWidget();

public:
	void setWidget(QWidget* pRunWidget, QWidget* pSetWidget);

private:
	Ui::QRightWidget ui;
	DataCtrl* m_pCtrl;
	QWidget* m_pRunWidget;
	QWidget* m_pSetWidget;
};
