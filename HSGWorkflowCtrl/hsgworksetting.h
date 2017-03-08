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
	void onEnableAutoCycle(int iState);
	void onEnableCheckBarcode(int iState);
	void onCheckSafeGrating(int iState);

private:
	Ui::QWorkSetting ui;
	QFlowCtrl * m_ctrl;

	QValidator* m_pValidatorDouble;
};

#endif // WORKSETTING_H
