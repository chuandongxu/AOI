#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <memory>

#include "ui_FindCircleWidget.h"
#include "EditInspWindowBaseWidget.h"

class FindCircleWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    FindCircleWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~FindCircleWidget();
    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

private:
    Ui::FindCircleWidget ui;

    std::unique_ptr<QComboBox>    m_pComboBoxInnerAttribute;
    std::unique_ptr<QCheckBox>  m_pCheckBoxFindPair;
    std::unique_ptr<QLineEdit>  m_pEditStartSrchAngle;
    std::unique_ptr<QLineEdit>  m_pEditEndSrchAngle;
    std::unique_ptr<QLineEdit>  m_pEditCaliperCount;
    std::unique_ptr<QLineEdit>  m_pEditCaliperWidth;
    std::unique_ptr<QLineEdit>  m_pEditEdgeThreshold;
    std::unique_ptr<QComboBox>  m_pComboBoxEdgeSelectMethod;
    std::unique_ptr<QLineEdit>  m_pEditRmStrayPointRatio;
    std::unique_ptr<QLineEdit>  m_pEditDiffFilterHalfW;
    std::unique_ptr<QLineEdit>  m_pEditDiffFilterSigma;
};
