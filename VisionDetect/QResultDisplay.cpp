#include "QResultDisplay.h"
#include "qcustomplot.h"

QResultDisplay::QResultDisplay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    m_pCustomPlot1 = new QCustomPlot(this);
    //setupSimpleDemo(m_pCustomPlot1);
    ui.dockWidget->setWidget(m_pCustomPlot1);

    m_pCustomPlot2 = new QCustomPlot(this);
    //setupSincScatterDemo(m_pCustomPlot2);
    ui.dockWidget_2->setWidget(m_pCustomPlot2);
}

QResultDisplay::~QResultDisplay()
{
    if (m_pCustomPlot1) delete m_pCustomPlot1;
    if (m_pCustomPlot2) delete m_pCustomPlot2;
}

void QResultDisplay::setupPlot1Data(VectorOfVectorOfFloat& dataStepPhase, VectorOfFloat& dataSlop)
{
    if (m_pCustomPlot1) delete m_pCustomPlot1;
    m_pCustomPlot1 = new QCustomPlot(this);
    ui.dockWidget->setWidget(m_pCustomPlot1);

    if (dataStepPhase.size() == 0 || (dataStepPhase[0].size() != dataSlop.size())) return;

    m_pCustomPlot1->legend->setVisible(true);
    m_pCustomPlot1->legend->setFont(QFont("Helvetica", 9));

    QPen pen;
    int nSlopNum = dataStepPhase[0].size();
    int nDataNum = dataStepPhase.size();
    // add graphs with different line styles:
    for (int i = 0; i < nSlopNum; i++)
    {
        m_pCustomPlot1->addGraph();
        pen.setColor(QColor(qSin(i * 1 + 1.2) * 80 + 80, qSin(i*0.3 + 0) * 80 + 80, qSin(i*0.3 + 1.5) * 80 + 80));
        m_pCustomPlot1->graph()->setPen(pen);
        m_pCustomPlot1->graph()->setName(QString("Slop %1").arg(i+1));
        m_pCustomPlot1->graph()->setLineStyle(QCPGraph::lsLine);
        m_pCustomPlot1->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

        // generate data:
        QVector<double> x(nDataNum), y(nDataNum);
        for (int j = 0; j < nDataNum; ++j)
        {
            x[j] = j/*dataStepPhase[j][i]*/;
            y[j] = dataSlop[i] * x[j];
        }        

        m_pCustomPlot1->graph()->setData(x, y);
        m_pCustomPlot1->graph()->rescaleAxes(true);
    }

    // zoom out a bit:
    m_pCustomPlot1->yAxis->scaleRange(1.1, m_pCustomPlot1->yAxis->range().center());
    m_pCustomPlot1->xAxis->scaleRange(1.1, m_pCustomPlot1->xAxis->range().center());
    // set blank axis lines:
    m_pCustomPlot1->xAxis->setTicks(true);
    m_pCustomPlot1->yAxis->setTicks(true);
    m_pCustomPlot1->xAxis->setTickLabels(true);
    m_pCustomPlot1->yAxis->setTickLabels(true);
    // make top right axes clones of bottom left axes:
    m_pCustomPlot1->axisRect()->setupFullAxesBox();    
}

void QResultDisplay::setupPlot2Data(VectorOfVectorOfFloat& dataStepPhase, VectorOfVectorOfFloat& dataStepPhaseDiff)
{
    if (m_pCustomPlot2) delete m_pCustomPlot2;
    m_pCustomPlot2 = new QCustomPlot(this);
    ui.dockWidget_2->setWidget(m_pCustomPlot2);

    if (dataStepPhase.size() != dataStepPhaseDiff.size()) return;

    m_pCustomPlot2->legend->setVisible(true);
    m_pCustomPlot2->legend->setFont(QFont("Helvetica", 9));

    QPen pen;
    int nSlopNum = dataStepPhase[0].size();
    int nDataNum = dataStepPhase.size();
    // add graphs with different line styles:
    for (int i = 0; i < nSlopNum; i++)
    {
        m_pCustomPlot2->addGraph();
        pen.setColor(QColor(qSin(i * 1 + 1.2) * 80 + 80, qSin(i*0.3 + 0) * 80 + 80, qSin(i*0.3 + 1.5) * 80 + 80));
        m_pCustomPlot2->graph()->setPen(pen);
        m_pCustomPlot2->graph()->setName(QString("Slop %1").arg(i + 1));
        m_pCustomPlot2->graph()->setLineStyle(QCPGraph::lsLine);
        m_pCustomPlot2->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
        // generate data:
        QVector<double> x(nDataNum), y(nDataNum);
        for (int j = 0; j < nDataNum; ++j)
        {
            x[j] = j/*dataStepPhase[j][i]*/;
            y[j] = dataStepPhaseDiff[j][i];
        }
        m_pCustomPlot2->graph()->setData(x, y);
        m_pCustomPlot2->graph()->rescaleAxes(true);
    }

    // zoom out a bit:
    m_pCustomPlot2->yAxis->scaleRange(1.1, m_pCustomPlot2->yAxis->range().center());
    m_pCustomPlot2->xAxis->scaleRange(1.1, m_pCustomPlot2->xAxis->range().center());
    // set blank axis lines:
    m_pCustomPlot2->xAxis->setTicks(true);
    m_pCustomPlot2->yAxis->setTicks(true);
    m_pCustomPlot2->xAxis->setTickLabels(true);
    m_pCustomPlot2->yAxis->setTickLabels(true);
    // make top right axes clones of bottom left axes:
    m_pCustomPlot2->axisRect()->setupFullAxesBox();    
}

void QResultDisplay::setupSimpleDemo(QCustomPlot *customPlot)
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

void QResultDisplay::setupSincScatterDemo(QCustomPlot *customPlot)
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

