#include "QStatisticsWidget.h"
#include "qcustomplot.h"
#include "../Common//SystemData.h"
#include <qmessagebox>
#include "../Common/eos.h"
#include "../include/IdDefine.h"
#include "../Common/CVSFile.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define ToFloat(param)      (static_cast<float>(param))

#define CHART_DATA_NUM 6

QStatisticsWidget::QStatisticsWidget(DataCtrl* pCtrl, QWidget *parent)
	:m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	QEos::Attach(EVENT_CHECK_STATE, this, SLOT(onStateEvent(const QVariantList &)));
	QEos::Attach(EVENT_RESULT_DISPLAY, this, SLOT(onResultEvent(const QVariantList &)));

		
	connect(ui.pushButton_runOnce, SIGNAL(clicked()), SLOT(onRunOnce()));

	ui.tableView_resultList->setModel(&m_model);
	ui.tableView_resultList->horizontalHeader()->setStretchLastSection(true);
	ui.tableView_resultList->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableView_resultList->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableView_resultList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(ui.tableView_resultList, SIGNAL(clicked(const QModelIndex &)), SLOT(mouseClickEvent(const QModelIndex &)));

	//m_pCustomPlot1 = new QCustomPlot(this);
	//setupSimpleDemo(m_pCustomPlot1);
	//ui.dockWidget->setWidget(m_pCustomPlot1);

	//m_pCustomPlot2 = new QCustomPlot(this);
	//setupSincScatterDemo(m_pCustomPlot2);
	//ui.dockWidget_2->setWidget(m_pCustomPlot2);

	for (int i = 0; i < CHART_DATA_NUM; i++)
	{
		m_customPlotValues.push_back(QMap<int, double>());
	}

	m_pCustomPlotObj = new QCustomPlot(this);
	setupLineChartDemo(m_pCustomPlotObj);
	ui.dockWidget_3->setWidget(m_pCustomPlotObj);

	m_pCustomPlotProf = new QCustomPlot(this);
	setupSincScatterDemo(m_pCustomPlotProf, QList<QVariant>(), QList<QVariant>(), QList<QVariant>(), QList<QVariant>(), QString(""));
	ui.dockWidget_4->setWidget(m_pCustomPlotProf);

	m_bAutoRunning = false;

	updateResultList();

	System->setSysParam("OK_COUNT_SYSTEM", 0);
}

QStatisticsWidget::~QStatisticsWidget()
{
}

void QStatisticsWidget::paintEvent(QPaintEvent* event)
{
	//QPainter painter(this);
	//// 设置画笔颜色
	//painter.setPen(QColor(0, 160, 230));

	//// 设置字体：微软雅黑、点大小50、斜体
	//QFont font;
	//font.setFamily("Microsoft YaHei");
	//font.setPointSize(20);
	//font.setItalic(true);
	//painter.setFont(font);

	//// 绘制文本
	//painter.drawText(QRect(10, 10, 35, 20), Qt::AlignCenter, "Qtttttttt");
	//painter.drawLine(0, 0, 100, 100);
}

void QStatisticsWidget::mouseClickEvent(const QModelIndex & index)
{
	if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
		QStringLiteral("是否导出数据到文件？"), QMessageBox::Ok, QMessageBox::Cancel))
	{
		QString path = QApplication::applicationDirPath();
		path += "/output/";

		QDir dir;
		if (!dir.exists(System->getRecordPath()))
		{
			QString filePath = QFileDialog::getExistingDirectory(this, QStringLiteral("打开文件夹"), path);
			if (!filePath.isEmpty())
			{
				System->setRecordPath(filePath);
			}
		}
		//QString picFilter = "File(*.csv)";
		//QString strFileName = QFileDialog::getSaveFileName(this, QStringLiteral("打开文件"), path/*/*/, picFilter);
		//QString filePath = QFileDialog::getExistingDirectory(this, QStringLiteral("打开文件夹"), path);


		//System->setRecordPath(filePath);
		System->enableRecord(true);

		if (qDataRecord().isNewFile())
		{
			qDataRecord() << QStringLiteral("Detect Item") << QStringLiteral("Detect Hight(mm)") << QDataRecord::endl;
		}

		for (int i = 0; i < m_customPlotHistoryValues.size(); i++)
		{
			qDataRecord() << QDataRecord::endl;
			qDataRecord() << m_customPlotHistoryNames[i] << QDataRecord::endl;

			for (int j = 0; j < m_customPlotHistoryValues[i].keys().size(); j++)
			{
				int nKey = m_customPlotHistoryValues[i].keys().at(j);
				qDataRecord() << nKey << m_customPlotHistoryValues[i].value(nKey) << QDataRecord::endl;
			}
		}

		QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("数据已经保存！"));
	}
}

