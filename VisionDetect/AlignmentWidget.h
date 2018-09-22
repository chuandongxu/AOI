#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <memory>

#include "ui_AlignmentWidget.h"
#include "EditInspWindowBaseWidget.h"
#include "SpecAndResultWidget.h"
#include "VisionAPI.h"

using namespace AOI;

class AlignmentWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    AlignmentWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~AlignmentWidget();
    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;
    static bool learnTemplate(Vision::PR_MATCH_TMPL_ALGORITHM enAlgo, cv::Mat& matMask, const cv::Rect &rectROI, int &recordId);

private:
    bool _srchTemplate(int recordId, bool bShowResult = true);

private:
    Ui::AlignmentWidget         ui;

    std::unique_ptr<QComboBox>  m_pComboBoxAlgorithm;
    std::unique_ptr<QLineEdit>  m_pEditRecordID;
    std::unique_ptr<QCheckBox>  m_pCheckBoxSubPixel;
    std::unique_ptr<QComboBox>  m_pComboBoxMotion;
    SpecAndResultWidgetPtr      m_pSpecAndResultMinScore;
    SpecAndResultWidgetPtr      m_pSpecAndResultMaxOffsetX;
    SpecAndResultWidgetPtr      m_pSpecAndResultMaxOffsetY;
    SpecAndResultWidgetPtr      m_pSpecAndResultMaxRotation;
};
