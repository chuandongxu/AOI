 #include "AppMainWidget.h"
#include <qfiledialog.h>
#include "../Common/SystemData.h"
#include "../include/IdDefine.h"
#include <qfile.h>
#include "TimeWidget.h"
#include "../Common/eos.h"
#include "../include/IFlowCtrl.h"
#include "../Common/ModuleMgr.h"
#include "../include/ILight.h"
#include "../include/IVision.h"
#include "../include/IMotion.h"
#include <QMessageBox>
#include <qdesktopwidget.h>
#include <QDateTime.h>
#define _NORMAL_WIN_WIDTH_ 1440

QAppMainWidget::QAppMainWidget(QWidget *parent)
    : QFrame(parent)
{
	//QBottonWidget::initBottomModel();

	this->setObjectName("mainWidget");
    m_topwidget = new QTopWidget;
	m_leftWidget = new QLeftWidget;	
    m_centerWidget = new QCenterWidget;
	m_bottonWidget = new QBottonWidget;
	m_dispBkWidget = new QWidget;
	
	m_topwidget->setObjectName("topWidget");	
	m_leftWidget->setObjectName("leftWidget");	
	m_dispBkWidget->setObjectName("dispBkwidget");

    m_dispBkLayout = new QVBoxLayout;
	m_leftLayout = new QHBoxLayout;	
	m_centerLayout = new QVBoxLayout;
	m_mainLayout = new QVBoxLayout;

	m_topwidget->setFixedHeight(95);
	m_centerWidget->setFixedHeight(900);	
	m_bottonWidget->setFixedHeight(150);
	QDesktopWidget* desktopWidget = QApplication::desktop();
	if(desktopWidget)
	{
		QRect applicationRect = desktopWidget->screenGeometry();
		int nScreenWidth = applicationRect.width();

		if(abs(_NORMAL_WIN_WIDTH_ - nScreenWidth) > 10)
		{			
			//m_bottonWidget->setFixedHeight(HSG_TYPE_HSG1 == hsgType ? 150 : 112);
			//m_bottonWidget->setFixedHeight(150);
		}
	}
	m_dispBkLayout->getContentsMargins(0,0,0,0);
	m_dispBkLayout->addWidget(m_centerWidget);
	m_dispBkLayout->addStretch();
	m_dispBkLayout->addWidget(m_bottonWidget,0,Qt::AlignBottom);
	m_dispBkWidget->setContentsMargins(0,0,0,0);
	m_dispBkWidget->setLayout(m_dispBkLayout);	

	m_centerLayout->setContentsMargins(0,0,0,0);
	m_centerLayout->addWidget(m_topwidget);
	m_centerLayout->addWidget(m_dispBkWidget);	
	m_centerLayout->addStretch();	

	m_leftLayout->setContentsMargins(0,0,0,0);
	m_leftLayout->addWidget(m_leftWidget);
	m_leftLayout->addLayout(m_centerLayout);	

	m_mainLayout->setContentsMargins(0,0,15,0);
	m_mainLayout->addLayout(m_leftLayout);
    this->setLayout(m_mainLayout);

    connect(m_topwidget,SIGNAL(closeBtnclick()),this,SLOT(onCloseBtnclick()));

	System->setTrackInfo(QStringLiteral("系统已启动"));

	this->onChangeModuleType(QVariantList());
	//QEos::Attach(EVENT_HSG_TYPE, this, SLOT(onChangeModuleType(const QVariantList &)));
}

QAppMainWidget::~QAppMainWidget()
{
}

void QAppMainWidget::onChangeModuleType(const QVariantList &vars)
{
	QString str = QStringLiteral("DEMO");

	QString title = m_topwidget->getTitle();
	int n = title.indexOf('(');
	title = title.left(n);
	title += "(" + str + ")";
	m_topwidget->setTitle(title);
}

void QAppMainWidget::onCloseBtnclick()
{
	IFlowCtrl * p = getModule<IFlowCtrl>(CTRL_MODEL);
	if(p)
	{
		if(p->isRuning())
		{
			QMessageBox::warning(this,QStringLiteral("警告"),QStringLiteral("设备正在运行中，请先停止设备再退出。"));
			return;
		}
	}

	if(System->isEnableBackupData())
	{
		QString dataPath = QApplication::applicationDirPath() + "/data/";
		QString backupPath = System->getBackupDataPath();

		QDateTime dtm = QDateTime::currentDateTime();
		QString strdate = dtm.toString("yyyy-MM-dd-hhmmss");
		QString backupFullPath = backupPath + "/" + strdate + "/";
		
		QDir dir;
		if(!dir.exists(backupPath))
		{
			dir.mkdir(backupPath);
		}
		dir.mkdir(backupFullPath);

		QFile::copy(dataPath + "defaultProduct.xml",	backupFullPath + "defaultProduct.xml");
		QFile::copy(dataPath + "systemData.s3db",		backupFullPath + "systemData.s3db");
	}

	ILight* pLight = getModule<ILight>(LIGHT_MODEL);
	if (pLight)
	{
		 pLight->getLightWidget()->close();
	}

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (pVision)
	{
		pVision->getColorWeightView()->close();
	}

    IMotion *pMotion = getModule<IMotion>(MOTION_MODEL);
    if (pMotion)
        pMotion->getJoyStickWidget()->close();

	this->close();
}

void QAppMainWidget::setStateWidget(QWidget * w)
{
	m_leftWidget->setStateWidget(w);
}

void QAppMainWidget::setCenterWidget(QWidget * w)
{
	m_centerWidget->setCenterWidget(w);
}

void QAppMainWidget::setTitle(const QString &title,const QString &ver)
{
	m_topwidget->setTitle(title,ver);
}