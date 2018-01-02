#include "QStatisticsWidget.h"
#include "qcustomplot.h"
#include "../Common//SystemData.h"
#include <qmessagebox>
#include "../Common/eos.h"
#include "../include/IdDefine.h"
#include "datamodule_global.h"


QStatisticsWidget::QStatisticsWidget(DataCtrl* pCtrl, QWidget *parent)
	:m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	QEos::Attach(EVENT_CHECK_STATE, this, SLOT(onStateEvent(const QVariantList &)));
	QEos::Attach(EVENT_RESULT_DISPLAY, this, SLOT(onResultEvent(const QVariantList &)));
		
	connect(ui.pushButton_runOnce, SIGNAL(clicked()), SLOT(onRunOnce()));
	connect(ui.checkBox_AutoRun, SIGNAL(stateChanged(int)), SLOT(onStateChangeAutoRun(int)));	

	ui.tableView_resultList->setModel(&m_model);

	//m_pCustomPlot1 = new QCustomPlot(this);
	//setupSimpleDemo(m_pCustomPlot1);
	//ui.dockWidget->setWidget(m_pCustomPlot1);

	//m_pCustomPlot2 = new QCustomPlot(this);
	//setupSincScatterDemo(m_pCustomPlot2);
	//ui.dockWidget_2->setWidget(m_pCustomPlot2);

	m_pCustomPlot3 = new QCustomPlot(this);
	setupBarChartDemo(m_pCustomPlot3);
	ui.dockWidget_3->setWidget(m_pCustomPlot3);

	m_bAutoRunning = false;

	updateResultList();
}

QStatisticsWidget::~QStatisticsWidget()
{
}

void QStatisticsWidget::updateResultList()
{
	m_model.clear();

	QStringList ls;
	ls << QStringLiteral("板子") << QStringLiteral("元件") << QStringLiteral("位置") << QStringLiteral("结果") << QStringLiteral("保留1");
	m_model.setHorizontalHeaderLabels(ls);

	ui.tableView_resultList->setColumnWidth(0, 55);
	ui.tableView_resultList->setColumnWidth(1, 60);
	ui.tableView_resultList->setColumnWidth(2, 60);
	ui.tableView_resultList->setColumnWidth(3, 100);
	ui.tableView_resultList->setColumnWidth(4, 60);
	
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
			ui.label_resultOutput->setStyleSheet("color: rgb(50, 255, 50);font: 75 48pt '宋体';");
			ui.label_resultOutput->setText(QStringLiteral("OK"));
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
	if (iEvent != STATION_RESULT_DISPLAY)return;
	int nObjIndex = data[2].toInt();
	int nDeteteIndex = data[3].toInt();
	double dHeightValue = data[4].toDouble();

	int nr = m_model.rowCount();
	m_model.insertRow(nr);

	m_model.setData(m_model.index(nr, 0), QStringLiteral("%1").arg(iBoard + 1));
	m_model.setData(m_model.index(nr, 1), QStringLiteral("%1").arg(nObjIndex + 1));
	m_model.setData(m_model.index(nr, 2), QStringLiteral("%1").arg(nDeteteIndex + 1));
	m_model.setData(m_model.index(nr, 3), QStringLiteral("%1mm").arg(dHeightValue));
	m_model.setData(m_model.index(nr, 4), QStringLiteral("--"));

	int nRowCount = 50;
	if (m_model.rowCount() > nRowCount)
	{
		m_model.removeRows(0, m_model.rowCount() - nRowCount);
	}	
}

void QStatisticsWidget::onStateChangeAutoRun(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	if ((bool)data)
	{
		ui.pushButton_runOnce->setText(QStringLiteral("自动测试"));			
	}
	else
	{
		ui.pushButton_runOnce->setText(QStringLiteral("单步测试"));		
	}	

	m_pCtrl->setInfiniteCycles(false);
	m_bAutoRunning = false;
}