void QStatisticsWidget::updateResultList()
{
	m_model.clear();

	QStringList ls;
	ls << QStringLiteral("元件") << QStringLiteral("位置") << QStringLiteral("数据") << QStringLiteral("结果");
	m_model.setHorizontalHeaderLabels(ls);

	ui.tableView_resultList->setColumnWidth(0, 70);
	ui.tableView_resultList->setColumnWidth(1, 70);
	ui.tableView_resultList->setColumnWidth(2, 120);
	ui.tableView_resultList->setColumnWidth(3, 70);
	
	//for (int j = 0; j < 6; j++)
	//{
	//	int nr = m_model.rowCount();
	//	m_model.insertRow(nr);

	//	m_model.setData(m_model.index(nr, 0), QStringLiteral("%1mm").arg(31.2));
	//	m_model.setData(m_model.index(nr, 1), QStringLiteral("%1mm").arg(31.2));
	//	m_model.setData(m_model.index(nr, 2), QStringLiteral("%1mm").arg(31.2));
	//	m_model.setData(m_model.index(nr, 3), QStringLiteral("%1mm").arg(15.0));
	//	m_model.setData(m_model.index(nr, 4), QStringLiteral("%1mm").arg(15.0));		
	//}
}

void QStatisticsWidget::onStateEvent(const QVariantList &data)
{
	int iBoard = data[0].toInt();
	int iEvent = data[1].toInt();
	//if (iEvent != STATION_STATE_RESOULT)return;
	int iValue = data[2].toInt();

	if (STATION_STATE_WAIT_START == iEvent)
	{
		//ui.label_resultStatus->setStyleSheet("background-color: rgb(50, 255, 50);");
		ui.label_resultStatus->setStyleSheet("background-color: rgb(0, 128, 192);");
		ui.label_resultStatus->setText(QStringLiteral("等待测试"));

		ui.progressBar_resultPrg->setValue(0);
	}
	else if (STATION_STATE_CAPTURING == iEvent)
	{
		ui.label_resultStatus->setStyleSheet("background-color: rgb(153, 217, 234);");
		ui.label_resultStatus->setText(QStringLiteral("抓取图像"));

		ui.progressBar_resultPrg->setValue(10);
	}
	else if (STATION_STATE_GENERATE_GRAY == iEvent)
	{
		ui.label_resultStatus->setStyleSheet("background-color: rgb(153, 217, 234);");
		ui.label_resultStatus->setText(QStringLiteral("生成灰度图"));

		ui.progressBar_resultPrg->setValue(20);
	}
	else if (STATION_STATE_CALCULATE_3D == iEvent)
	{
		ui.label_resultStatus->setStyleSheet("background-color: rgb(153, 217, 234);");
		ui.label_resultStatus->setText(QStringLiteral("计算3D"));

		ui.progressBar_resultPrg->setValue(40);
	}
	else if (STATION_STATE_MATCH_POSITION == iEvent)
	{
		ui.label_resultStatus->setStyleSheet("background-color: rgb(153, 217, 234);");
		ui.label_resultStatus->setText(QStringLiteral("匹配元件框"));

		ui.progressBar_resultPrg->setValue(70);
	}
	else if (STATION_STATE_CALCULATE_HEIGHT == iEvent)
	{
		ui.label_resultStatus->setStyleSheet("background-color: rgb(153, 217, 234);");
		ui.label_resultStatus->setText(QStringLiteral("计算高度值"));

		ui.progressBar_resultPrg->setValue(80);
	}	
	else if (STATION_STATE_RESOULT == iEvent)
	{
		ui.progressBar_resultPrg->setValue(100);

		if (iValue > 0)
		{
			int n = System->getSysParam("OK_COUNT_SYSTEM").toInt();
			n++;
			System->setSysParam("OK_COUNT_SYSTEM", n);		

			ui.label_resultOutput->setStyleSheet("color: rgb(50, 255, 50);font: 75 48pt '宋体';");
			ui.label_resultOutput->setText(QStringLiteral("OK"));
			//ui.label_resultOutput->setText(QStringLiteral("OK(") + QString::number(n) + QStringLiteral(")")/*QStringLiteral("OK")*/);
		}
		else 
		{		
			ui.label_resultOutput->setStyleSheet("color: rgb(255, 50, 50);font: 75 48pt '宋体';");
			ui.label_resultOutput->setText(QStringLiteral("FAIL"));
		}		
	}
}

