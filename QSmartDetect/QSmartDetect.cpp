#include "QSmartDetect.h"
#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IData.h"
#include "../include/IVision.h"
#include "../include/ICamera.h"
#include "../include/IDlp.h"
#include "../Common/eos.h"

#include "../HSGWorkflowCtrl/hsgworkflowctrl_global.h"
#include "QBottomWidget.h"
#include "QRightWidget.h"
#include "QStatisticsWidget.h"
#include "QRunSettingWidget.h"

#include <QMessageBox>
#include<QTime>
#include <QThreadPool>

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

QSmartDetect::QSmartDetect(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	init();

	QEos::Attach(EVENT_TAB_STATE, this, SLOT(onTabEvent(const QVariantList &)));

	m_pRightRunWidget = new QStatisticsWidget(&m_ctrl);
	m_pRightSetWidget = new QRunSettingWidget(&m_ctrl);
	m_pRightRunWidget->setVisible(false);
	m_pRightSetWidget->setVisible(true);

	m_pRightWidget = new QRightWidget(&m_ctrl);
	((QRightWidget*)m_pRightWidget)->setWidget(m_pRightRunWidget, m_pRightSetWidget);

	m_pBottomWidget = new QBottomWidget();

	//IVision * pUI = getModule<IVision>(VISION_MODEL);
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);

	m_subHLayout = new QHBoxLayout;
	
	m_subVLayout = new QVBoxLayout;
	if (pCam)
	{
		QWidget* pWidget = pCam->getMainView();
		pWidget->setFixedSize(800, 670);		
		m_subVLayout->addWidget(pWidget);
	}
	m_subVLayout->addWidget(m_pBottomWidget);
	m_subHLayout->addLayout(m_subVLayout);
	m_subHLayout->addWidget(m_pRightWidget);
	m_subHLayout->setContentsMargins(10, 0, 10, 0);
	m_subHLayout->setAlignment(Qt::AlignTop);
	
	ui.verticalLayout->addLayout(m_subHLayout);

	m_nTimerID = this->startTimer(500);
	m_dateTime = QDateTime::currentDateTime();

	System->setParam("sys_run_dlp_mode", 0);// USB trigger DLP	
	System->setParam("motion_trigger_pattern_num", 12);
	m_pMainParamMap = NULL;
	m_pMainStation = NULL;
	m_errorCode = -1;

	initStationParam();
	initErrorCode();

	QString user;
	int level = 0;
	System->getUser(user, level);
	if (USER_LEVEL_MANAGER > level)
	{
		if (settingMenu) settingMenu->setEnabled(false);
	}
}

QSmartDetect::~QSmartDetect()
{	
	this->killTimer(m_nTimerID);

	QCheckerParamMapList::iterator it = m_stationParams.begin();
	for (; it != m_stationParams.end(); ++it)delete *it;

	m_stationParams.clear();
	m_stationDatas.clear();

	if (m_pMainParamMap)
	{
		delete m_pMainParamMap;
		m_pMainParamMap = NULL;
	}
}

void QSmartDetect::initStationParam()
{
	//第一个检测器
	QCheckerParamMap *paramMap = new QCheckerParamMap;
	paramMap->insert(PARAM_STATION_ID, 1);
	/*paramMap->insert(PARAM_STATION_START_IO, DI_START_STATION1);
	paramMap->insert(PARAM_STATION_IN_AXIS,AXIS_CHECK1_IN);
	paramMap->insert(PARAM_STATION_TOP_AXIS, AXIS_CHECK1_TOP);
	paramMap->insert(PARAM_STATION_CHECK_POS, 1);
	paramMap->insert(PARAM_STATION_BACK_POS, 0);
	paramMap->insert(PARAM_STATION_CHECK_TYPE, CHECK_TYPE_ATEQ);
	paramMap->insert(PARAM_STATION_OK_LIGHT, DO_CHECK_OK1);
	paramMap->insert(PARAM_STATION_NG_LIGHT, DO_CHECK_NG1);
	paramMap->insert(PARAM_STATION_AI, CHECK_AI_1);*/
	m_stationParams.append(paramMap);
	m_stationDatas.append(Q3DStructData());

	paramMap = new QCheckerParamMap;
	paramMap->insert(PARAM_STATION_ID, 2);
	/*paramMap->insert(PARAM_STATION_START_IO, DI_START_STATION1);
	paramMap->insert(PARAM_STATION_IN_AXIS,AXIS_CHECK1_IN);
	paramMap->insert(PARAM_STATION_TOP_AXIS, AXIS_CHECK1_TOP);
	paramMap->insert(PARAM_STATION_CHECK_POS, 1);
	paramMap->insert(PARAM_STATION_BACK_POS, 0);
	paramMap->insert(PARAM_STATION_CHECK_TYPE, CHECK_TYPE_ATEQ);
	paramMap->insert(PARAM_STATION_OK_LIGHT, DO_CHECK_OK1);
	paramMap->insert(PARAM_STATION_NG_LIGHT, DO_CHECK_NG1);
	paramMap->insert(PARAM_STATION_AI, CHECK_AI_1);*/
	m_stationParams.append(paramMap);
	m_stationDatas.append(Q3DStructData());

	m_pMainParamMap = new QCheckerParamMap;
}

