#ifndef INSP3DSOLDERWIDGET_H
#define INSP3DSOLDERWIDGET_H

#include "EditInspWindowBaseWidget.h"
#include "ui_Insp3DSolderWidget.h"
#include "SpecAndResultWidget.h"

class Insp3DSolderWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    Insp3DSolderWidget(InspWindowWidget *parent = 0);
    ~Insp3DSolderWidget();

    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

private:
    Ui::Insp3DSolderWidget ui;
    SpecAndResultWidgetPtr  m_pConductorAbsHeight;
    SpecAndResultWidgetPtr  m_pConductorAbsHeightUpLimit;
    SpecAndResultWidgetPtr  m_pConductorAbsHeightLoLimit;
    SpecAndResultWidgetPtr  m_pConductorRelHeightUpLimit;
    SpecAndResultWidgetPtr  m_pSolderHeightLoLimit;
    SpecAndResultWidgetPtr  m_pSolderHeightRatioLoLimit;
    SpecAndResultWidgetPtr  m_pSolderCovRatioLoLimit;
    std::unique_ptr<QLineEdit>  m_pSolderHeightUpLimit;
    std::unique_ptr<QLineEdit>  m_pSolderWettingWidth;
};

#endif // INSP3DSOLDERWIDGET_H
