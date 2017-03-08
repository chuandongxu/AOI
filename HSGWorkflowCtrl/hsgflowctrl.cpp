#include <time.h>
#include <qthread.h>
#include <qthreadpool.h>
#include <qmessagebox.h>
#include <QApplication>
#include <qdebug.h>
#include <qthreadpool.h>
#include "hsgflowctrl.h"
#include "../Common/ModuleMgr.h"
#include "../Common/SystemData.h"
#include "../include/IMotion.h"
#include "../include/IdDefine.h"
#include "../include/ICAN.h"
#include "../Common/eos.h"
#include "../include/IBarCodeReader.h"
#include "../Common/CVSFile.h"
#include<QThread>
#include<QTime>
#include "../include/IFlowCtrl.h"
#include "../Common/ThreadPrioc.h"

#define POS_ZHOME   "zHome%0"
#define PROFILE_X   "xMoveProfile%0"
const QString TimeFormatString ="MMdd";

QCheckerRunable::QCheckerRunable( QCheckerParamMap * paramMap)
	:m_paramMap(paramMap),m_exit(false),m_normal("evAiValue.xml")	
{
}

QCheckerRunable::~QCheckerRunable()
{

}

void QCheckerRunable::quit()
{
	resetResoultLight();
	m_exit = true;	
}

void QCheckerRunable::imgStop()
{	
}

void QCheckerRunable::run()
{
	System->setTrackInfo(QString(QStringLiteral("模块%0启动成功")).arg(1));

	//bool bAutoCycle = System->getParam("sys_run_auto_cycle").toBool();
	//bool bCheckBarcode = System->getParam("sys_run_check_barcode").toBool();

	while(!isExit())
	{	
		if(isExit())break;

		double dtime_start = double(clock());
		//if(!moveToPressPos())continue;
		if(isExit())break;
		double dtime_movePos = double(clock());
		double dtime1;
		dtime1 = (dtime_movePos - dtime_start)/1000;
		System->setTrackInfo(QStringLiteral("运动到下压位置: %0 s").arg(dtime1),false);

		//if(!addPress())continue;
		if(isExit())break;	
		
	}

	System->setTrackInfo(QString(QStringLiteral("模块%0已停止")).arg(1));
}

bool QCheckerRunable::isExit()
{
	return m_exit;
}

void QCheckerRunable::setResoultLight(bool isOk)
{
	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		int okLight = 0;
		int ngLight = 0;
		if(getLightIO(okLight,ngLight))
		{
			if(isOk)
			{
				p->setExtDO(okLight,1);
				p->setExtDO(ngLight,0);
			}
			else
			{
				p->setExtDO(okLight,0);
				p->setExtDO(ngLight,1);
			}
		}
	}
}

void QCheckerRunable::resetResoultLight()
{
	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		int okLight = 0;
		int ngLight = 0;
		if(getLightIO(okLight,ngLight))
		{
			p->setExtDO(okLight,0);
			p->setExtDO(ngLight,0);
		}
	}
}

bool QCheckerRunable::getLightIO(int &okLight,int &ngLight)
{
	if(m_paramMap->contains(PARAM_STATION_OK_LIGHT)
		&& m_paramMap->contains(PARAM_STATION_NG_LIGHT))
	{
		okLight = m_paramMap->value(PARAM_STATION_OK_LIGHT).toInt();
		ngLight = m_paramMap->value(PARAM_STATION_NG_LIGHT).toInt();

		return true;
	}
	
	return false;
}

//*****************************************************************************
//
//
//******************************************************************************
QFlowCtrl::QFlowCtrl(QObject *parent)
	: QObject(parent),m_isHome(false),m_isStart(false),m_timerId(-1),m_normal("evAiValue.xml")
	,m_homeIng(false)
{
	m_barCode.clear();
	for(int i=0; i<STATION_COUNT; i++)
	{
		m_barCode.append(" ");		
	}
	m_errorCode = -1;
	m_timerId = startTimer(50);

	this->initStationParam();

	QSystem * p = QSystem::instance();
	bool s = connect(p,SIGNAL(goHome()),this,SLOT(home()));

	IMotion * pMotion = getModule<IMotion>(MOTION_MODEL);
	if(pMotion)
	{
		pMotion->setExtDO(DO_YELLOW_LIGHT,1);
		pMotion->setExtDO(DO_GREEN_LIGHT,0);
		pMotion->setExtDO(DO_RED_LIGHT,0);
		pMotion->setExtDO(DO_BUZZER,0);
	}
	System->setParam("sys_run_auto_cycle", (bool)false);
}