void QStatisticsWidget::onResultEvent(const QVariantList &data)
{
	int iBoard = data[0].toInt();
	int iEvent = data[1].toInt();
	//if (iEvent != STATION_RESULT_DISPLAY)return;
	if (STATION_RESULT_DISPLAY_CLEAR == iEvent)
	{
		//updateResultList();
		clearChartData();
	}
	else if (STATION_RESULT_DISPLAY_CLEAR_DATA == iEvent)
	{
		updateResultList();
		//clearChartData(m_pCustomPlot3);
	}
	else if (STATION_RESULT_DISPLAY == iEvent)
	{
		QString szObjName = data[2].toString();
		int nObjIndex = data[3].toInt();
		double dHeightValue = data[4].toDouble();

		int nr = m_model.rowCount();
		m_model.insertRow(nr);

		m_model.setData(m_model.index(nr, 0), szObjName);
		m_model.setData(m_model.index(nr, 1), QStringLiteral("%1").arg(nObjIndex + 1));
		m_model.setData(m_model.index(nr, 2), QStringLiteral("%1mm").arg(dHeightValue));
		m_model.setData(m_model.index(nr, 3), QStringLiteral("OK"));
		m_model.item(nr, 3)->setForeground(QBrush(QColor(0, 128, 0)));
		//m_model.item(nr, 3)->setBackground(QBrush(QColor(0, 128, 0)));

		int nRowCount = 6;
		if (m_model.rowCount() > nRowCount)
		{
			m_model.removeRows(0, m_model.rowCount() - nRowCount);
		}

		insertChartData(nObjIndex, dHeightValue);
		insertHistoryData(nObjIndex, dHeightValue, szObjName);
	}
	else if (STATION_RESULT_DISPLAY_PROF == iEvent)
	{
		QString szObjName = data[2].toString();
		int nObjIndex = data[3].toInt();
		QString szValueStr = data[4].toString();
		QList<QVariant> varList = data[5].toList();
		QList<QVariant> varRangeList = data[6].toList();
		QList<QVariant> varRangeTypeList = data[7].toList();
		QList<QVariant> varRangeValueList = data[8].toList();

		updateProfChart(nObjIndex, szValueStr, varList, varRangeList, varRangeTypeList, varRangeValueList);
	}
}

void QStatisticsWidget::onRunOnce()
{
	bool isAutoRun = !m_pCtrl->isRunOnce();

	if (isAutoRun)
	{
		if (m_bAutoRunning)
		{
			ui.pushButton_runOnce->setText(QStringLiteral("一键检测"));
			m_pCtrl->setInfiniteCycles(false);
			m_bAutoRunning = false;
		}
		else
		{
			ui.pushButton_runOnce->setText(QStringLiteral("停止检测"));
			m_pCtrl->setInfiniteCycles(true);
			m_bAutoRunning = true;
		}
	}
	else
	{
		if (m_pCtrl)
		{
			m_pCtrl->incrementCycleTests();
		}
	}
}



