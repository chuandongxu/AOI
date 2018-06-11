#include <time.h>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QThreadPool>
#include <QFileDialog>
#include <QThread>
#include <QTime>

#include "hsgflowctrl.h"
#include "../Common/ModuleMgr.h"
#include "../Common/SystemData.h"
#include "../include/IMotion.h"
#include "../include/IData.h"
#include "../include/IDlp.h"
#include "../include/ICamera.h"
#include "../include/IVision.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/eos.h"
#include "../Common/CVSFile.h"

#include "../include/IFlowCtrl.h"
#include "../Common/ThreadPrioc.h"
#include "CryptLib.h"
#include "opencv2/highgui.hpp"
#include "../DataModule/DataUtils.h"


#define POS_ZHOME   "zHome%0"
#define PROFILE_X   "xMoveProfile%0"
const QString TimeFormatString ="MMdd";

QFlowCtrl::QFlowCtrl(QObject *parent)
	: QObject(parent),m_isHome(false),m_isStart(false),m_timerId(-1)
	,m_homeIng(false)
{	
	m_errorCode = -1;
	m_timerId = startTimer(50);

	m_pAutoRunThread = NULL;

	this->initStationParam();

	QSystem *p = QSystem::instance();
	bool s = connect(p, SIGNAL(goHome()), this, SLOT(home()));
	s = connect(p, SIGNAL(start()), this, SLOT(startAutoRun()));
	s = connect(p, SIGNAL(stop()), this, SLOT(stopAutoRun()));

	m_dateTime = QDateTime::currentDateTime();

	QEos::Attach(EVENT_IMAGE_STATE, this, SLOT(onImageEvent(const QVariantList &)));
    QEos::Attach(EVENT_THREAD_STATE, this, SLOT(onThreadState(const QVariantList &)));

	//IMotion * pMotion = getModule<IMotion>(MOTION_MODEL);
	//if(pMotion)
	//{
	//	pMotion->setExtDO(DO_YELLOW_LIGHT,1);
	//	pMotion->setExtDO(DO_GREEN_LIGHT,0);
	//	pMotion->setExtDO(DO_RED_LIGHT,0);
	//	pMotion->setExtDO(DO_BUZZER,0);
	//}	
}

QFlowCtrl::~QFlowCtrl()
{
}

bool QFlowCtrl::isRuning()
{
	return m_isStart;
}

void QFlowCtrl::onImageEvent(const QVariantList &data)
{
	if (data.size() < 3) return;

	int iBoard = data[0].toInt();
	int iEvent = data[1].toInt();
	if (iEvent != IMAGE_STATE_CHANGE) return;
	int nIndex = data[2].toInt();
}

void QFlowCtrl::onThreadState(const QVariantList &data)
{
    if (data.size() <= 0) return;

    int iEvent = data[0].toInt();

    switch (iEvent)
    {
    case THREAD_CLOSED:
        stop();
        break;

    case REFRESH_BIG_IMAGE:
        _refreshDisplayImage();
        break;

    case AUTO_RUN_WITH_ERROR:
        _onAutoRunError();
        break;

    default:
        break;
    }
}

void QFlowCtrl::home()
{	
	if(m_isStart)
	{
		QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在运行中，请先停止在回零"));
		QApplication::processEvents();

		//this->stop();
		return ;
	}

    QEos::Notify(EVENT_GOHOMEING_STATE, GOHOMEING_STATE_OK);

    //急停判断
    IMotion * p = getModule<IMotion>(MOTION_MODEL);
    if (p) {
        int iState = 1;
        //p->getExtDI(DI_IM_STOP, iState);
        if (0 == iState) {
            QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备处于急停状态，请先排除急停。"));
            return;
        }
    }
    else
        System->setTrackInfo(QStringLiteral("系统暂无控制系统，无法回零。"));

    QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在回零中..."), 0);
    QApplication::processEvents();

    if (p->homeAll(true)) {
        m_isHome = true;
        QEos::Notify(EVENT_GOHOME_STATE, GOHOME_STATE_OK);
        QSystem::closeMessage();
    }
    else {
        m_isHome = false;
        QEos::Notify(EVENT_GOHOME_STATE, GOHOME_STATE_NG);
        System->setErrorCode(ERROR_HOME_MOTION_ALM);
        QSystem::closeMessage();
    }
}

