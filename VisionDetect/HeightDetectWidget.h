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
    void on_measureChanged(int index);
    void on_baseTypeChanged(int index);   

    void onRelHtChanged(bool bInsp);

private:
    Ui::HeightDetectWidget ui;

    std::unique_ptr<QComboBox>  m_pComboxMeasureType;
    std::unique_ptr<QComboBox>  m_pComboxBaseType;   
    std::unique_ptr<QLineEdit>  m_pEditBaseScale;
    std::unique_ptr<QLineEdit>  m_pEditMinRange;
    std::unique_ptr<QLineEdit>  m_pEditMaxRange;

    SpecAndResultWidgetPtr      m_pSpecAndResultAbsHt;
    SpecAndResultWidgetPtr      m_pSpecAndResultMaxHtErr;
    SpecAndResultWidgetPtr      m_pSpecAndResultMinHtErr;
    std::unique_ptr<QCheckBox>  m_pCheckBoxRelHt;
    SpecAndResultWidgetPtr      m_pSpecAndResultLefRigRelHt;
    SpecAndResultWidgetPtr      m_pSpecAndResultTopBomRelHt;
};
