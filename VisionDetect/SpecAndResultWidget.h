#pragma once

#include <QTableWidget>
#include <QLineEdit>
#include <memory>
#include "ui_SpecAndResultWidget.h"

class SpecAndResultWidget : public QTableWidget
{
    Q_OBJECT

public:
    SpecAndResultWidget(QWidget *parent = Q_NULLPTR, float fMin = -10000, float fMax = 10000, int nPrecesion = 2);
    ~SpecAndResultWidget();
    void setSpec(float fSpec);
    float getSpec() const;
    void setResult(float result);
    void clearResult();

private:
    Ui::SpecAndResultWidget ui;
    std::unique_ptr<QLineEdit>  m_pEditSpec;
    std::unique_ptr<QLineEdit>  m_pEditResult;
};

using SpecAndResultWidgetPtr = std::unique_ptr<SpecAndResultWidget>;