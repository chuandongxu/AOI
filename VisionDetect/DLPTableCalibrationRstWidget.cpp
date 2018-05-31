#include "DLPTableCalibrationRstWidget.h"

#include "qcustomplot.h"

#include "../Common/SystemData.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

const int IMG_DISPLAY_WIDTH = 1200;
const int IMG_DISPLAY_HEIGHT = 400;

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
    m_pProfGrayPlotX = std::make_shared<QCustomPlot>();
    _setupGrayPlotData(m_pProfGrayPlotX, m_vecVecFrameChartData, true);
    m_profGrayDataSeneX = std::make_shared<QGraphicsScene>();
    ui.graphicsView_rstX->setScene(m_profGrayDataSeneX.get());
    ui.graphicsView_rstX->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_rstX->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_rstX->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
    m_pProfGrayPlotX->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
    m_profGrayDataSeneX->addPixmap(m_pProfGrayPlotX->grab());

    m_pProfGrayPlotY = std::make_shared<QCustomPlot>();
    _setupGrayPlotData(m_pProfGrayPlotY, m_vecVecFrameChartData, true);
    m_profGrayDataSeneY = std::make_shared<QGraphicsScene>();
    ui.graphicsView_rstY->setScene(m_profGrayDataSeneY.get());
    ui.graphicsView_rstY->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_rstY->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_rstY->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
    m_pProfGrayPlotY->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
    m_profGrayDataSeneY->addPixmap(m_pProfGrayPlotY->grab());
}

void DLPTableCalibrationRstWidget::_setupGrayPlotData(std::shared_ptr<QCustomPlot> customPlot, Vision::VectorOfVectorOfPoint2f& profData, bool bCreate)
{
    if (bCreate)
    {
        customPlot->legend->setVisible(true);
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
            x[j] = profData[i][j].x;
            y[j] = profData[i][j].y;
        }
        customPlot->graph()->setData(x, y);
        customPlot->graph()->rescaleAxes(true);
    }

    if (bCreate)
    {
        //customPlot->xAxis->setLabel("Frequence (Lp/mm)");
        //customPlot->yAxis->setLabel("MTF [%]");

        // zoom out a bit:
        customPlot->yAxis->scaleRange(0.05, -customPlot->yAxis->range().center()*0.05);
        customPlot->xAxis->scaleRange(1.0, customPlot->xAxis->range().center());

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
    Vision::VectorOfVectorOfPoint2f::iterator itr = m_vecVecFrameChartData.begin();

    if (m_vecVecFrameChartData.size() >= 8)
    {
        Vision::VectorOfVectorOfPoint2f  vecVecFrameChartData;
        vecVecFrameChartData.assign(itr, itr + 4);
        _setupGrayPlotData(m_pProfGrayPlotX, vecVecFrameChartData);

        m_pProfGrayPlotX->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
        m_profGrayDataSeneX->clear();
        m_profGrayDataSeneX->addPixmap(m_pProfGrayPlotX->grab());
        ui.graphicsView_rstX->update();

        vecVecFrameChartData.assign(itr + 4, itr + 8);
        _setupGrayPlotData(m_pProfGrayPlotY, vecVecFrameChartData);

        m_pProfGrayPlotY->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
        m_profGrayDataSeneY->clear();
        m_profGrayDataSeneY->addPixmap(m_pProfGrayPlotY->grab());
        ui.graphicsView_rstY->update();
    }   
}
