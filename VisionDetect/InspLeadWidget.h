#pragma once

#include <QLineEdit>
#include <QComboBox>

#include "EditInspWindowBaseWidget.h"
#include "ui_InspLeadWidget.h"
#include "VisionAPI.h"
#include "SpecAndResultWidget.h"

using namespace AOI;

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
    void _autoAddInspWindows(const Vision::PR_AUTO_LOCATE_LEAD_RPY &stRpy);

private:
    Ui::InspLeadWidget ui;
    std::unique_ptr<QLineEdit>  m_pEditPadRecordID;
    std::unique_ptr<QLineEdit>  m_pEditLeadRecordID;
    std::unique_ptr<QLineEdit>  m_pEditPadLeadDist;
    SpecAndResultWidgetPtr      m_pSpecAndResultMaxOffsetX;
    SpecAndResultWidgetPtr      m_pSpecAndResultMaxOffsetY;
};

