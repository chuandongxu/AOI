#include "TowerSetting.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IMotion.h"
#include "../include/constants.h"

TowerSetting::TowerSetting(TowerCtrl* pCtrl, QWidget *parent)
    : m_pCtrl(pCtrl), QWidget(parent)
{
    ui.setupUi(this);

    initUI();
    loadConfig();
}

TowerSetting::~TowerSetting()
{
}

void TowerSetting::initUI()
{
    m_bgIdleRed = std::make_unique<QButtonGroup>(this);
    m_bgIdleRed->addButton(ui.radioButton, 0);
    m_bgIdleRed->addButton(ui.radioButton_2, 1);
    m_bgIdleRed->addButton(ui.radioButton_3, 2);
    m_bgIdleYellow = std::make_unique<QButtonGroup>(this);
    m_bgIdleYellow->addButton(ui.radioButton_4, 0);
    m_bgIdleYellow->addButton(ui.radioButton_5, 1);
    m_bgIdleYellow->addButton(ui.radioButton_6, 2);
    m_bgIdleGreen = std::make_unique<QButtonGroup>(this);
    m_bgIdleGreen->addButton(ui.radioButton_7, 0);
    m_bgIdleGreen->addButton(ui.radioButton_8, 1);
    m_bgIdleGreen->addButton(ui.radioButton_8, 2);
    m_bgIdleBuzzer = std::make_unique<QButtonGroup>(this);
    m_bgIdleBuzzer->addButton(ui.radioButton_45, 0);
    m_bgIdleBuzzer->addButton(ui.radioButton_46, 1);

    m_bgRunRed = std::make_unique<QButtonGroup>(this);
    m_bgRunRed->addButton(ui.radioButton_10, 0);
    m_bgRunRed->addButton(ui.radioButton_11, 1);
    m_bgRunRed->addButton(ui.radioButton_12, 2);
    m_bgRunYellow = std::make_unique<QButtonGroup>(this);
    m_bgRunYellow->addButton(ui.radioButton_13, 0);
    m_bgRunYellow->addButton(ui.radioButton_14, 1);
    m_bgRunYellow->addButton(ui.radioButton_15, 2);
    m_bgRunGreen = std::make_unique<QButtonGroup>(this);
    m_bgRunGreen->addButton(ui.radioButton_16, 0);
    m_bgRunGreen->addButton(ui.radioButton_17, 1);
    m_bgRunGreen->addButton(ui.radioButton_18, 2);
    m_bgRunBuzzer = std::make_unique<QButtonGroup>(this);
    m_bgRunBuzzer->addButton(ui.radioButton_47, 0);
    m_bgRunBuzzer->addButton(ui.radioButton_48, 1);

    m_bgWarnRed = std::make_unique<QButtonGroup>(this);
    m_bgWarnRed->addButton(ui.radioButton_19, 0);
    m_bgWarnRed->addButton(ui.radioButton_20, 1);
    m_bgWarnRed->addButton(ui.radioButton_21, 2);
    m_bgWarnYellow = std::make_unique<QButtonGroup>(this);
    m_bgWarnYellow->addButton(ui.radioButton_22, 0);
    m_bgWarnYellow->addButton(ui.radioButton_23, 1);
    m_bgWarnYellow->addButton(ui.radioButton_24, 2);
    m_bgWarnGreen = std::make_unique<QButtonGroup>(this);
    m_bgWarnGreen->addButton(ui.radioButton_25, 0);
    m_bgWarnGreen->addButton(ui.radioButton_26, 1);
    m_bgWarnGreen->addButton(ui.radioButton_27, 2);
    m_bgWarnBuzzer = std::make_unique<QButtonGroup>(this);
    m_bgWarnBuzzer->addButton(ui.radioButton_49, 0);
    m_bgWarnBuzzer->addButton(ui.radioButton_50, 1);

    m_bgErrorRed = std::make_unique<QButtonGroup>(this);
    m_bgErrorRed->addButton(ui.radioButton_28, 0);
    m_bgErrorRed->addButton(ui.radioButton_29, 1);
    m_bgErrorRed->addButton(ui.radioButton_30, 2);
    m_bgErrorYellow = std::make_unique<QButtonGroup>(this);
    m_bgErrorYellow->addButton(ui.radioButton_31, 0);
    m_bgErrorYellow->addButton(ui.radioButton_32, 1);
    m_bgErrorYellow->addButton(ui.radioButton_33, 2);
    m_bgErrorGreen = std::make_unique<QButtonGroup>(this);
    m_bgErrorGreen->addButton(ui.radioButton_34, 0);
    m_bgErrorGreen->addButton(ui.radioButton_35, 1);
    m_bgErrorGreen->addButton(ui.radioButton_36, 2);
    m_bgErrorBuzzer = std::make_unique<QButtonGroup>(this);
    m_bgErrorBuzzer->addButton(ui.radioButton_51, 0);
    m_bgErrorBuzzer->addButton(ui.radioButton_52, 1);

    QStringList ls;
    ls << QStringLiteral("Idle") << QStringLiteral("Running") << QStringLiteral("Warning") << QStringLiteral("Error");
    ui.comboBox_mode->addItems(ls);
}

