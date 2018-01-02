#pragma once

#include <QWidget>
#include "ui_QStatisticsWidget.h"
#include "DataCtrl.h"
#include <QVector>
#include <QMap>

#include <qstandarditemmodel>

class QCPItemTracer;
class QCustomPlot;
class QStatisticsWidget : public QWidget
{
	Q_OBJECT

public:
	QStatisticsWidget(DataCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~QStatisticsWidget();	

protected:
	void paintEvent(QPaintEvent*);

private:
	void updateResultList();

protected slots:
	void mouseClickEvent(const QModelIndex &);
	void onStateEvent(const QVariantList &data);
	void onResultEvent(const QVariantList &data);

	void onRunOnce();

private:
	void setupSimpleDemo(QCustomPlot *customPlot);
	void setupSincScatterDemo(QCustomPlot *customPlot, QList<QVariant>& varList, QList<QVariant>& varRangeList, QList<QVariant>& varRangeTypeList, QList<QVariant>& varRangeValueList, QString& szValue);
	void setupBarChartDemo(QCustomPlot *customPlot);
	void setupLineChartDemo(QCustomPlot *customPlot);

	void insertChartData(int nObjIndex, double dValue);
	void clearChartData();

	void updateProfChart(int nObjIndex, QString szValue, QList<QVariant>& varList, QList<QVariant>& varRangeList, QList<QVariant>& varRangeTypeList, QList<QVariant>& varRangeValueList);

	void insertHistoryData(int nObjIndex, double dValue, QString& name = QString(""));

private:
	Ui::QStatisticsWidget ui;
	DataCtrl* m_pCtrl;
	QStandardItemModel m_model;

	QCPItemTracer *itemDemoPhaseTracer;
	//QCustomPlot* m_pCustomPlot1;
	QCustomPlot* m_pCustomPlotProf;
	QCustomPlot* m_pCustomPlotObj;

	QVector<QMap<int, double>> m_customPlotValues;
	QVector<QMap<int, double>> m_customPlotHistoryValues;
	QVector<QString> m_customPlotHistoryNames;

	bool m_bAutoRunning;
};