void QSmartDetect::initErrorCode()
{
	System->addErrorMap(ERROR_ATHU_NORIGHT_WARRING, MSG_ATH_NORIGHT_WARRING);
	System->addErrorMap(ERROR_SAFE_DOOR, MSG_SAFE_DOOR);
	System->addErrorMap(ERROR_MOTOR_ALM, MSG_MOTOR_ALM);
	System->addErrorMap(ERROR_ZHOME_ALM, MSG_ZHOME_ALM);
	System->addErrorMap(ERROR_XHOME_ALM, MSG_XHOME_ALM);
	System->addErrorMap(ERROR_SHOME_ALM, MSG_SHOME_ALM);
	System->addErrorMap(ERROR_HOME_MOTION_ALM, MSG_HOME_NOMOTION_ALM);
	System->addErrorMap(ERROR_MOTION_POS_WARRING, MSG_MOTION_POS_WARRING);
	System->addErrorMap(ERROR_STATION_SAFE_GRATING_ALRM, MSG_STTATION_SAFE_GRATING_ALRM);
}

void QSmartDetect::init()
{
	//setAttribute(Qt::WA_DeleteOnClose);

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	setUnifiedTitleAndToolBarOnMac(true);
}

void QSmartDetect::createActions()
{
	settingAct = new QAction(QIcon("image/setting.png"), QStringLiteral("系统设置"), this);
	settingAct->setShortcuts(QKeySequence::HelpContents);
	settingAct->setStatusTip(tr("System Setting"));
	connect(settingAct, SIGNAL(triggered()), this, SLOT(onSetting()));

	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutSystemAct = new QAction(tr("About System"), this);
	aboutSystemAct->setStatusTip(tr("Show the System About box"));
	connect(aboutSystemAct, SIGNAL(triggered()), this, SLOT(aboutSystem()));

}

void QSmartDetect::createMenus()
{
	ui.mainToolBar->setVisible(false);

	settingMenu = menuBar()->addMenu(QStringLiteral("设置"));
	settingMenu->addAction(settingAct);

	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu(QStringLiteral("帮助"));
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(aboutSystemAct);
}


void QSmartDetect::createToolBars()
{
	//! [0]
	//ui.mainToolBar->addAction(cameraAct);

	//configToolBar = addToolBar(tr("Config"));
	//configToolBar->addAction(openConfigAct);
	//configToolBar->addAction(saveAsConfigAct);
}

void QSmartDetect::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
	//statusBar()->setSizeGripEnabled(false);
}

void QSmartDetect::about()
{
	QMessageBox::about(this, tr("About Editor"),
		tr("The <b>Editor</b> example demonstrates how to edit or "
		"display 3D applications using system."));
}

void QSmartDetect::aboutSystem()
{
	QMessageBox::about(this, tr("About Editor"),
		tr("The <b>Editor</b> example demonstrates how to display "
		"3D applications using system."));
}

void QSmartDetect::onSetting()
{
	m_dlgSetting.show();
}

void QSmartDetect::onTabEvent(const QVariantList &data)
{
	int iBoard = data[0].toInt();
	int iEvent = data[1].toInt();
	//if (iEvent != STATION_STATE_RESOULT)return;
	int iValue = data[2].toInt();

	if (RUN_TAB_SETTING == iEvent)
	{
		m_pRightRunWidget->setVisible(false);
		m_pRightSetWidget->setVisible(true);

		QRunTabWidget* pWidget = m_ctrl.getWidget();
		if (pWidget)
		{
			pWidget->showBtn(true);
		}

		stop();

		settingAct->setEnabled(true);
	}
	else if (RUN_TAB_RUNING == iEvent)
	{
		//QEos::Notify(EVENT_RESULT_DISPLAY, 0, STATION_RESULT_DISPLAY_CLEAR);
		if (start())
		{
			m_pRightRunWidget->setVisible(true);
			m_pRightSetWidget->setVisible(false);

			QRunTabWidget* pWidget = m_ctrl.getWidget();
			if (pWidget)
			{
				pWidget->showBtn(false);
			}

			settingAct->setEnabled(false);
		}
	}
}

void QSmartDetect::timerEvent(QTimerEvent *event)
{
	static int count = 0;

	if (count > 10*2)
	{
		checkError();
		checkAuthError();
		showErrorMessage();
	}
	else count++;
}

void QSmartDetect::checkAuthError()
{
	if (m_dateTime.secsTo(QDateTime::currentDateTime()) >= 60 * 20)		
	{
		if (!System->checkRuntimeAuthRight())
		{
			System->setErrorCode(ERROR_ATHU_NORIGHT_WARRING);
			return;
		}
		m_dateTime = QDateTime::currentDateTime();
	}
}


