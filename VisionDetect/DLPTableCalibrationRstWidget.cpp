#include "DLPTableCalibrationRstWidget.h"

#include "qcustomplot.h"

#include "../Common/SystemData.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

const int IMG_DISPLAY_WIDTH = 1200;
const int IMG_DISPLAY_HEIGHT = 800;

DLPTableCalibrationRstWidget::DLPTableCalibrationRstWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    _initUI();
}

DLPTableCalibrationRstWidget::~DLPTableCalibrationRstWidget()
{
}

void DLPTableCalibrationRstWidget::closeEvent(QCloseEvent *e){
    //qDebug() << "¹Ø±ÕÊÂ¼þ";
    e->ignore();

    this->hide();
}


void DLPTableCalibrationRstWidget::_initUI()
{
    m_pProfGrayPlot = std::make_shared<QCustomPlot>();
    _setupGrayPlotData(m_pProfGrayPlot, m_vecVecFrameChartData, true);
    m_profGrayDataSene = std::make_shared<QGraphicsScene>();
    ui.graphicsView_rst->setScene(m_profGrayDataSene.get());
    ui.graphicsView_rst->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_rst->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_rst->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
    m_pProfGrayPlot->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
    m_profGrayDataSene->addPixmap(m_pProfGrayPlot->grab());
}

void DLPTableCalibrationRstWidget::_setupGrayPlotData(std::shared_ptr<QCustomPlot> customPlot, Vision::VectorOfVectorOfFloat& profData, bool bCreate)
{
    if (bCreate)
    {
        customPlot->legend->setVisible(false);
        customPlot->legend->setFont(QFont("Helvetica", 9));
    }

    customPlot->clearGraphs();
    QPen pen;
    int nSlopNum = profData.size();
    // add graphs with different line styles:
    for (int i = 0; i < nSlopNum; i++)
    {
        customPlot->addGraph();
        pen.setColor(QColor(qSin(i * 3 + 1.2) * 80 + 80, qSin(i * 1 + 0) * 80 + 80, qSin(i*0.3 + 1.5) * 80 + 80));
        customPlot->graph()->setPen(pen);
        customPlot->graph()->setName(QString("DLP%1 Offset").arg(i + 1));
        customPlot->graph()->setLineStyle(QCPGraph::lsLine);
        customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 1));
        // generate data:
        int nDataNum = profData[i].size();
        QVector<double> x(nDataNum), y(nDataNum);
        for (int j = 0; j < nDataNum; ++j)
        {
            x[j] = j + 1;
            y[j] = profData[i][j];
        }
        customPlot->graph()->setData(x, y);
        customPlot->graph()->rescaleAxes(true);
    }

    if (bCreate)
    {
        //customPlot->xAxis->setLabel("Frequence (Lp/mm)");
        //customPlot->yAxis->setLabel("MTF [%]");

        // zoom out a bit:
        customPlot->yAxis->scaleRange(1.1, customPlot->yAxis->range().center());
        customPlot->xAxis->scaleRange(1.1, customPlot->xAxis->range().center());

        // set blank axis lines:
        //customPlot->xAxis->setTicks(true);
        customPlot->yAxis->setTicks(true);
        //customPlot->xAxis->setTickLabels(true);
        customPlot->yAxis->setTickLabels(true);

        // make top right axes clones of bottom left axes:
        customPlot->axisRect()->setupFullAxesBox();
    }
}

void DLPTableCalibrationRstWidget::_updateChart()
{
    _setupGrayPlotData(m_pProfGrayPlot, m_vecVecFrameChartData);

    m_pProfGrayPlot->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);

    m_profGrayDataSene->clear();
    m_profGrayDataSene->addPixmap(m_pProfGrayPlot->grab());
    ui.graphicsView_rst->update();
}
