#pragma once

#include <QWidget>
#include <qbuttongroup.h>
#include <memory>

#include "ui_TowerSetting.h"
#include "TowerCtrl.h"

class TowerSetting : public QWidget
{
    Q_OBJECT

public:
    TowerSetting(TowerCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
    ~TowerSetting();

private:
    void initUI();
    void updateUI();
    void loadConfig();
    void saveConfig();

private slots:
    void on_pushButton_LoadAll_clicked();
    void on_pushButton_SaveAll_clicked();

    void on_pushButton_Trigger_clicked();
    void on_pushButton_Stop_clicked();

private:
    Ui::TowerSetting ui;
    TowerCtrl* m_pCtrl;

    std::unique_ptr<QButtonGroup> m_bgIdleRed;
    std::unique_ptr<QButtonGroup> m_bgIdleYellow;
    std::unique_ptr<QButtonGroup> m_bgIdleGreen;
    std::unique_ptr<QButtonGroup> m_bgIdleBuzzer;

    std::unique_ptr<QButtonGroup> m_bgRunRed;
    std::unique_ptr<QButtonGroup> m_bgRunYellow;
    std::unique_ptr<QButtonGroup> m_bgRunGreen;
    std::unique_ptr<QButtonGroup> m_bgRunBuzzer;

    std::unique_ptr<QButtonGroup> m_bgWarnRed;
    std::unique_ptr<QButtonGroup> m_bgWarnYellow;
    std::unique_ptr<QButtonGroup> m_bgWarnGreen;
    std::unique_ptr<QButtonGroup> m_bgWarnBuzzer;

    std::unique_ptr<QButtonGroup> m_bgErrorRed;
    std::unique_ptr<QButtonGroup> m_bgErrorYellow;
    std::unique_ptr<QButtonGroup> m_bgErrorGreen;
    std::unique_ptr<QButtonGroup> m_bgErrorBuzzer;
};
