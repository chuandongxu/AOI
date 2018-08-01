#include "WorkSetting.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IMotion.h"
#include "../include/constants.h"

WorkSettingOnLive::WorkSettingOnLive(WorkSetting* pSetting)
    : m_pSetting(pSetting)
{
    m_bQuit = false;
    m_bRuning = false;
}

void WorkSettingOnLive::run()
{
    m_bRuning = true;

    while (!m_bQuit)
    {
        m_pSetting->updateIO();

        if (m_bQuit)break;

        QThread::msleep(200);
    }

    m_bRuning = false;
}

WorkSetting::WorkSetting(WorkCtrl* pCtrl, QWidget *parent)
    : m_pCtrl(pCtrl), QWidget(parent)
{
    ui.setupUi(this);

    m_pThreadOnLive = NULL;

    initUI();
}

WorkSetting::~WorkSetting()
{
}

void WorkSetting::initUI()
{
    connect(ui.pushButton_onLive, SIGNAL(clicked()), SLOT(onAutoLive()));
    connect(ui.pushButton_onStop, SIGNAL(clicked()), SLOT(onAutoStop()));
    ui.pushButton_onLive->setEnabled(true);
    ui.pushButton_onStop->setEnabled(false);

    connect(ui.pushButton_enable, SIGNAL(clicked()), SLOT(onEnable()));
    connect(ui.pushButton_disable, SIGNAL(clicked()), SLOT(onDisable()));
    connect(ui.pushButton_home, SIGNAL(clicked()), SLOT(onHome()));
    connect(ui.pushButton_imStop, SIGNAL(clicked()), SLOT(onImStop()));
    connect(ui.pushButton_clearError, SIGNAL(clicked()), SLOT(onClearError()));

    connect(ui.checkBox_cylinder, SIGNAL(stateChanged(int)), SLOT(onCylinderUpDown(int)));

    connect(ui.pushButton_move, SIGNAL(clicked()), SLOT(onMove()));
    connect(ui.pushButton_stop, SIGNAL(clicked()), SLOT(onStop()));
    connect(ui.pushButton_moveInput, SIGNAL(clicked()), SLOT(onInputMove()));
    connect(ui.pushButton_moveOutput, SIGNAL(clicked()), SLOT(onOutputMove()));
   
    connect(ui.tabWidget, SIGNAL(currentChanged(int)), SLOT(onTabWidgetChanged(int)));

    connect(ui.pushButton_enableWidth, SIGNAL(clicked()), SLOT(onEnableWidth()));
    connect(ui.pushButton_disableWidth, SIGNAL(clicked()), SLOT(onDisableWidth()));
    connect(ui.pushButton_homeWidth, SIGNAL(clicked()), SLOT(onHomeWidth()));
    connect(ui.pushButton_imStopWidth, SIGNAL(clicked()), SLOT(onImStopWidth()));
    connect(ui.pushButton_clearErrorWidth, SIGNAL(clicked()), SLOT(onClearErrorWidth()));

    connect(ui.pushButton_joystickWidth, SIGNAL(clicked()), SLOT(onJoystick()));
    connect(ui.pushButton_moveToReady, SIGNAL(clicked()), SLOT(onMoveToReady()));
    connect(ui.pushButton_moveToWidth, SIGNAL(clicked()), SLOT(onMoveToWidth()));

    ui.pushButton_joystickWidth->setIcon(QIcon("image/joystick.png"));

    auto nTrackWidth = System->getParam("WH_TRACK_WIDTH").toDouble();
    ui.lineEdit_width->setText(QString("%1").arg(nTrackWidth));
    connect(ui.lineEdit_width, SIGNAL(textChanged(const QString &)), SLOT(onTextChangedWidth(const QString &)));
}

void WorkSetting::onAutoLive()
{
    if (!m_pThreadOnLive)
    {
        ui.pushButton_onLive->setEnabled(false);
        ui.pushButton_onStop->setEnabled(true);

        m_pThreadOnLive = new WorkSettingOnLive(this);
        m_pThreadOnLive->start();      
    }
}