void QStatisticsWidget::onRunOnce()
{
	bool isAutoRun = ui.checkBox_AutoRun->isChecked();

	if (isAutoRun)
	{
		if (m_bAutoRunning)
		{
			ui.pushButton_runOnce->setText(QStringLiteral("自动测试"));
			m_pCtrl->setInfiniteCycles(false);
			m_bAutoRunning = false;
		}
		else
		{
			ui.pushButton_runOnce->setText(QStringLiteral("停止测试"));
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

void QStatisticsWidget::setupSincScatterDemo(QCustomPlot *customPlot)
{
	//demoName = "Sinc Scatter Demo";
	customPlot->legend->setVisible(true);
	customPlot->legend->setFont(QFont("Helvetica", 9));
	// set locale to english, so we get english decimal separator:
	customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
	// add confidence band graphs:
	customPlot->addGraph();
	QPen pen;
	pen.setStyle(Qt::DotLine);
	pen.setWidth(1);
	pen.setColor(QColor(180, 180, 180));
	customPlot->graph(0)->setName("Confidence Band 68%");
	customPlot->graph(0)->setPen(pen);
	customPlot->graph(0)->setBrush(QBrush(QColor(255, 50, 30, 20)));
	customPlot->addGraph();
	customPlot->legend->removeItem(customPlot->legend->itemCount() - 1); // don't show two confidence band graphs in legend
	customPlot->graph(1)->setPen(pen);
	customPlot->graph(0)->setChannelFillGraph(customPlot->graph(1));
	// add theory curve graph:
	customPlot->addGraph();
	pen.setStyle(Qt::DashLine);
	pen.setWidth(2);
	pen.setColor(Qt::red);
	customPlot->graph(2)->setPen(pen);
	customPlot->graph(2)->setName("Theory Curve");
	// add data point graph:
	customPlot->addGraph();
	customPlot->graph(3)->setPen(QPen(Qt::blue));
	customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
	customPlot->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 4));
	// add error bars:
	QCPErrorBars *errorBars = new QCPErrorBars(customPlot->xAxis, customPlot->yAxis);
	errorBars->removeFromLegend();
	errorBars->setAntialiased(false);
	errorBars->setDataPlottable(customPlot->graph(3));
	errorBars->setPen(QPen(QColor(180, 180, 180)));
	customPlot->graph(3)->setName("Measurement");

	// generate ideal sinc curve data and some randomly perturbed data for scatter plot:
	QVector<double> x0(250), y0(250);
	QVector<double> yConfUpper(250), yConfLower(250);
	for (int i = 0; i < 250; ++i)
	{
		x0[i] = (i / 249.0 - 0.5) * 30 + 0.01; // by adding a small offset we make sure not do divide by zero in next code line
		y0[i] = qSin(x0[i]) / x0[i]; // sinc function
		yConfUpper[i] = y0[i] + 0.15;
		yConfLower[i] = y0[i] - 0.15;
		x0[i] *= 1000;
	}
	QVector<double> x1(50), y1(50), y1err(50);
	for (int i = 0; i < 50; ++i)
	{
		// generate a gaussian distributed random number:
		double tmp1 = rand() / (double)RAND_MAX;
		double tmp2 = rand() / (double)RAND_MAX;
		double r = qSqrt(-2 * qLn(tmp1))*qCos(2 * M_PI*tmp2); // box-muller transform for gaussian distribution
		// set y1 to value of y0 plus a random gaussian pertubation:
		x1[i] = (i / 50.0 - 0.5) * 30 + 0.25;
		y1[i] = qSin(x1[i]) / x1[i] + r*0.15;
		x1[i] *= 1000;
		y1err[i] = 0.15;
	}
	// pass data to graphs and let QCustomPlot determine the axes ranges so the whole thing is visible:
	customPlot->graph(0)->setData(x0, yConfUpper);
	customPlot->graph(1)->setData(x0, yConfLower);
	customPlot->graph(2)->setData(x0, y0);
	customPlot->graph(3)->setData(x1, y1);
	errorBars->setData(y1err);
	customPlot->graph(2)->rescaleAxes();
	customPlot->graph(3)->rescaleAxes(true);
	// setup look of bottom tick labels:
	customPlot->xAxis->setTickLabelRotation(30);
	customPlot->xAxis->ticker()->setTickCount(9);
	customPlot->xAxis->setNumberFormat("ebc");
	customPlot->xAxis->setNumberPrecision(1);
	customPlot->xAxis->moveRange(-10);
	// make top right axes clones of bottom left axes. Looks prettier:
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