void QFlowCtrl::startAutoRun()
{
	start();
}

void QFlowCtrl::stopAutoRun()
{
	if (m_isStart) stop();
}

void QFlowCtrl::readbarCode()
{	
}

void QFlowCtrl::imStop()
{
	this->stop();
	System->userImStop();

	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		//p->setExtDO(DO_YELLOW_LIGHT,0);
		//p->setExtDO(DO_GREEN_LIGHT,0);
		//p->setExtDO(DO_RED_LIGHT,1);
		//p->setExtDO(DO_BUZZER,1);
	}

	QSystem::showMessage(QStringLiteral("告警"), QStringLiteral("设备处于急停状态。"));
}
	
void QFlowCtrl::reset()
{
	if(m_isStart)
	{
		QMessageBox::warning(NULL,QStringLiteral("警告"), QStringLiteral("复位前请停止设备。"));
		return;
	}
	m_errorCode =-1;
	m_isHome = false;
	m_isStart = false;
	System->setErrorCode(ERROR_NO_ERROR);
	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		//p->clearError();
		QThread::msleep(100);
		//p->setExtDO(DO_YELLOW_LIGHT,1);
		//p->setExtDO(DO_GREEN_LIGHT,0);
		//p->setExtDO(DO_RED_LIGHT,0);
		//p->setExtDO(DO_BUZZER,0);
	}	
	
	if(QSystem::isMessageShowed())
	{
		QSystem::closeMessage();
	}	
}
	
void QFlowCtrl::start()
{
    ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
    if (!pCam) return;

    IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
    if (!pDlp) return;

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    if (!pUI) return;

    if (_prepareRunData() != OK)
        return;

    if (!m_isHome) {
        QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("请先将设备回零"));
        return;
    }

    if (m_isStart) {
        QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在运行，请先按停止按钮"));
        return;
    }

    QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在启动中..."), 0);
    QApplication::processEvents();

    if (! System->isRunOffline()) {
        pCam->selectCaptureMode(ICamera::TRIGGER_ALL);
        if (pCam->getCameraNum() > 0) {
            if (!pUI->startUpCapture(false)) {
                QSystem::closeMessage();
                QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("相机初始化问题。"));
                return;
            }
        }
        else {
            QSystem::closeMessage();
            QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("请检查相机是否连接。"));
            return;
        }

        int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
        for (int i = 0; i < nStationNum; ++ i) {
            if (pDlp->isConnected(i)) {
                if (! pDlp->startUpCapture(i))
                    continue;
            }
            else
                System->setTrackInfo(QString(QStringLiteral("工位%0启动失败, 请检查DLP硬件！")).arg(i + 1));
        }
    }

    auto enScanDir = static_cast<Vision::PR_SCAN_IMAGE_DIR>(System->getParam("scan_image_Direction").toInt());

    AutoRunParams stAutoRunParams(m_nImgWidth, m_nImgHeight, m_fBoardLeft, m_fBoardTop, m_fBoardRight, m_fBoardBottom, m_fOverlapX, m_fOverlapY, enScanDir);
    m_pAutoRunThread = new AutoRunThread(m_vecAlignments, m_vecDeviceInspWindow, m_vecVecFrameCtr, stAutoRunParams, &m_mapBoardInspResult);
    connect(m_pAutoRunThread, &AutoRunThread::finished, m_pAutoRunThread, &QObject::deleteLater);
    m_pAutoRunThread->start();

    m_isStart = true;

    QEos::Notify(EVENT_RUN_STATE, RUN_STATE_RUNING);

    System->setParam("camera_show_image_toScreen_enable", false);

    QSystem::closeMessage();
}
	
