#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QSmartDetect.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include "DataCtrl.h"
#include "settingdialog.h"
#include "QDetectRunnable.h"

class QSmartDetect : public QMainWindow
{
	typedef QList<QCheckerParamMap*> QCheckerParamMapList;
	typedef QList<QCheckerRunable*> QCheckerRunableList;
    Q_OBJECT

public:
    QSmartDetect(QWidget *parent = Q_NULLPTR);
	~QSmartDetect();

private slots:
	void onTabEvent(const QVariantList &data);

	void onSetting();
	void about();
	void aboutSystem();

private:
	void init();
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();


	QMenu *settingMenu;
	QMenu *helpMenu;

	QAction *settingAct;
	QAction *aboutAct;
	QAction *aboutSystemAct;

protected:
	void timerEvent(QTimerEvent *event);
	void checkAuthError();
	void checkError();
	void showErrorMessage();

private:
	void initStationParam();
	void initErrorCode();
	bool start();
	void stop();
	void imStop();

private:
    Ui::QSmartDetectClass ui;
	QHBoxLayout * m_subHLayout;
	QVBoxLayout * m_subVLayout;
	int m_nTimerID;
	QDateTime m_dateTime;
	DataCtrl m_ctrl;
	QWidget* m_pBottomWidget;
	QWidget* m_pRightWidget;
	QWidget* m_pRightRunWidget;
	QWidget* m_pRightSetWidget;
	QSettingDialog m_dlgSetting;

private:
	QCheckerRunableList m_checkStantion;
	QCheckerParamMapList m_stationParams;
	QCheckerParamDataList m_stationDatas;
	QCheckerParamMap *m_pMainParamMap;
	AutoRunThread* m_pMainStation;
	int m_errorCode;
};