void QSmartDetect::checkError()
{
	unsigned int code = System->getErrorCode();
	if (m_errorCode == code) return;
	if (code < 0x30000000)
	{
		if (!System->isImStop())
		{
			this->imStop();
			m_errorCode = code;
		}
	}
}
void QSmartDetect::showErrorMessage()
{
	int code = System->getErrorCode();
	if (ERROR_NO_ERROR != code)
	{
		QString msg = System->getErrorString();

		QString str;
		if (msg.isEmpty())str = QString(QStringLiteral("未知确切含义错误 %0")).arg(code);
		else str = QString(QStringLiteral(" %0")).arg(msg);
		if (code < 0x30000000)
		{
			QSystem::showMessage(QStringLiteral("错误"), str, 0);
		}
		else if (code < 0x40000000)
		{
			QSystem::showMessage(QStringLiteral("告警"), str);
		}
	}
}

bool QSmartDetect::start()
{	
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return false;

	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return false;

	if (m_ctrl.isRunning())
	{
		QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在运行，请先按停止按钮"));
		return false;
	}

	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在启动中..."), 0);
	QApplication::processEvents();

	if (pCam->getCameraNum() > 0)
	{
		if (!pCam->startUpCapture())
		{
			QSystem::closeMessage();
			QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("相机初始化问题。"));
			return false;
		}
	}
	else
	{
		QSystem::closeMessage();
		QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("请检查相机是否连接。"));
		return false;
	}

	int nCaptureMode = System->getParam("camera_capture_mode").toInt();
	bool isImageFloder = (2 == nCaptureMode);

	//int nStationNum = STATION_COUNT;
	int nStationNum = pData->getCoreData(0);

	m_checkStantion.clear();
	for (int i = 0; i < nStationNum; i++)
	{
		if (isImageFloder || pDlp->isConnected(i))
		{
			if (!isImageFloder)
			{
				if (!pDlp->startUpCapture(i)) continue;
			}			

			m_stationDatas[i].clear();
			QCheckerRunable * p = new QCheckerRunable(m_stationParams[i], &(m_stationDatas[i]));
			m_checkStantion.append(p);
			QThreadPool::globalInstance()->start(p, QThread::TimeCriticalPriority);
		}
		else
		{
			System->setTrackInfo(QString(QStringLiteral("工位%0启动失败, 请检查DLP硬件！")).arg(i + 1));
		}
	}

	m_pMainStation = new QMainRunable(m_pMainParamMap, &m_stationDatas, &m_ctrl);
	QThreadPool::globalInstance()->start(m_pMainStation);

	m_ctrl.run(true);

	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在导入数据中..."), 0);
	QApplication::processEvents();

	bool bStationStarted = true;
	int nWaitTime = 30 * 5;
	do 
	{
		bStationStarted = true;
		QCheckerRunableList::iterator it = m_checkStantion.begin();
		for (; it != m_checkStantion.end(); ++it)
		{
			QCheckerRunable * p = *it;
			if (p && !p->isRunning())
			{
				bStationStarted = false;
			}
		}
		QThread::msleep(200);
	} while (!bStationStarted && nWaitTime-- > 0);

	if (nWaitTime <= 0)
	{
		System->setTrackInfo(QStringLiteral("等待工位启动失败！"));
		QSystem::closeMessage();
		return true;
	}

	//QEos::Notify(EVENT_RUN_STATE, RUN_STATE_RUNING);

	QSystem::closeMessage();

	return true;
}

void QSmartDetect::stop()
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return;

	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return;

	if (!m_ctrl.isRunning()) return;

	//m_isHome = false;

	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在停止中..."), 0);
	QApplication::processEvents();

	QCheckerRunableList::iterator it = m_checkStantion.begin();
	for (; it != m_checkStantion.end(); ++it)
	{
		QCheckerRunable * p = *it;
		if (p) p->quit();
	}
	if (m_pMainStation) m_pMainStation->quit();

	//if (QThreadPool::globalInstance()->activeThreadCount())
	QThreadPool::globalInstance()->waitForDone();

	//it = m_checkStantion.begin();
	//for (; it != m_checkStantion.end(); ++it)
	//{
	//	delete *it;
	//}
	m_checkStantion.clear();
	//if (m_pMainParamMap)
	//{
	//	delete m_pMainParamMap;
	//	m_pMainParamMap = NULL;
	//}

	if (pCam->getCameraNum() > 0)
	{
		pCam->endUpCapture();
	}

	for (int i = 0; i < STATION_COUNT; i++)
	{
		if (pDlp->isConnected(i))
		{
			if (!pDlp->endUpCapture(i)) continue;
		}
	}

	m_ctrl.run(false);
	//QEos::Notify(EVENT_RUN_STATE, RUN_STATE_STOP);
	QSystem::closeMessage();
}

void QSmartDetect::imStop()
{	
	QCheckerRunableList::iterator it = m_checkStantion.begin();
	for (; it != m_checkStantion.end(); ++it)
	{
		QCheckerRunable * p = *it;
		if (p)p->imgStop();
	}
	this->stop();
	System->userImStop();	

	QSystem::showMessage(QStringLiteral("告警"), QStringLiteral("设备处于急停状态。"));
}
