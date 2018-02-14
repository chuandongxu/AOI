#include "InspWindowSelectWidget.h"
#include "InspWindowWidget.h"

InspWindowSelectWidget::InspWindowSelectWidget(InspWindowWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    m_pParent = parent;
}

InspWindowSelectWidget::~InspWindowSelectWidget()
{
}

void InspWindowSelectWidget::on_btnVoidInspWindow_clicked()
{
    m_pParent->setCurrentIndex ( INSP_VOID );
}