void QStatisticsWidget::insertChartData(int nObjIndex, double dValue)
{
	//qDebug() << nObjIndex << " "<< m_customPlotValues[nObjIndex].size();
	if (nObjIndex < m_customPlotValues.size())
	{
		m_customPlotValues[nObjIndex].insert(m_customPlotValues[nObjIndex].size(), dValue);

		if (m_pCustomPlotObj)
		{
			delete m_pCustomPlotObj;
			m_pCustomPlotObj = new QCustomPlot(this);
			setupLineChartDemo(m_pCustomPlotObj);
			ui.dockWidget_3->setWidget(m_pCustomPlotObj);
		}
	}
}

void QStatisticsWidget::updateProfChart(int nObjIndex, QString szValue, QList<QVariant>& varList, QList<QVariant>& varRangeList, QList<QVariant>& varRangeTypeList, QList<QVariant>& varRangeValueList)
{
	if (m_pCustomPlotProf)
	{
		delete m_pCustomPlotProf;
		m_pCustomPlotProf = new QCustomPlot(this);
		setupSincScatterDemo(m_pCustomPlotProf, varList, varRangeList, varRangeTypeList, varRangeValueList, szValue);
		ui.dockWidget_4->setWidget(m_pCustomPlotProf);		
	}
}

void QStatisticsWidget::clearChartData()
{
	//customPlot->clearGraphs();
	//customPlot->clearItems();

	for (int i = 0; i < CHART_DATA_NUM; i++)
	{
		m_customPlotValues[i].clear();
	}

	if (m_pCustomPlotObj)
	{
		delete m_pCustomPlotObj;
		m_pCustomPlotObj = new QCustomPlot(this);
		setupLineChartDemo(m_pCustomPlotObj);
		ui.dockWidget_3->setWidget(m_pCustomPlotObj);
	}	

	if (m_pCustomPlotProf)
	{
		delete m_pCustomPlotProf;
		m_pCustomPlotProf = new QCustomPlot(this);
		setupSincScatterDemo(m_pCustomPlotProf, QList<QVariant>(), QList<QVariant>(), QList<QVariant>(), QList<QVariant>(), QString(""));
		ui.dockWidget_4->setWidget(m_pCustomPlotProf);
	}
}

void QStatisticsWidget::insertHistoryData(int nObjIndex, double dValue, QString& name)
{
	if (nObjIndex < 0) return;

	if (nObjIndex >= m_customPlotHistoryValues.size())
	{
		m_customPlotHistoryValues.push_back(QMap<int, double>());
		m_customPlotHistoryNames.push_back(name);
	}

	m_customPlotHistoryValues[nObjIndex].insert(m_customPlotHistoryValues[nObjIndex].size(), dValue);
}

