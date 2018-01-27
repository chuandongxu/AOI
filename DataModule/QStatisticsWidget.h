#pragma once

#include <QWidget>
#include "ui_QStatisticsWidget.h"
#include "DataCtrl.h"

#include <qstandarditemmodel>


class QStatisticsWidget : public QWidget
{
	Q_OBJECT

public:
	QStatisticsWidget(DataCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~QStatisticsWidget();

private:
	void updateResultList();

protected slots:
	void onStateEvent(const QVariantList &data);
	void onResultEvent(const QVariantList &data);

	void onStateChangeAutoRun(int iState);
	void onRunOnce();

	void onStartAutoRun();
	void onStopAutoRun();

private:
	Ui::QStatisticsWidget ui;
	DataCtrl* m_pCtrl;
	QStandardItemModel m_model;

	bool m_bAutoRunning;
};
