#pragma once

#include <QWidget>
#include "ui_QResultDisplay.h"

using VectorOfFloat = std::vector < float > ;
using VectorOfVectorOfFloat = std::vector < VectorOfFloat > ;

class QCustomPlot;
class QResultDisplay : public QWidget
{
    Q_OBJECT

public:
    QResultDisplay(QWidget *parent = Q_NULLPTR);
    ~QResultDisplay();

public:    
    void setupPlot1Data(VectorOfVectorOfFloat& dataStepPhase, VectorOfFloat& dataSlop);
    void setupPlot2Data(VectorOfVectorOfFloat& dataStepPhase, VectorOfVectorOfFloat& dataStepPhaseDiff);

private:
    void setupSimpleDemo(QCustomPlot *customPlot);
    void setupSincScatterDemo(QCustomPlot *customPlot);

private:
    Ui::QResultDisplay ui;

private:
    QCustomPlot* m_pCustomPlot1;
    QCustomPlot* m_pCustomPlot2;
};