void QStatisticsWidget::setupSimpleDemo(QCustomPlot *customPlot)
{
	//demoName = "Simple Demo";

	// add two new graphs and set their look:
	customPlot->addGraph();
	customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
	customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
	customPlot->addGraph();
	customPlot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
	// generate some points of data (y0 for first, y1 for second graph):
	QVector<double> x(251), y0(251), y1(251);
	for (int i = 0; i < 251; ++i)
	{
		x[i] = i;
		y0[i] = qExp(-i / 150.0)*qCos(i / 10.0); // exponentially decaying cosine
		y1[i] = qExp(-i / 150.0);              // exponential envelope
	}
	// configure right and top axis to show ticks but no labels:
	// (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
	customPlot->xAxis2->setVisible(true);
	customPlot->xAxis2->setTickLabels(false);
	customPlot->yAxis2->setVisible(true);
	customPlot->yAxis2->setTickLabels(false);
	// make left and bottom axes always transfer their ranges to right and top axes:
	connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
	connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
	// pass data points to graphs:
	customPlot->graph(0)->setData(x, y0);
	customPlot->graph(1)->setData(x, y1);
	// let the ranges scale themselves so graph 0 fits perfectly in the visible area:
	customPlot->graph(0)->rescaleAxes();
	// same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
	customPlot->graph(1)->rescaleAxes(true);
	// Note: we could have also just called customPlot->rescaleAxes(); instead
	// Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
	customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void QStatisticsWidget::setupSincScatterDemo(QCustomPlot *customPlot, QList<QVariant>& varList, QList<QVariant>& varRangeList, QList<QVariant>& varRangeTypeList, QList<QVariant>& varRangeValueList, QString& szValue)
{
	customPlot->legend->setVisible(true);
	customPlot->legend->setFont(QFont("Helvetica", 8));
	customPlot->legend->setIconSize(0, 0);
	customPlot->legend->setTextColor(QColor(128, 0, 64));
	// set locale to english, so we get english decimal separator:
	customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));

	// add theory curve graph:
	customPlot->addGraph();
	QPen pen;
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(0);
	pen.setColor(Qt::darkBlue);
	customPlot->graph(0)->setPen(pen);
	customPlot->graph(0)->setName("Profile");

	// generate data:
	int nDataCount = varList.size();
	QVector<double> x(nDataCount), y(nDataCount);
	for (int j = 0; j < nDataCount; ++j)
	{
		QPointF ptValue = varList[j].toPointF();
		x[j] = ptValue.x();
		y[j] = ptValue.y();
	}

	customPlot->graph(0)->setData(x, y);
	customPlot->graph(0)->rescaleAxes();
	customPlot->graph(0)->rescaleAxes(true);

	for (int i = 0; i < varRangeList.size(); i++)
	{
		// add theory curve graph:
		customPlot->addGraph();
		//pen.setColor(QColor(qSin(i * 1 + 1.2) * 80 + 80, qSin(i*0.3 + 0) * 80 + 80, qSin(i*0.3 + 1.5) * 80 + 80));
		pen.setColor(Qt::red);
		customPlot->graph()->setPen(pen);
		//customPlot->graph()->setName("Profile");
		customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 2.5));

		switch (varRangeTypeList[i].toInt())
		{
		case  0:
			customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 2.5));
			break;
		case  1:
			customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
			break;
		case  2:
			customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 2.5));
			break;
		default:
			break;
		}

		// generate data:
		cv::Point ptLine;
		ptLine.x = varRangeList[i].toPoint().x();
		ptLine.y = varRangeList[i].toPoint().y();

		int nDataCount = ptLine.y - ptLine.x;
		if (nDataCount == 0)
		{
			nDataCount = 1;			
		}
	
		QVector<double> x(nDataCount), y(nDataCount);
		for (int j = 0; j < nDataCount; ++j)
		{
			int nIndex = ptLine.x + j;
			if (nIndex >= varList.size())
			{
				if (varList.size() > 0)
				{
					QPointF ptValue = varList[varList.size() - 1].toPointF();
					x[j] = ptValue.x();
					y[j] = ptValue.y();
				}
				else
				{
					x[j] = 0;
					y[j] = 0;
				}			
				continue;
			}

			QPointF ptValue = varList[nIndex].toPointF();
			x[j] = ptValue.x();
			y[j] = ptValue.y();
		}

		customPlot->graph()->setData(x, y);
		customPlot->graph()->rescaleAxes(true);
	}	

	for (int i = 0; i < varRangeTypeList.size(); i++)
	{
		if (2 == varRangeTypeList[i].toInt())
		{
			// add theory curve graph:
			customPlot->addGraph();
			//pen.setColor(QColor(qSin(i * 1 + 1.2) * 80 + 80, qSin(i*0.3 + 0) * 80 + 80, qSin(i*0.3 + 1.5) * 80 + 80));
			pen.setColor(Qt::darkGreen);
			customPlot->graph()->setPen(pen);
			//customPlot->graph()->setName("Profile");
			customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssPlusCircle, 5));

			// generate data:
			cv::Point ptValue;
			ptValue.x = varRangeValueList[i].toPoint().x();
			ptValue.y = varRangeValueList[i].toPoint().y();			

			int nDataCount = 1;
			QVector<double> x(nDataCount), y(nDataCount);
			for (int j = 0; j < nDataCount; ++j)
			{
				x[j] = ptValue.x;
				y[j] = ptValue.y;
			}

			customPlot->graph()->setData(x, y);
			customPlot->graph()->rescaleAxes(true);
		}
	}

	if (varRangeList.size() <= 0)
	{
		customPlot->legend->setVisible(false);
	}
	else
	{
		for (int i = customPlot->legend->itemCount() - 1; i > 0; i--)
		{
			customPlot->legend->removeItem(i);
		}

		customPlot->graph(0)->setName(szValue);
	}

	// zoom out a bit:
	customPlot->yAxis->scaleRange(1.5, customPlot->yAxis->range().center());
	customPlot->xAxis->scaleRange(1.2, customPlot->xAxis->range().center());
	// setup look of bottom tick labels:
	//customPlot->xAxis->setTickLabelRotation(30);
	//customPlot->xAxis->setAutoTickCount(9);
	//customPlot->xAxis->setNumberFormat("ebc");
	customPlot->xAxis->setNumberPrecision(1);
	//customPlot->xAxis->moveRange(-10);

	// make top right axes clones of bottom left axes. Looks prettier:
	customPlot->axisRect()->setupFullAxesBox();
}