QFlowCtrl::~QFlowCtrl()
{
	if(-1 != m_timerId)killTimer(m_timerId);
	QCheckerParamMapList::iterator it = m_stationParams.begin();
	for(; it != m_stationParams.end(); ++it)delete *it;

	m_stationParams.clear();
}

bool QFlowCtrl::isRuning()
{
	return m_isStart;
}

void QFlowCtrl::home()
{	
	if(m_isStart)
	{
		QSystem::showMessage(QStringLiteral("提示"),QStringLiteral("设备正在运行中，请先停止在回零"));
		//QApplication::processEvents();
		return ;
	}

	m_homeIng=true;
	QEos::Notify(EVENT_GOHOMEING_STATE,GOHOMEING_STATE_OK);

	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		int iState = 1;
		p->getExtDI(DI_IM_STOP,iState);
		if(0 == iState)
		{
			QSystem::showMessage(QStringLiteral("提示"),QStringLiteral("设备处于急停状态，请先排除急停。"));
			return;
		}
	}
	else
	{
		System->setTrackInfo(QStringLiteral("无法回零，运动控制系统异常。"));
	}

		
	QSystem::showMessage(QStringLiteral("提示"),QStringLiteral("设备正在回零中..."),0);
	QApplication::processEvents();

	m_isHome = false;
	QEos::Notify(EVENT_GOHOME_STATE,GOHOME_STATE_NG);
	System->setErrorCode(ERROR_HOME_MOTION_ALM);
	QSystem::closeMessage();
}

void QFlowCtrl::timerEvent(QTimerEvent * event)
{
	static int count = 0;

	if(count > 50)
	{
		checkPowError();
		checkImStop();
		checkReset();
		checkStart();
		checkStop();
		readbarCode();
		checkMotionState();
		checkError();		
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
		p->getExtDI(DI_IM_STOP,iState);
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
		p->getExtDI(DI_RESET,iState);
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
			p->getExtDI(DI_START,iState);
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
			p->getExtDI(DI_STOP,iState);
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
	m_isHome = false;
	QCheckerRunableList::iterator it = m_checkStantion.begin();
	for(; it != m_checkStantion.end(); ++it)
	{
		QCheckerRunable * p = *it;
		if(p)p->imgStop();
	}
	this->stop();
	System->userImStop();

	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		p->setExtDO(DO_YELLOW_LIGHT,0);
		p->setExtDO(DO_GREEN_LIGHT,0);
		p->setExtDO(DO_RED_LIGHT,1);
		p->setExtDO(DO_BUZZER,1);
	}

	QSystem::showMessage(QStringLiteral("告警"),QStringLiteral("设备处于急停状态。"));
}
	
void QFlowCtrl::reset()
{
	if(m_isStart)
	{
		QMessageBox::warning(NULL,QStringLiteral("警告"),QStringLiteral("复位前请停止设备。"));
		return;
	}
	m_errorCode =-1;
	m_isHome = false;
	m_isStart = false;
	System->setErrorCode(ERROR_NO_ERROR);
	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		p->clearError();
		QThread::msleep(100);
		p->setExtDO(DO_YELLOW_LIGHT,1);
		p->setExtDO(DO_GREEN_LIGHT,0);
		p->setExtDO(DO_RED_LIGHT,0);
		p->setExtDO(DO_BUZZER,0);
	}	
	
	if(QSystem::isMessageShowed())
	{
		QSystem::closeMessage();
	}	
}
	