void WorkSetting::onAutoStop()
{
    if (m_pThreadOnLive)
    {
        m_pThreadOnLive->setQuitFlag();
        while (m_pThreadOnLive->isRuning())
        {
            QThread::msleep(10);
            QApplication::processEvents();
        }
        QThread::msleep(200);
        m_pThreadOnLive->deleteLater();
        //delete m_pThreadOnLive;
        m_pThreadOnLive = NULL;

        ui.pushButton_onLive->setEnabled(true);
        ui.pushButton_onStop->setEnabled(false);
    }
}

void WorkSetting::onTabWidgetChanged(int index)
{
    bool bStepPre = (0 == index);
    if (bStepPre)
    {
    }
}

void WorkSetting::updateIO()
{
    ui.radioButton_inputSensor->setChecked(m_pCtrl->IsBoardArrived());
    ui.radioButton_stopSensor->setChecked(m_pCtrl->IsBoardStop());
    ui.radioButton_inplaceSensor->setChecked(m_pCtrl->IsBoardReady());
    ui.radioButton_outputSensor->setChecked(m_pCtrl->IsBoardDelivered());
}

void WorkSetting::onEnable()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->enable(AXIS_MOTOR_TRACKING)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("使能进料马达错误."));
        return;
    }
}

void WorkSetting::onDisable()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->disable(AXIS_MOTOR_TRACKING)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("去使能进料马达错误."));
        return;
    }
}

void WorkSetting::onHome()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->home(AXIS_MOTOR_TRACKING, true)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("回零进料马达错误."));
        return;
    }
}

void WorkSetting::onImStop()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->EmStop(AXIS_MOTOR_TRACKING)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("急停进料马达错误."));
        return;
    }
}

void WorkSetting::onClearError()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    pMotion->clearAllError();
}

void WorkSetting::onCylinderUpDown(int iState)
{
    m_pCtrl->upDownCylinder(iState == Qt::Checked);
 }


void WorkSetting::onMove()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->move(AXIS_MOTOR_TRACKING, 0, 99999, false)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("移动进料马达错误."));
        return;
    }
}

void WorkSetting::onStop()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->stopMove(AXIS_MOTOR_TRACKING)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("停止进料马达错误."));
        return;
    }
}

void WorkSetting::onInputMove()
{
    if (!m_pCtrl->moveInput(false)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("上料错误."));
        return;
    }
}

void WorkSetting::onOutputMove()
{
    if (!m_pCtrl->moveOutput(false)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("下料错误."));
        return;
    }
}

void WorkSetting::onEnableWidth()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->enable(AXIS_MOTOR_TRACK_WIDTH)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("使能调宽马达错误."));
        return;
    }
}

void WorkSetting::onDisableWidth()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->disable(AXIS_MOTOR_TRACK_WIDTH)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("去使能调宽马达错误."));
        return;
    }
}

void WorkSetting::onHomeWidth()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->home(AXIS_MOTOR_TRACK_WIDTH, true)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("回零调宽马达错误."));
        return;
    }
}

void WorkSetting::onImStopWidth()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->EmStop(AXIS_MOTOR_TRACK_WIDTH)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("急停调宽马达错误."));
        return;
    }
}

void WorkSetting::onClearErrorWidth()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    pMotion->clearAllError();
}

void WorkSetting::onJoystick()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);

    pMotion->setJoystickXMotor(AXIS_MOTOR_TRACK_WIDTH, 1.2, ui.lineEdit_width);

    pMotion->startJoystick();
}

void WorkSetting::onMoveToReady()
{
    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->moveTo(AXIS_MOTOR_TRACK_WIDTH, 0, 0, false)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("移动进料马达错误."));
        return;
    }
}

void WorkSetting::onMoveToWidth()
{
    auto nTrackWidth = ui.lineEdit_width->text().toDouble();

    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    if (!pMotion->moveTo(AXIS_MOTOR_TRACK_WIDTH, 0, nTrackWidth, false)){
        System->showMessage(QStringLiteral("设置上下料机构"), QStringLiteral("移动进料马达错误."));
        return;
    }
}

void WorkSetting::onTextChangedWidth(const QString &text)
{
    auto nTrackWidth = text.toDouble();
    System->setParam("WH_TRACK_WIDTH", nTrackWidth);
}

