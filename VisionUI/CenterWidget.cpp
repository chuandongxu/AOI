#include "CenterWidget.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../include/IFlowCtrl.h"
#include <QStyleOption>
#include <QPainter>

QCenterWidget::QCenterWidget(QWidget *parent)
    :QFrame(parent)
{
    IFlowCtrl * p = getModule<IFlowCtrl>(CTRL_MODEL);
    if(!p)return;

    m_widget = p->getFlowCtrlWidget();

    m_rightWidget = new QRightWidget;
    m_rightWidget->setObjectName("rightWidget");
    m_rightLayout = new QHBoxLayout;
    m_rightLayout->setContentsMargins(0, 0, 0, 0);
    m_rightLayout->addWidget(m_rightWidget);

    m_subVLayout = new QHBoxLayout;
    m_subVLayout->setContentsMargins(0, 0, 0, 0);
    m_subVLayout->addWidget(m_widget);
    m_subVLayout->addWidget(m_rightWidget);
    m_subVLayout->setAlignment(Qt::AlignHCenter);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->addLayout(m_subVLayout);
    this->setLayout(m_mainLayout);

    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, QColor(235, 235, 235));
    setAutoFillBackground(true);
    setPalette(Pal);
}

void QCenterWidget::setCenterWidget(QWidget * w)
{
    
}

void QCenterWidget::paintEvent(QPaintEvent *event)
{  
    QStyleOption option;
    option.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);

    QWidget::paintEvent(event);
}