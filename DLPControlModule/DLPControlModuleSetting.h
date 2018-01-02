#pragma once

#include <QWidget>
#include "ui_DLPControlModuleSetting.h"
#include "DLPControl.h"
#include <QVBoxLayout>
#include <QTabWidget>

#include <qstandarditemmodel>

class DLPControlModuleSetting : public QWidget
{
	Q_OBJECT

public:
	DLPControlModuleSetting(DLPControl* pCtrl, QWidget *parent = Q_NULLPTR);
	~DLPControlModuleSetting();

protected slots:
	void onDLPIndexChanged(int iState);
	void onInqureInfo();
	void onBindDLPSNB();

private:
	void refreshHWInfo();

private:
	Ui::DLPControlModuleSetting ui;
	DLPControl* m_pCtrl;
	QTabWidget* m_pTabWidget;

	QStandardItemModel m_model;
};
