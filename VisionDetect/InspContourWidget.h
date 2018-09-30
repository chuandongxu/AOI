#pragma once
#include <QLineEdit>

#include "EditInspWindowBaseWidget.h"
#include "ui_InspContourWidget.h"

class InspContourWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    InspContourWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~InspContourWidget();

    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

private:
    bool _learnContour(int &recordId);
    bool _inspContour(int recordId, bool bShowResult = true);

private:
    Ui::InspContourWidget ui;
    std::unique_ptr<QLineEdit>    m_pEditDefectThreshold;
    std::unique_ptr<QLineEdit>    m_pEditMinDefectArea;
    std::unique_ptr<QLineEdit>    m_pEditDefectInnerLengthTol;
    std::unique_ptr<QLineEdit>    m_pEditDefectOuterLengthTol;
    std::unique_ptr<QLineEdit>    m_pEditInnerMaskDepth;
    std::unique_ptr<QLineEdit>    m_pEditOuterMaskDepth;
    std::unique_ptr<QLineEdit>  m_pEditRecordID;
};
