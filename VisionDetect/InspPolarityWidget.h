#pragma once

#include <QComboBox>
#include <QLineEdit>

#include "ui_InspPolarityWidget.h"
#include "EditInspWindowBaseWidget.h"
#include "SpecAndResultWidget.h"


class InspPolarityWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    InspPolarityWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~InspPolarityWidget();

    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

protected slots:
    void onTypeChange(int index);

private:
    Ui::InspPolarityWidget ui;
    std::unique_ptr<QComboBox>	m_pComboBoxType;
    std::unique_ptr<QComboBox>	m_pComboBoxAttribute;
    SpecAndResultWidgetPtr      m_pSpecAndResultIntensityDiffTol;
};
