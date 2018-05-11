#include "AutoRunPanel.h"
#include "../Common/SystemData.h"
#include <qmessagebox>
#include "../Common/eos.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IData.h"

AutoRunPanel::AutoRunPanel(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    QEos::Attach(EVENT_CHECK_STATE, this, SLOT(onStateEvent(const QVariantList &)));
    QEos::Attach(EVENT_RESULT_DISPLAY, this, SLOT(onResultEvent(const QVariantList &)));

    connect(ui.pushButton_runOnce, SIGNAL(clicked()), SLOT(onRunOnce()));
    connect(ui.checkBox_AutoRun, SIGNAL(stateChanged(int)), SLOT(onStateChangeAutoRun(int)));

    connect(ui.pushButton_startAutoRun, SIGNAL(clicked()), SLOT(onStartAutoRun()));
    connect(ui.pushButton_stopAutoRun, SIGNAL(clicked()), SLOT(onStopAutoRun()));

    ui.tableView_resultList->setModel(&m_model);

    m_bAutoRunning = false;

    updateResultList();
}

AutoRunPanel::~AutoRunPanel()
{
}

void AutoRunPanel::updateResultList()
{
    m_model.clear();

    QStringList ls;
    ls << QStringLiteral("板子") << QStringLiteral("元件") << QStringLiteral("位置") << QStringLiteral("结果") << QStringLiteral("保留1");
    m_model.setHorizontalHeaderLabels(ls);

    ui.tableView_resultList->setColumnWidth(0, 55);
    ui.tableView_resultList->setColumnWidth(1, 60);
    ui.tableView_resultList->setColumnWidth(2, 60);
    ui.tableView_resultList->setColumnWidth(3, 100);
    ui.tableView_resultList->setColumnWidth(4, 60);
}

void AutoRunPanel::onStateEvent(const QVariantList &data)
{
    int iBoard = data[0].toInt();
    int iEvent = data[1].toInt();
    //if (iEvent != STATION_STATE_RESOULT)return;
    int iValue = data[2].toInt();

    if (STATION_STATE_WAIT_START == iEvent)
    {
        //ui.label_resultStatus->setStyleSheet("background-color: rgb(50, 255, 50);");
        ui.label_resultStatus->setStyleSheet("background-color: rgb(0, 128, 192);");
        ui.label_resultStatus->setText(QStringLiteral("等待测试"));

        ui.progressBar_resultPrg->setValue(0);
    }
    else if (STATION_STATE_CAPTURING == iEvent)
    {
        ui.label_resultStatus->setStyleSheet("background-color: rgb(153, 217, 234);");
        ui.label_resultStatus->setText(QStringLiteral("抓取图像"));

        ui.progressBar_resultPrg->setValue(10);
    }
    else if (STATION_STATE_GENERATE_GRAY == iEvent)
    {
        ui.label_resultStatus->setStyleSheet("background-color: rgb(153, 217, 234);");
        ui.label_resultStatus->setText(QStringLiteral("生成灰度图"));

        ui.progressBar_resultPrg->setValue(20);
    }
    else if (STATION_STATE_CALCULATE_3D == iEvent)
    {
        ui.label_resultStatus->setStyleSheet("background-color: rgb(153, 217, 234);");
        ui.label_resultStatus->setText(QStringLiteral("计算3D"));

        ui.progressBar_resultPrg->setValue(40);
    }
    else if (STATION_STATE_MATCH_POSITION == iEvent)
    {
        ui.label_resultStatus->setStyleSheet("background-color: rgb(153, 217, 234);");
        ui.label_resultStatus->setText(QStringLiteral("匹配元件框"));

        ui.progressBar_resultPrg->setValue(70);
    }
    else if (STATION_STATE_CALCULATE_HEIGHT == iEvent)
    {
        ui.label_resultStatus->setStyleSheet("background-color: rgb(153, 217, 234);");
        ui.label_resultStatus->setText(QStringLiteral("计算高度值"));

        ui.progressBar_resultPrg->setValue(80);
    }
    else if (STATION_STATE_RESOULT == iEvent)
    {
        ui.progressBar_resultPrg->setValue(100);

        if (iValue > 0)
        {
            ui.label_resultOutput->setStyleSheet("color: rgb(50, 255, 50);font: 75 48pt '宋体';");
            ui.label_resultOutput->setText(QStringLiteral("OK"));
        }
        else
        {
            ui.label_resultOutput->setStyleSheet("color: rgb(255, 50, 50);font: 75 48pt '宋体';");
            ui.label_resultOutput->setText(QStringLiteral("FAIL"));
        }
    }
}

void AutoRunPanel::onResultEvent(const QVariantList &data)
{
    int iBoard = data[0].toInt();
    int iEvent = data[1].toInt();
    if (iEvent != STATION_RESULT_DISPLAY)return;
    int nObjIndex = data[2].toInt();
    int nDeteteIndex = data[3].toInt();
    double dHeightValue = data[4].toDouble();

    int nr = m_model.rowCount();
    m_model.insertRow(nr);

    m_model.setData(m_model.index(nr, 0), QStringLiteral("%1").arg(iBoard + 1));
    m_model.setData(m_model.index(nr, 1), QStringLiteral("%1").arg(nObjIndex + 1));
    m_model.setData(m_model.index(nr, 2), QStringLiteral("%1").arg(nDeteteIndex + 1));
    m_model.setData(m_model.index(nr, 3), QStringLiteral("%1mm").arg(dHeightValue));
    m_model.setData(m_model.index(nr, 4), QStringLiteral("--"));

    int nRowCount = 50;
    if (m_model.rowCount() > nRowCount)
    {
        m_model.removeRows(0, m_model.rowCount() - nRowCount);
    }
}

void AutoRunPanel::onStateChangeAutoRun(int iState)
{
    int data = 0;
    if (Qt::Checked == iState)data = 1;

    if ((bool)data)
    {
        ui.pushButton_runOnce->setText(QStringLiteral("自动测试"));
    }
    else
    {
        ui.pushButton_runOnce->setText(QStringLiteral("单步测试"));
    }

    m_bAutoRunning = false;
}

void AutoRunPanel::onRunOnce()
{
    bool isAutoRun = ui.checkBox_AutoRun->isChecked();

    if (isAutoRun)
    {
        if (m_bAutoRunning)
        {
            ui.pushButton_runOnce->setText(QStringLiteral("自动测试"));          
            m_bAutoRunning = false;
        }
        else
        {
            ui.pushButton_runOnce->setText(QStringLiteral("停止测试"));           
            m_bAutoRunning = true;
        }
    }
    else
    {
        IData * pData = getModule<IData>(DATA_MODEL);
        if (pData)
        {
            pData->incrementCycleTests();
        }
    }
}

void AutoRunPanel::onStartAutoRun()
{
    System->userStart();
}

void AutoRunPanel::onStopAutoRun()
{
    System->userStop();
}
