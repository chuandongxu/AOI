#include "QDlpMTFRsltDisplay.h"
#include "qcustomplot.h"

QDlpMTFRsltDisplay::QDlpMTFRsltDisplay(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	m_pCustomPlot1 = new QCustomPlot(this);
	ui.dockWidget->setWidget(m_pCustomPlot1);

	m_pCustomPlot2 = new QCustomPlot(this);	
	ui.dockWidget_2->setWidget(m_pCustomPlot2);

	m_pCustomPlot3 = new QCustomPlot(this);
	ui.dockWidget_3->setWidget(m_pCustomPlot3);

	m_pCustomPlot4 = new QCustomPlot(this);
	ui.dockWidget_4->setWidget(m_pCustomPlot4);
}

QDlpMTFRsltDisplay::~QDlpMTFRsltDisplay()
{
	if (m_pCustomPlot1) delete m_pCustomPlot1;
	if (m_pCustomPlot2) delete m_pCustomPlot2;
	if (m_pCustomPlot3) delete m_pCustomPlot3;
	if (m_pCustomPlot4) delete m_pCustomPlot4;
}


void QDlpMTFRsltDisplay::setupPlot1Data(QString& plotName, VectorOfVectorOfFloat& dataDisplay, std::vector<QString>& plotDataName)
{
	if (m_pCustomPlot1) delete m_pCustomPlot1;
	m_pCustomPlot1 = new QCustomPlot(this);
	ui.dockWidget->setWidget(m_pCustomPlot1);

	ui.dockWidget->setWindowTitle(plotName);

	setupPlotData(dataDisplay, plotDataName, m_pCustomPlot1);
}

void QDlpMTFRsltDisplay::setupPlot2Data(QString& plotName, VectorOfVectorOfFloat& dataDisplay, std::vector<QString>& plotDataName)
{
	if (m_pCustomPlot2) delete m_pCustomPlot2;
	m_pCustomPlot2 = new QCustomPlot(this);
	ui.dockWidget_2->setWidget(m_pCustomPlot2);

	ui.dockWidget_2->setWindowTitle(plotName);

	setupPlotData(dataDisplay, plotDataName, m_pCustomPlot2);
}

void QDlpMTFRsltDisplay::setupPlot3Data(QString& plotName, VectorOfVectorOfFloat& dataDisplay, std::vector<QString>& plotDataName)
{
	if (m_pCustomPlot3) delete m_pCustomPlot3;
	m_pCustomPlot3 = new QCustomPlot(this);
	ui.dockWidget_3->setWidget(m_pCustomPlot3);

	ui.dockWidget_3->setWindowTitle(plotName);

	setupPlotData(dataDisplay, plotDataName, m_pCustomPlot3);
}

void QDlpMTFRsltDisplay::setupPlot4Data(QString& plotName, VectorOfVectorOfFloat& dataDisplay, std::vector<QString>& plotDataName)
{
	if (m_pCustomPlot4) delete m_pCustomPlot4;
	m_pCustomPlot4 = new QCustomPlot(this);
	ui.dockWidget_4->setWidget(m_pCustomPlot4);

	ui.dockWidget_4->setWindowTitle(plotName);

	setupPlotData(dataDisplay, plotDataName, m_pCustomPlot4);
}


void QDlpMTFRsltDisplay::setupPlotData(VectorOfVectorOfFloat& dataDisplay, std::vector<QString>& plotDataName, QCustomPlot *customPlot)
{
	if (dataDisplay.size() != plotDataName.size()) return;

	customPlot->legend->setVisible(true);
	customPlot->legend->setFont(QFont("Helvetica", 9));

	QPen pen;
	int nSlopNum = dataDisplay.size();
	// add graphs with different line styles:
	for (int i = 0; i < nSlopNum; i++)
	{
		customPlot->addGraph();
		pen.setColor(QColor(qSin(i * 3 + 1.2) * 80 + 80, qSin(i*1 + 0) * 80 + 80, qSin(i*0.3 + 1.5) * 80 + 80));
		customPlot->graph()->setPen(pen);
		customPlot->graph()->setName(QString(plotDataName[i]));
		customPlot->graph()->setLineStyle(QCPGraph::lsLine);
		customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
		// generate data:
		int nDataNum = dataDisplay[i].size();
		QVector<double> x(nDataNum), y(nDataNum);
		for (int j = 0; j < nDataNum; ++j)
		{
			x[j] = j+1;
			y[j] = dataDisplay[i][j];
		}
		customPlot->graph()->setData(x, y);
		customPlot->graph()->rescaleAxes(true);
	}

	// zoom out a bit:
	customPlot->yAxis->scaleRange(0.15, customPlot->yAxis->range().center()*0.15);
	customPlot->xAxis->scaleRange(1.1, customPlot->xAxis->range().center());
	// set blank axis lines:
	customPlot->xAxis->setTicks(true);
	customPlot->yAxis->setTicks(true);
	customPlot->xAxis->setTickLabels(true);
	customPlot->yAxis->setTickLabels(true);
	// make top right axes clones of bottom left axes:
	customPlot->axisRect()->setupFullAxesBox();
}
