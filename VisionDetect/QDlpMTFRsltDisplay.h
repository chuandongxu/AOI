#pragma once

#include <QWidget>
#include "ui_QDlpMTFRsltDisplay.h"

using VectorOfFloat = std::vector < float > ;
using VectorOfVectorOfFloat = std::vector < VectorOfFloat > ;

class QCustomPlot;
class QDlpMTFRsltDisplay : public QWidget
{
    Q_OBJECT

public:
    QDlpMTFRsltDisplay(QWidget *parent = Q_NULLPTR);
    ~QDlpMTFRsltDisplay();

public:
    void setupPlot1Data(QString& plotName, VectorOfVectorOfFloat& dataDisplay, std::vector<QString>& plotDataName);
    void setupPlot2Data(QString& plotName, VectorOfVectorOfFloat& dataDisplay, std::vector<QString>& plotDataName);
    void setupPlot3Data(QString& plotName, VectorOfVectorOfFloat& dataDisplay, std::vector<QString>& plotDataName);
    void setupPlot4Data(QString& plotName, VectorOfVectorOfFloat& dataDisplay, std::vector<QString>& plotDataName);

private:
    void setupPlotData(VectorOfVectorOfFloat& dataDisplay, std::vector<QString>& plotDataName, QCustomPlot *customPlot);

private:
    Ui::QDlpMTFRsltDisplay ui;

private:
    QCustomPlot* m_pCustomPlot1;
    QCustomPlot* m_pCustomPlot2;
    QCustomPlot* m_pCustomPlot3;
    QCustomPlot* m_pCustomPlot4;
};