void QStatisticsWidget::setupLineChartDemo(QCustomPlot *customPlot)
{
	//demoName = "Line Style Demo";
	customPlot->legend->setVisible(false);
	customPlot->legend->setFont(QFont("Helvetica", 9));
	QPen pen;
	QStringList lineNames;
	lineNames << "Obj1" << "Obj2" << "Obj3" << "Obj4" << "Obj5" << "Obj6";
	// add graphs with different line styles:
	for (int i = 0; i < CHART_DATA_NUM; ++i)
	{
		customPlot->addGraph();
		pen.setColor(QColor(qSin(i * 1 + 1.2) * 80 + 80, qSin(i*0.3 + 0) * 80 + 80, qSin(i*0.3 + 1.5) * 80 + 80));
		customPlot->graph()->setPen(pen);
		customPlot->graph()->setName(lineNames.at(i));
		customPlot->graph()->setLineStyle(QCPGraph::LineStyle::lsLine);
		customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
		// generate data:
		int nDataCount = m_customPlotValues[i].size();
		int nDataIndex = nDataCount > 10 ? nDataCount - 10: 0;
		QVector<double> x(nDataCount), y(nDataCount);
		int nIndex = 0;
		for (int j = nDataIndex; j < nDataCount; ++j)
		{
			x[j] = nIndex++;
			y[j] = m_customPlotValues[i].value(m_customPlotValues[i].keys().at(j));
		}
		customPlot->graph()->setData(x, y);
		
		customPlot->graph()->rescaleAxes(true);
	}
	// zoom out a bit:
	customPlot->yAxis->scaleRange(1.5, customPlot->yAxis->range().center());
	customPlot->xAxis->scaleRange(1.1, customPlot->xAxis->range().center());
	// set blank axis lines:
	customPlot->xAxis->setTicks(false);
	customPlot->yAxis->setTicks(true);
	customPlot->xAxis->setTickLabels(false);
	customPlot->yAxis->setTickLabels(true);
	// make top right axes clones of bottom left axes:
	customPlot->axisRect()->setupFullAxesBox();
}