void QFlowCtrl::start()
{
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

	int nStationNum = STATION_COUNT;	

	m_checkStantion.clear();
	for(int i=0; i< nStationNum; i++)
	{
		if(true)
		{
			QCheckerRunable * p = new QCheckerRunable(m_stationParams[i]);
			m_checkStantion.append(p);
			QThreadPool::globalInstance()->start(p);
		}else
		{
			QCheckerRunable * p_disable = new QCheckerRunable(m_stationParams[i]);
			m_checkStantion.append(p_disable);
			IMotion * p = getModule<IMotion>(MOTION_MODEL);
			if(p)
			{
				p->setStationStart(i);
			}
		}
	}

	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		p->setExtDO(DO_YELLOW_LIGHT,0);
		p->setExtDO(DO_GREEN_LIGHT,1);
		p->setExtDO(DO_RED_LIGHT,0);
		p->setExtDO(DO_BUZZER,0);
	}

	m_isStart = true;

	QEos::Notify(EVENT_RUN_STATE,RUN_STATE_RUNING);
}
	
void QFlowCtrl::stop()
{
	m_isHome = false;
	for(int i = 0; i <STATION_COUNT; i++)
	{
		imStopStation(i);
	}

	QCheckerRunableList::iterator it = m_checkStantion.begin();
	for(; it != m_checkStantion.end(); ++it)
	{
		QCheckerRunable * p = *it;
		if(p)p->quit();
	}

	QThreadPool::globalInstance()->waitForDone();
	m_checkStantion.clear();

	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		p->setExtDO(DO_YELLOW_LIGHT,1);
		p->setExtDO(DO_GREEN_LIGHT,0);
		p->setExtDO(DO_RED_LIGHT,0);
		p->setExtDO(DO_BUZZER,0);

		p->releaseInputLock();
		p->releaseAllStationStart();
	}

	m_isStart = false;
	QEos::Notify(EVENT_RUN_STATE,RUN_STATE_STOP);
	QSystem::closeMessage();
}

void QFlowCtrl::initStationParam()
{
	//第一个检测器
	QCheckerParamMap *paramMap = new QCheckerParamMap;
	paramMap->insert(PARAM_STATION_ID, 0);
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
		p->getState(nAxis,axis,state);

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
	if(code < 0x40000000)
	{
		if(!System->isImStop())
		{
			this->imStop();
			m_errorCode = code;
		}
	}
}

void QFlowCtrl::checkPowError()
{
	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		if(p->IsPowerError())
		{
			QCheckerRunableList::iterator it = m_checkStantion.begin();
			for(; it != m_checkStantion.end(); ++it)
			{
				QCheckerRunable * p = *it;
				if(p)p->quit();
			}

			QThreadPool::globalInstance()->waitForDone();
			m_checkStantion.clear();

			QMessageBox::warning(NULL,QStringLiteral("警告"),QStringLiteral("系统错误或驱动掉电，请排查供电情况。软件即将退出。"));
			killTimer(m_timerId);
			m_timerId = -1;
			//QApplication::quit();		
		}
	}
}

void QFlowCtrl::initErrorCode()
{
	System->addErrorMap(ERROR_Z_POS_WARRING,MSG_Z_POS_WARRING);
	//System->addErrorMap(ERROR_Z_POS_WARRING,MSG_Z_POS_WARRING);
	//System->addErrorMap(ERROR_Z_POS_WARRING,MSG_Z_POS_WARRING);
	System->addErrorMap(ERROR_MOTION_POS_WARRING,MSG_MOTION_POS_WARRING);
	System->addErrorMap(ERROR_STATION_SAFE_GRATING_ALRM,MSG_STTATION_SAFE_GRATING_ALRM);
}

void QFlowCtrl::imStopStation(int nStation)
{	
	IMotion * p = getModule<IMotion>(MOTION_MODEL);
	if(p)
	{
		/*int limtCount[6] = {2,2,2,2,2,2};
		int axisTop[6] = {-1,-1,-1,-1,-1,-1};
		int count = getAllTopAxis(axisTop);
		p->stopMove(axisTop[nStation]);*/	
	}	
}