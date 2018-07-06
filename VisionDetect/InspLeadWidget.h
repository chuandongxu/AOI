#pragma once

#include <QLineEdit>
#include <QComboBox>

#include "EditInspWindowBaseWidget.h"
#include "ui_InspLeadWidget.h"

class InspLeadWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    InspLeadWidget(InspWindowWidget *parent = 0);
    ~InspLeadWidget();

    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

private slots:
    void on_btnAutoLocateLead_clicked();

private:
    Ui::InspLeadWidget ui;
};

