#pragma once

#include "EditInspWindowBaseWidget.h"
#include <QComboBox>
#include <memory>
#include "ui_InspVoidWidget.h"

class InspVoidWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    InspVoidWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~InspVoidWidget();
    
private slots:
    void on_inspModeChanged(int index);
    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

private:
    Ui::InspVoidWidget          ui;
    std::unique_ptr<QComboBox>  m_pComboBoxInspMode;
    std::unique_ptr<QLineEdit>  m_pEditMaxAreaRatio;
    std::unique_ptr<QLineEdit>  m_pEditMinAreaRatio;
    std::unique_ptr<QLineEdit>  m_pEditMaxHoleCount;
    std::unique_ptr<QLineEdit>  m_pEditMinHoleCount;
    std::unique_ptr<QLineEdit>  m_pEditMaxHoleArea;
    std::unique_ptr<QLineEdit>  m_pEditMinHoleArea;
};
