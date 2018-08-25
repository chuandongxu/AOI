#pragma once

#include <QWidget>

#include "EditInspWindowBaseWidget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QStandardItemModel>
#include <memory>

#include "ui_HeightDetectWidget.h"
#include "SpecAndResultWidget.h"

class HeightDetectWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    HeightDetectWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~HeightDetectWidget();
    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

protected slots:
    void onBaseTypeChanged(bool bInsp);
    void onMeasureTypeChanged(bool bInsp);

private:
    Ui::HeightDetectWidget ui;

    std::unique_ptr<QCheckBox>  m_pCheckBoxBase;
    std::unique_ptr<QCheckBox>  m_pCheckBoxMeasure;
    std::unique_ptr<QLineEdit>  m_pEditMinRange;
    std::unique_ptr<QLineEdit>  m_pEditMaxRange;
    SpecAndResultWidgetPtr      m_pSpecAndResultMaxRelHt;
    SpecAndResultWidgetPtr      m_pSpecAndResultMinRelHt;
};
