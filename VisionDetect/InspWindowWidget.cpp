#include "InspWindowWidget.h"
#include "../include/VisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"

InspWindowWidget::InspWindowWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    m_pInspWindowSelectWidget = new InspWindowSelectWidget(this);
    //m_pInspWindowSelectWidget->setParent(this);
    ui.stackedWidget->addWidget ( m_pInspWindowSelectWidget );

    m_pInspVoidWidget = new InspVoidWidget(this);
    ui.stackedWidget->addWidget ( m_pInspVoidWidget );    
}

InspWindowWidget::~InspWindowWidget()
{
}

void InspWindowWidget::setCurrentIndex(int index)
{
    ui.stackedWidget->setCurrentIndex ( index );
}

void InspWindowWidget::showEvent(QShowEvent *event)
{
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SELECT_ROI);
}

void InspWindowWidget::on_btnReturn_clicked()
{
    ui.stackedWidget->setCurrentIndex ( INSP_WINDOW_SELECT );
}