#pragma once

#include <QWidget>
#include "ui_DLPTableCalibrationRstWidget.h"
#include "VisionAPI.h"

using namespace AOI;

class QCustomPlot;
class DLPTableCalibrationRstWidget : public QWidget
{
    Q_OBJECT

public:
    DLPTableCalibrationRstWidget(QWidget *parent = Q_NULLPTR);
    ~DLPTableCalibrationRstWidget();

    void setFrameChartData(Vision::VectorOfVectorOfFloat&  vecVecFrameChartData){ m_vecVecFrameChartData = vecVecFrameChartData; _updateChart(); }

protected:
    void closeEvent(QCloseEvent *e);

private:
    void _initUI();

    void _setupGrayPlotData(std::shared_ptr<QCustomPlot> customPlot, Vision::VectorOfVectorOfFloat& profData, bool bCreate = false);

    void _updateChart();

private:
    Ui::DLPTableCalibrationRstWidget ui;

    Vision::VectorOfVectorOfFloat  m_vecVecFrameChartData;

    std::shared_ptr<QGraphicsScene> m_profGrayDataSene;
    std::shared_ptr<QCustomPlot> m_pProfGrayPlot;
};