void QStatisticsWidget::setupBarChartDemo(QCustomPlot *customPlot)
{
	//demoName = "Bar Chart Demo";
	// set dark background gradient:
	QLinearGradient gradient(0, 0, 0, 400);
	gradient.setColorAt(0, QColor(90, 90, 90));
	gradient.setColorAt(0.38, QColor(105, 105, 105));
	gradient.setColorAt(1, QColor(70, 70, 70));
	customPlot->setBackground(QBrush(gradient));

	// create empty bar chart objects:
	QCPBars *regen = new QCPBars(customPlot->xAxis, customPlot->yAxis);
	QCPBars *nuclear = new QCPBars(customPlot->xAxis, customPlot->yAxis);
	QCPBars *fossil = new QCPBars(customPlot->xAxis, customPlot->yAxis);
	regen->setAntialiased(false); // gives more crisp, pixel aligned bar borders
	nuclear->setAntialiased(false);
	fossil->setAntialiased(false);
	regen->setStackingGap(1);
	nuclear->setStackingGap(1);
	fossil->setStackingGap(1);
	// set names and colors:
	fossil->setName("Fossil fuels");
	fossil->setPen(QPen(QColor(111, 9, 176).lighter(170)));
	fossil->setBrush(QColor(111, 9, 176));
	nuclear->setName("Nuclear");
	nuclear->setPen(QPen(QColor(250, 170, 20).lighter(150)));
	nuclear->setBrush(QColor(250, 170, 20));
	regen->setName("Regenerative");
	regen->setPen(QPen(QColor(0, 168, 140).lighter(130)));
	regen->setBrush(QColor(0, 168, 140));
	// stack bars on top of each other:
	nuclear->moveAbove(fossil);
	regen->moveAbove(nuclear);

	// prepare x axis with country labels:
	QVector<double> ticks;
	QVector<QString> labels;
	ticks << 1 << 2 << 3 << 4 << 5 << 6 << 7;
	labels << "USA" << "Japan" << "Germany" << "France" << "UK" << "Italy" << "Canada";
	QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
	textTicker->addTicks(ticks, labels);
	customPlot->xAxis->setTicker(textTicker);
	customPlot->xAxis->setTickLabelRotation(60);
	customPlot->xAxis->setSubTicks(false);
	customPlot->xAxis->setTickLength(0, 4);
	customPlot->xAxis->setRange(0, 8);
	customPlot->xAxis->setBasePen(QPen(Qt::white));
	customPlot->xAxis->setTickPen(QPen(Qt::white));
	customPlot->xAxis->grid()->setVisible(true);
	customPlot->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
	customPlot->xAxis->setTickLabelColor(Qt::white);
	customPlot->xAxis->setLabelColor(Qt::white);

	// prepare y axis:
	customPlot->yAxis->setRange(0, 12.1);
	customPlot->yAxis->setPadding(5); // a bit more space to the left border
	customPlot->yAxis->setLabel("Power Consumption in\nKilowatts per Capita (2007)");
	customPlot->yAxis->setBasePen(QPen(Qt::white));
	customPlot->yAxis->setTickPen(QPen(Qt::white));
	customPlot->yAxis->setSubTickPen(QPen(Qt::white));
	customPlot->yAxis->grid()->setSubGridVisible(true);
	customPlot->yAxis->setTickLabelColor(Qt::white);
	customPlot->yAxis->setLabelColor(Qt::white);
	customPlot->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
	customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));

	// Add data:
	QVector<double> fossilData, nuclearData, regenData;
	fossilData << 0.86*10.5 << 0.83*5.5 << 0.84*5.5 << 0.52*5.8 << 0.89*5.2 << 0.90*4.2 << 0.67*11.2;
	nuclearData << 0.08*10.5 << 0.12*5.5 << 0.12*5.5 << 0.40*5.8 << 0.09*5.2 << 0.00*4.2 << 0.07*11.2;
	regenData << 0.06*10.5 << 0.05*5.5 << 0.04*5.5 << 0.06*5.8 << 0.02*5.2 << 0.07*4.2 << 0.25*11.2;
	fossil->setData(ticks, fossilData);
	nuclear->setData(ticks, nuclearData);
	regen->setData(ticks, regenData);

	// setup legend:
	customPlot->legend->setVisible(true);
	customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignHCenter);
	customPlot->legend->setBrush(QColor(255, 255, 255, 100));
	customPlot->legend->setBorderPen(Qt::NoPen);
	QFont legendFont = font();
	legendFont.setPointSize(10);
	customPlot->legend->setFont(legendFont);
	customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

