#pragma once

#include <QLineEdit>
#include <QComboBox>

#include "EditInspWindowBaseWidget.h"
#include "ui_InspChipWidget.h"

class InspChipWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    InspChipWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~InspChipWidget();
    
    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

private:
    bool _learnChip(int &recordId);
    bool _inspChip(int recordId, bool bShowResult = true);

private:
    Ui::InspChipWidget ui;
    std::unique_ptr<QComboBox>  m_pComboBoxInspMode;
    std::unique_ptr<QLineEdit>  m_pEditRecordID;

    bool                        m_bIsTryInspected = false;
};
