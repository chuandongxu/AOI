#pragma once

#include <QComboBox>
#include <memory>

#include "ui_InspVoidWidget.h"
#include "EditInspWindowBaseWidget.h"
#include "SpecAndResultWidget.h"

class InspVoidWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    InspVoidWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~InspVoidWidget();
    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;
    
private slots:
    void on_inspModeChanged(int index);

private:
    Ui::InspVoidWidget          ui;
    std::unique_ptr<QComboBox>  m_pComboBoxInspMode;
    SpecAndResultWidgetPtr      m_pSpecAndResultMaxAreaRatio;
    SpecAndResultWidgetPtr      m_pSpecAndResultMinAreaRatio;
    std::unique_ptr<QLineEdit>  m_pEditMaxHoleArea;
    std::unique_ptr<QLineEdit>  m_pEditMinHoleArea;
    SpecAndResultWidgetPtr      m_pSpecAndResultMaxHoleCount;
    SpecAndResultWidgetPtr      m_pSpecAndResultMinHoleCount;    
};
