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

	m_pMainParamMap = NULL;
	m_pAutoRunThread = NULL;

	this->initStationParam();

	QSystem * p = QSystem::instance();
	bool s = connect(p,SIGNAL(goHome()),this,SLOT(home()));
	s = connect(p, SIGNAL(start()), this, SLOT(startAutoRun()));
	s = connect(p, SIGNAL(stop()), this, SLOT(stopAutoRun()));

	m_dateTime = QDateTime::currentDateTime();

	QEos::Attach(EVENT_IMAGE_STATE, this, SLOT(onImageEvent(const QVariantList &)));

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
	if(-1 != m_timerId)killTimer(m_timerId);
	QCheckerParamMapList::iterator it = m_stationParams.begin();
	for(; it != m_stationParams.end(); ++it)delete *it;

	m_stationParams.clear();
	m_stationDatas.clear();

	if (m_pMainParamMap)
	{
		delete m_pMainParamMap;
		m_pMainParamMap = NULL;
	}
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

void QFlowCtrl::home()
{	
	if(m_isStart)
	{
		QSystem::showMessage(QStringLiteral("提示"),QStringLiteral("设备正在运行中，请先停止在回零"));
		QApplication::processEvents();

		//this->stop();
		return ;
	}

	if (!m_isHome)
	{
		m_homeIng = true;
		QEos::Notify(EVENT_GOHOMEING_STATE, GOHOMEING_STATE_OK);

		//急停判断
		IMotion * p = getModule<IMotion>(MOTION_MODEL);
		if (p)
		{
			int iState = 1;
			//p->getExtDI(DI_IM_STOP, iState);
			if (0 == iState)
			{
				QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备处于急停状态，请先排除急停。"));
				return;
			}
		}
		else
		{
			System->setTrackInfo(QStringLiteral("系统暂无控制系统，无法回零。"));			
		}

		QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在回零中..."), 0);
		QApplication::processEvents();

		if (p->homeAll(true))
		{
			m_isHome = true;
			QEos::Notify(EVENT_GOHOME_STATE, GOHOME_STATE_OK);
			QSystem::closeMessage();
		}
		else
		{
			m_isHome = false;
			QEos::Notify(EVENT_GOHOME_STATE, GOHOME_STATE_NG);
			System->setErrorCode(ERROR_HOME_MOTION_ALM);
			QSystem::closeMessage();
		}
	}

	//if (m_isHome)
	//{
		//this->start();
	//}	
}

void QFlowCtrl::startAutoRun()
{
	start();
}

void QFlowCtrl::stopAutoRun()
{
	if (m_isStart) stop();
}

void QFlowCtrl::autoThreadFinish()
{
    delete m_pAutoRunThread;
    m_pAutoRunThread = NULL;
}

void QFlowCtrl::timerEvent(QTimerEvent * event)
{
	static int count = 0;

	if(count > 50)
	{
		checkImStop();
		checkReset();
		checkStart();
		checkStop();
		readbarCode();
		checkMotionState();
		checkError();
		checkAuthError();
	}
	else count++;
}


void QFlowCtrl::checkImStop()
{
	static int s_iState = 1;

	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		int iState = 1;
		//p->getExtDI(DI_IM_STOP,iState);
		if(0 == iState && 1 == s_iState)
		{
			this->imStop();
			s_iState = iState;
		}
		else if(1 == iState)
		{
			s_iState = 1;
		}
	}
}

void QFlowCtrl::checkReset()
{
	static int s_iState = 0;

	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		int iState = 0;
		//p->getExtDI(DI_RESET,iState);
		if(1 == iState && 0 == s_iState)
		{
			this->reset();
			s_iState = iState;
		}
		else if(0 == iState)
		{
			s_iState = 0;
		}
	}
}

void QFlowCtrl::checkStart()
{
	static int s_iState = 0;
	static clock_t s_clock = 0;

	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		int iState = 0;
		if(System->isEnableOutline())
		{
			iState = System->data(OUT_LINE_RUN).toInt();
		}
		else
		{
			//p->getExtDI(DI_START,iState);
		}

		if(1 == iState && 0 == s_iState)
		{
			//启动按钮持续按下200ms以上在响应
			if(0 == s_clock)
			{
				s_clock = clock();
			}
			else
			{
				clock_t tm = clock();
				if((tm - s_clock) > 50)
				{
					this->start();
					s_iState = iState;
					s_clock = 0;
				}
			}
		}
		else if(0 == iState)
		{
			s_iState = 0;
			s_clock = 0;
		}
	}
}

