﻿#include "rightwidget.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../include/IData.h"
#include "../include/IVision.h"
#include "../include/ICamera.h"
#include "../include/IFlowCtrl.h"
#include <QStyleOption>
#include <QPainter>
#include "../common/SystemData.h"
#include "../Common/eos.h"

QRightWidget::QRightWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    this->setFixedWidth(410);

    ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
    if (pCam)
    {
        ui.stackedWidget->addWidget(new QWidget());
    }

    IVision* pVision = getModule<IVision>(VISION_MODEL);
    if (pVision)
    {
        ui.stackedWidget->addWidget(pVision->getDetectView());
        ui.stackedWidget->addWidget(pVision->getInspWindowView());
        ui.stackedWidget->addWidget(pVision->getCaliView());
    }

    IData * pData = getModule<IData>(DATA_MODEL);    
    if (pData) {
        ui.stackedWidget->addWidget(pData->getDataWidget());        
    }
    IFlowCtrl * p = getModule<IFlowCtrl>(CTRL_MODEL);
    if (p)
    {
        ui.stackedWidget->addWidget(p->getAutoRunPanel());
    }
    ui.stackedWidget->setCurrentIndex(0);    

    QEos::Attach(EVENT_RUN_STATE, this, SLOT(onRunState(const QVariantList &)));
    QEos::Attach(EVENT_UI_STATE, this, SLOT(onUIState(const QVariantList &)));

    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, QColor(235, 235, 235));
    setAutoFillBackground(true);
    setPalette(Pal);    
}

QRightWidget::~QRightWidget()
{
}

void QRightWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption option;
    option.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);

    QWidget::paintEvent(event);
}

void QRightWidget::onRunState(const QVariantList &data)
{
    if (data.size() == 1)
    {
        int iState = data[0].toInt();
        if (RUN_STATE_RUNING == iState)
        {
            //m_widgetRun->setVisible(true);
            //m_widgetEdit->setVisible(false);
        }
        else
        {
            //m_widgetRun->setVisible(false);
            //m_widgetEdit->setVisible(true);
        }
    }
}

void QRightWidget::onUIState(const QVariantList &data)
{
    int iBoard = data[0].toInt();
    int iEvent = data[1].toInt();
    //if (iEvent != RUN_UI_STATE_SETTING) return;

    switch (iEvent)
    {
    case RUN_UI_STATE_HARDWARE:
        ui.stackedWidget->setCurrentIndex(0);
        break;
    case RUN_UI_STATE_SETTING:
        ui.stackedWidget->setCurrentIndex(1);
        break;
    case RUN_UI_STATE_TOOLS:
        ui.stackedWidget->setCurrentIndex(2);
        break;
    case RUN_UI_STATE_CALI:
        ui.stackedWidget->setCurrentIndex(3);
        break;
    case RUN_UI_STATE_DATA:
        ui.stackedWidget->setCurrentIndex(4);
        break;
    case RUN_UI_STATE_AUTORUN:
        ui.stackedWidget->setCurrentIndex(5);
        break;
    default:
        break;
    }
}