void QFlowCtrl::stop()
{		
	//m_isHome = false;

	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在停止中..."), 0);
	QApplication::processEvents();

	//if (m_pAutoRunThread) m_pAutoRunThread->quit();
    QEos::Notify(EVENT_THREAD_STATE, SHUTDOWN_MAIN_THREAD);

	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		//p->setExtDO(DO_YELLOW_LIGHT,1);
		//p->setExtDO(DO_GREEN_LIGHT,0);
		//p->setExtDO(DO_RED_LIGHT,0);
		//p->setExtDO(DO_BUZZER,0);

		//p->releaseInputLock();
		//p->releaseAllStationStart();
	}

	System->setParam("camera_show_image_toScreen_enable", true);

	m_isStart = false;
	QEos::Notify(EVENT_RUN_STATE, RUN_STATE_STOP);
	QSystem::closeMessage();
}

void QFlowCtrl::initStationParam()
{
}

void QFlowCtrl::checkAuthError()
{
	if (m_dateTime.secsTo(QDateTime::currentDateTime()) >= 60*10)
	//if (m_dateTime.daysTo(QDateTime::currentDateTime()) >= 1)
	{
		if (!System->checkRuntimeAuthRight())
		{
			System->setErrorCode(ERROR_ATHU_NORIGHT_WARRING);
			return;
		}
		m_dateTime = QDateTime::currentDateTime();
	}	
}

//检测电机驱动器状态，报警，急停，使能等。
void QFlowCtrl::checkMotionState()
{
}

void QFlowCtrl::checkError()
{
}

void QFlowCtrl::initErrorCode()
{
	System->addErrorMap(ERROR_ATHU_NORIGHT_WARRING, MSG_ATH_NORIGHT_WARRING);
	//System->addErrorMap(ERROR_Z_POS_WARRING,MSG_Z_POS_WARRING);
	//System->addErrorMap(ERROR_Z_POS_WARRING,MSG_Z_POS_WARRING);
	System->addErrorMap(ERROR_MOTION_POS_WARRING,MSG_MOTION_POS_WARRING);
	System->addErrorMap(ERROR_STATION_SAFE_GRATING_ALRM,MSG_STTATION_SAFE_GRATING_ALRM);
}

