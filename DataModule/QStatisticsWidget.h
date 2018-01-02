#pragma once

#include <QWidget>
#include "ui_QStatisticsWidget.h"
#include "DataCtrl.h"

#include <qstandarditemmodel>

class QCPItemTracer;
class QCustomPlot;
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

private:
	void setupSimpleDemo(QCustomPlot *customPlot);
	void setupSincScatterDemo(QCustomPlot *customPlot);
	void setupBarChartDemo(QCustomPlot *customPlot);

private:
	Ui::QStatisticsWidget ui;
	DataCtrl* m_pCtrl;
	QStandardItemModel m_model;

	QCPItemTracer *itemDemoPhaseTracer;
	//QCustomPlot* m_pCustomPlot1;
	//QCustomPlot* m_pCustomPlot2;
	QCustomPlot* m_pCustomPlot3;

	bool m_bAutoRunning;
};
