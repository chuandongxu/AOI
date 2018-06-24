#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <memory>

#include "ui_FindLineWidget.h"
#include "EditInspWindowBaseWidget.h"
#include "SpecAndResultWidget.h"

class FindLineWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    FindLineWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~FindLineWidget();
    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

private:
    Ui::FindLineWidget ui;    

    std::unique_ptr<QCheckBox>  m_pCheckBoxFindPair;
    std::unique_ptr<QComboBox>  m_pComboBoxFindLineDirection;
    std::unique_ptr<QLineEdit>  m_pEditCaliperCount;
    std::unique_ptr<QLineEdit>  m_pEditCaliperWidth;
    std::unique_ptr<QLineEdit>  m_pEditEdgeThreshold;
    std::unique_ptr<QComboBox>  m_pComboBoxEdgeSelectMethod;
    std::unique_ptr<QLineEdit>  m_pEditRmStrayPointRatio;
    std::unique_ptr<QLineEdit>  m_pEditDiffFilterHalfW;
    std::unique_ptr<QLineEdit>  m_pEditDiffFilterSigma;
    std::unique_ptr<QLineEdit>  m_pEditPointMaxOffset;
    std::unique_ptr<QCheckBox>  m_pCheckLinerity;    
    SpecAndResultWidgetPtr      m_pSpecAndResultMinLinearity;
    std::unique_ptr<QCheckBox>  m_pEditCheckAngle;
    SpecAndResultWidgetPtr      m_pSpecAndResultAngle;
    SpecAndResultWidgetPtr      m_pSpecAndResultAngleDiffTol;
};