int QFlowCtrl::_prepareRunData()
{
    int nResult = Engine::GetBigBoardCoords(m_fBoardLeft, m_fBoardTop, m_fBoardRight, m_fBoardBottom);
    if (Engine::OK != nResult) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString msg(QStringLiteral("读取电路板坐标失败, 错误消息: "));
        msg + errorMessage.c_str();
        System->showMessage(QStringLiteral("准备自动运行"), msg);
        return NOK;
    }

    
    nResult = Engine::GetAllAlignments(m_vecAlignments);
    if (Engine::OK != nResult) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString msg(QStringLiteral("读取定位标记信息失败, 错误消息: "));
        msg + errorMessage.c_str();
        System->showMessage(QStringLiteral("准备自动运行"), msg);
        return NOK;
    }

    if (m_vecAlignments.size() <= 1) {
        System->showMessage(QStringLiteral("Prepare auto run"), QStringLiteral("Please set at least 2 alignment point!"));
        return NOK;
    }

    for (auto &alignment : m_vecAlignments) {
        alignment.tmplPosX += m_fBoardLeft;
        alignment.tmplPosY += m_fBoardBottom;
    }

    auto fResolutionX = System->getSysParam("CAM_RESOLUTION_X").toFloat();
    auto fResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toFloat();    

    auto pCam = getModule<ICamera>(CAMERA_MODEL);
    pCam->getCameraScreenSize(m_nImgWidth, m_nImgHeight);
    float fovWidth = m_nImgWidth * fResolutionX, fovHeight = m_nImgHeight * fResolutionY;  // 2032 is the camera resolution, later need to get from elsewhere.
    
    nResult = DataUtils::assignFrames(m_fBoardLeft, m_fBoardTop, m_fBoardRight, m_fBoardBottom, fovWidth, fovHeight, m_vecVecFrameCtr, m_fOverlapX, m_fOverlapY);
    if (nResult != OK) {
        System->showMessage(QStringLiteral("分配Frame"), QStringLiteral("分配Frame失败!"));
        return NOK;
    }   

    Engine::DeviceVector vecDevice;
    nResult = Engine::GetAllDevices(vecDevice);
    if (Engine::OK != nResult) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString msg(QStringLiteral("读取元件信息失败, 错误消息: "));
        msg + errorMessage.c_str();
        System->showMessage(QStringLiteral("准备自动运行"), msg);
        return NOK;
    }

    m_vecDeviceInspWindow.clear();
    for (const auto &device: vecDevice) {
        Engine::WindowVector vecWindows;
        nResult = Engine::GetDeviceWindows(device.Id, vecWindows);
        if (Engine::OK != nResult) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString msg(QStringLiteral("读取元件检测框失败, 错误消息: "));
            msg + errorMessage.c_str();
            System->showMessage(QStringLiteral("准备自动运行"), msg);
            return NOK;
        }

        if (vecWindows.empty())
            continue;

        DeviceInspWindow deviceInpWindow;
        deviceInpWindow.bInspected = false;
        deviceInpWindow.device = device;
        nResult = Engine::GetDeviceUngroupedWindows(device.Id, deviceInpWindow.vecUngroupedWindows);
        if (Engine::OK != nResult) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString msg(QStringLiteral("读取元件未分组检测框失败, 错误消息: "));
            msg + errorMessage.c_str();
            System->showMessage(QStringLiteral("准备自动运行"), msg);
            return NOK;
        }

        for (auto &window : deviceInpWindow.vecUngroupedWindows) {
            window.x += m_fBoardLeft;
            window.y += m_fBoardBottom;
        }

        Int64Vector vecGroupId;
        nResult = Engine::GetDeviceWindowGroups(device.Id, vecGroupId);
        if (Engine::OK != nResult) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString msg(QStringLiteral("读取元件检测框组失败, 错误消息: "));
            msg + errorMessage.c_str();
            System->showMessage(QStringLiteral("准备自动运行"), msg);
            return NOK;
        }

        if (!vecGroupId.empty()) {            
            for (const auto groupId : vecGroupId) {
                Engine::WindowGroup windowGroup;
                nResult = Engine::GetGroupWindows(groupId, windowGroup);
                if (Engine::OK != nResult) {
                    String errorType, errorMessage;
                    Engine::GetErrorDetail(errorType, errorMessage);
                    QString msg(QStringLiteral("读取元件检测框组失败, 错误消息: "));
                    msg + errorMessage.c_str();
                    System->showMessage(QStringLiteral("准备自动运行"), msg);
                    return NOK;
                }

                for (auto &window : windowGroup.vecWindows) {
                    window.x += m_fBoardLeft;
                    window.y += m_fBoardBottom;
                }

                deviceInpWindow.vecWindowGroup.push_back(windowGroup);
            }
        }

        m_vecDeviceInspWindow.push_back(deviceInpWindow);
    }
    
    return OK;
}

void QFlowCtrl::_refreshDisplayImage() {
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setImage(m_pAutoRunThread->getBigImage());
}

void QFlowCtrl::_onAutoRunError() {
    int nReturn = System->showInteractMessage(QString(QStringLiteral("自动运行")), m_pAutoRunThread->getErrorMsg());
    bool bExit = false;
    if (nReturn != QDialog::Accepted)
        bExit = true;
    m_pAutoRunThread->nofityResponse(bExit);
}