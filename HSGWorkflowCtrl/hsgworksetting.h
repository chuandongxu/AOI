#ifndef WORKSETTING_H
#define WORKSETTING_H

#include <QWidget>
#include "hsgflowctrl.h"
#include "ui_worksetting.h"


class QWorkSetting : public QWidget
{
    Q_OBJECT

public:
    QWorkSetting(QFlowCtrl *p, QWidget *parent = 0);
    ~QWorkSetting();

private:
    void initUI();

protected slots:    
    void onClickFullSpeed(bool s);
    void onClickHightSpeed(bool s);
    void onClickMidSpeed(bool s);
    void onclickLowSpeed(bool s);

    void onComBoxChange1(int iState);
    void onComBoxChange2(int iState);
    void onComBoxChange4(int iState);
    void onSelRecord();
    void onSaveRecord();
    void onSelRecordDetail();
    void onSaveRecordDetail();
    void onSelBackup();
    void onSaveBackup();

    void onTypeIndexChanged(int index);
    void onStationIndexChanged(int index);
    void onSaveHsgType();

    void onEnableSafeDoor(int iState);
    void onCheckSafeGrating(int iState);

    void onDlpModeIndexChanged(int index);
    void onDlpModeSave();

    void onTriggerNumIndexChanged(int iIndex);
    void onTriggerSave();

    // vision log
    void onLogAllCase(int iState);
    void onClickLogFailCase(bool s);
    void onClickLogAllCase(bool s);
    void onInitPRSystem();
    void onUninitPRSystem();
    void onClearAllRecords();
    void onDumpVisionTimeLog();
    void onAutoClearRecord(int iState);

    void onCheckStartUpHome(int iState);
    void onCheckStartZReady(int iState);
    void onCheckStartLoadDB(int iState);
    void onCheckStartDLPInit(int iState);
    void onSaveZReady();
    void onSelectDBPath();
    void onSaveDBPath();

    void onSelectOffLnPath();
    void onSaveOffLnPath();

private:
    Ui::QWorkSetting ui;
    QFlowCtrl * m_ctrl;

    QValidator* m_pValidatorDouble;
};

#endif // WORKSETTING_H