void TowerSetting::updateUI()
{

}

void TowerSetting::loadConfig()
{
    int nBgRed = System->getParam("tower_light_idle_red").toInt();
    int nBgYellow = System->getParam("tower_light_idle_yellow").toInt();
    int nBgGreen = System->getParam("tower_light_idle_green").toInt();
    int nBgBuzzer = System->getParam("tower_light_idle_buzzer").toInt();
    m_bgIdleRed->button(nBgRed)->setChecked(true);
    m_bgIdleYellow->button(nBgYellow)->setChecked(true);
    m_bgIdleGreen->button(nBgGreen)->setChecked(true);
    m_bgIdleBuzzer->button(nBgBuzzer)->setChecked(true);

    nBgRed = System->getParam("tower_light_run_red").toInt();
    nBgYellow = System->getParam("tower_light_run_yellow").toInt();
    nBgGreen = System->getParam("tower_light_run_green").toInt();
    nBgBuzzer = System->getParam("tower_light_run_buzzer").toInt();
    m_bgRunRed->button(nBgRed)->setChecked(true);
    m_bgRunYellow->button(nBgYellow)->setChecked(true);
    m_bgRunGreen->button(nBgGreen)->setChecked(true);
    m_bgRunBuzzer->button(nBgBuzzer)->setChecked(true);

    nBgRed = System->getParam("tower_light_warn_red").toInt();
    nBgYellow = System->getParam("tower_light_warn_yellow").toInt();
    nBgGreen = System->getParam("tower_light_warn_green").toInt();
    nBgBuzzer = System->getParam("tower_light_warn_buzzer").toInt();
    m_bgWarnRed->button(nBgRed)->setChecked(true);
    m_bgWarnYellow->button(nBgYellow)->setChecked(true);
    m_bgWarnGreen->button(nBgGreen)->setChecked(true);
    m_bgWarnBuzzer->button(nBgBuzzer)->setChecked(true);

    nBgRed = System->getParam("tower_light_error_red").toInt();
    nBgYellow = System->getParam("tower_light_error_yellow").toInt();
    nBgGreen = System->getParam("tower_light_error_green").toInt();
    nBgBuzzer = System->getParam("tower_light_error_buzzer").toInt();
    m_bgErrorRed->button(nBgRed)->setChecked(true);
    m_bgErrorYellow->button(nBgYellow)->setChecked(true);
    m_bgErrorGreen->button(nBgGreen)->setChecked(true);
    m_bgErrorBuzzer->button(nBgBuzzer)->setChecked(true);
}

void TowerSetting::saveConfig()
{
    System->setParam("tower_light_idle_red", m_bgIdleRed->checkedId());
    System->setParam("tower_light_idle_yellow", m_bgIdleYellow->checkedId());
    System->setParam("tower_light_idle_green", m_bgIdleGreen->checkedId());
    System->setParam("tower_light_idle_buzzer", m_bgIdleBuzzer->checkedId());

    System->setParam("tower_light_run_red", m_bgRunRed->checkedId());
    System->setParam("tower_light_run_yellow", m_bgRunYellow->checkedId());
    System->setParam("tower_light_run_green", m_bgRunGreen->checkedId());
    System->setParam("tower_light_run_buzzer", m_bgRunBuzzer->checkedId());

    System->setParam("tower_light_warn_red", m_bgWarnRed->checkedId());
    System->setParam("tower_light_warn_yellow", m_bgWarnYellow->checkedId());
    System->setParam("tower_light_warn_green", m_bgWarnGreen->checkedId());
    System->setParam("tower_light_warn_buzzer", m_bgWarnBuzzer->checkedId());

    System->setParam("tower_light_error_red", m_bgErrorRed->checkedId());
    System->setParam("tower_light_error_yellow", m_bgErrorYellow->checkedId());
    System->setParam("tower_light_error_green", m_bgErrorGreen->checkedId());
    System->setParam("tower_light_error_buzzer", m_bgErrorBuzzer->checkedId());
}

void TowerSetting::on_pushButton_LoadAll_clicked()
{
    loadConfig();
    updateUI();
}

void TowerSetting::on_pushButton_SaveAll_clicked()
{
    saveConfig();
}


void TowerSetting::on_pushButton_Trigger_clicked()
{
    int nMode = ui.comboBox_mode->currentIndex();
    m_pCtrl->setLightMode(static_cast<LightMode>(nMode));
}

void TowerSetting::on_pushButton_Stop_clicked()
{
    m_pCtrl->stopLight();
}