void QFlowCtrl::checkStop()
{
	static int s_iState = 0;

	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		int iState = 0;
		if(System->isEnableOutline())
		{
			iState = System->data(OUT_LINE_STOP).toInt();
		}
		else
		{
			//p->getExtDI(DI_STOP,iState);
		}
		
		if(1 == iState && 0 == s_iState)
		{			
			this->stop();
			s_iState = iState;
		}
		else if(0 == iState)
		{
			s_iState = 0;
		}
	}
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
    Engine::AlignmentVector vecAlignments;
    int nResult = Engine::GetAllAlignments(vecAlignments);
    if (Engine::OK != nResult) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get alignment from data base, error message " + errorMessage;
        System->showMessage(QStringLiteral("Prepare auto run"), errorMessage.c_str());
        return;
    }

    if (vecAlignments.size() <= 1) {
        System->showMessage(QStringLiteral("Prepare auto run"), QStringLiteral("Please set at least 2 alignment point!"));
        return;
    }

    float left = 0, top = 0, right = 0, bottom = 0;
    nResult = Engine::GetBigBoardCoords(left, top, right, bottom);
    if (Engine::OK != nResult) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get big board coordinates from data base, error message " + errorMessage;
        System->showMessage(QStringLiteral("Prepare auto run"), errorMessage.c_str());
        return;
    }

    Engine::WindowVector vecWindows;
    nResult = Engine::GetAllWindows(vecWindows);
    if (Engine::OK != nResult) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get inspection windows from data base, error message " + errorMessage;
        System->showMessage(QStringLiteral("Prepare auto run"), errorMessage.c_str());
        return;
    }

    auto fResolutionX = System->getSysParam("CAM_RESOLUTION_X").toFloat();
    auto fResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toFloat();
    float fovWidth = 350.f, fovHeight = 350.f;
    Vision::VectorOfVectorOfPoint2f vecVecFrameCtr;
    nResult = DataUtils::assignFrames(left, top, right, bottom, fovWidth, fovHeight, vecVecFrameCtr);

	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return;

	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return;

	IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
	if (!pUI) return;

	if(!m_isHome)
	{		
		QSystem::showMessage(QStringLiteral("提示"),QStringLiteral("请先将设备回零"));
		return;
	}

	if(m_isStart)
	{		
		QSystem::showMessage(QStringLiteral("提示"),QStringLiteral("设备正在运行，请先按停止按钮"));
		return;
	}

	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在启动中..."), 0);
	QApplication::processEvents();

	System->setParam("camera_capture_num_mode", 0);// all capturing images

    if (! System->isRunOffline()) {
	    if (pCam->getCameraNum() > 0)
	    {
		    if (!pCam->startUpCapture() || !pUI->startUpCapture())
		    {
			    QSystem::closeMessage();
			    QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("相机初始化问题。"));
			    return;
		    }
	    }else {
		    QSystem::closeMessage();
		    QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("请检查相机是否连接。"));
		    return;
	    }
    }

	int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
    for (int i = 0; i < nStationNum; ++ i)
	{
		if (pDlp->isConnected(i))
		{
			if (! pDlp->startUpCapture(i))
                continue;
        }
		else
			System->setTrackInfo(QString(QStringLiteral("工位%0启动失败, 请检查DLP硬件！")).arg(i + 1));
	}

    m_pAutoRunThread = new AutoRunThread(m_pMainParamMap, &m_stationDatas, vecAlignments, vecWindows, vecVecFrameCtr);
    connect(m_pAutoRunThread, &AutoRunThread::finished, m_pAutoRunThread, &QObject::deleteLater);
    m_pAutoRunThread->start();

	m_isStart = true;

	QEos::Notify(EVENT_RUN_STATE,RUN_STATE_RUNING);

	System->setParam("camera_show_image_toScreen_enable", false);

	QSystem::closeMessage();
}
	
void QFlowCtrl::stop()
{	
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return;

	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return;	

	IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
	if (!pUI) return;
	
	//m_isHome = false;

	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("设备正在停止中..."), 0);
	QApplication::processEvents();

	if (m_pAutoRunThread) m_pAutoRunThread->quit();

	QThreadPool::globalInstance()->waitForDone();

	if (pCam->getCameraNum() > 0)
	{
		pCam->endUpCapture();
	}
	pUI->endUpCapture();
	
    int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
	for (int i = 0; i < nStationNum; i++)
	{
		if (pDlp->isConnected(i))
		{
			if (!pDlp->endUpCapture(i))
                continue;
		}
	}

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
	QEos::Notify(EVENT_RUN_STATE,RUN_STATE_STOP);
	QSystem::closeMessage();
}

void QFlowCtrl::initStationParam()
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
	m_stationParams.append(paramMap);
	m_stationDatas.append(Q3DStructData());

	paramMap = new QCheckerParamMap;
	paramMap->insert(PARAM_STATION_ID, 3);
	m_stationParams.append(paramMap);
	m_stationDatas.append(Q3DStructData());

	paramMap = new QCheckerParamMap;
	paramMap->insert(PARAM_STATION_ID, 4);
	m_stationParams.append(paramMap);
	m_stationDatas.append(Q3DStructData());

	m_pMainParamMap = new QCheckerParamMap;
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
	static bool alm_state = false;

	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		int nAxis = 0;
		int axis[12];
		IMotion::STATE state[12];
		memset(axis,0,sizeof(int));
		memset(state,0,sizeof(IMotion::STATE));
		//p->getState(nAxis,axis,state);

		for(int i=0; i<nAxis; i++)
		{
			if(state[i] == IMotion::STATE_ALM)
			{
				if(!alm_state)
				{
					alm_state = true;
					System->setErrorCode(ERROR_MOTOR_ALM);
					return;
				}
			}
		}

		alm_state = false;
	}
}

void QFlowCtrl::checkError()
{
	unsigned int code = System->getErrorCode();
	if( m_errorCode == code) return;
	if (code < 0x30000000)
	{
		if(!System->isImStop())
		{
			this->imStop();
			m_errorCode = code;
		}
	}
}

void QFlowCtrl::initErrorCode()
{
	System->addErrorMap(ERROR_ATHU_NORIGHT_WARRING, MSG_ATH_NORIGHT_WARRING);
	//System->addErrorMap(ERROR_Z_POS_WARRING,MSG_Z_POS_WARRING);
	//System->addErrorMap(ERROR_Z_POS_WARRING,MSG_Z_POS_WARRING);
	System->addErrorMap(ERROR_MOTION_POS_WARRING,MSG_MOTION_POS_WARRING);
	System->addErrorMap(ERROR_STATION_SAFE_GRATING_ALRM,MSG_STTATION_SAFE_GRATING_ALRM);
}