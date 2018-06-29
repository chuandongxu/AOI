#pragma once

#include "EditInspWindowBaseWidget.h"
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <memory>
#include "ui_InspBridgeWidget.h"

class InspBridgeWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    InspBridgeWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~InspBridgeWidget();
    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

private slots:
    void on_inspModeChanged(int index);

private:
    Ui::InspBridgeWidget ui;
    std::unique_ptr<QComboBox>  m_pComboBoxInspMode;
    std::unique_ptr<QCheckBox>  m_pCheckLeft;
    std::unique_ptr<QCheckBox>  m_pCheckRight;
    std::unique_ptr<QCheckBox>  m_pCheckUp;
    std::unique_ptr<QCheckBox>  m_pCheckDown;
    std::unique_ptr<QLineEdit>  m_pEditMaxWidth;
    std::unique_ptr<QLineEdit>  m_pEditMaxHeight;
};
