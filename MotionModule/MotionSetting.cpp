#include "MotionSetting.h"
#include "../common/SystemData.h"
#include "../include/IdDefine.h"
#include <QVBoxLayout>

MotionSetting::MotionSetting(MotionControl* pCtrl, QWidget *parent)
    : m_pCtrl(pCtrl), QWidget(parent)
{
    ui.setupUi(this);

    m_pMotionIO = new MotionIO(m_pCtrl);
    m_pMotionMtr = new MotionMotor(m_pCtrl);

    initUI();    

    //m_timer = new QTimer(this);
    //connect(m_timer, SIGNAL(timeout()), this, SLOT(updataIO()));
    //m_timer->start(200);
}

MotionSetting::~MotionSetting()
{
    //if (m_timer)
    //{
    //    delete m_timer;
    //    m_timer = NULL;
    //}

    if (m_pMotionIO)
    {
        delete m_pMotionIO;
        m_pMotionIO = NULL;
    }

    if (m_pMotionMtr)
    {
        delete m_pMotionMtr;
        m_pMotionMtr = NULL;
    }
}

void MotionSetting::initUI()
{
    // IO    
    QVBoxLayout *pVLayout = new QVBoxLayout();
    pVLayout->addWidget(m_pMotionIO);
    ui.tab_2->setLayout(pVLayout);

    pVLayout = new QVBoxLayout();
    pVLayout->addWidget(m_pMotionMtr);
    ui.tab_1->setLayout(pVLayout);
}