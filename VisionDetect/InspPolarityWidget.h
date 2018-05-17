#pragma once

#include <QComboBox>
#include <QLineEdit>
#include "EditInspWindowBaseWidget.h"
#include "ui_InspPolarityWidget.h"

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

private:
    Ui::InspPolarityWidget ui;
    std::unique_ptr<QComboBox>	m_pComboBoxType;
    std::unique_ptr<QComboBox>	m_pComboBoxAttribute;
    std::unique_ptr<QLineEdit>  m_pEditIntensityDiffTol;
};
