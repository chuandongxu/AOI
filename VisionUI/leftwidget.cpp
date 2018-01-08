#include <QDateTime>
#include "leftwidget.h"
#include <qlistview>
#include "../common/SystemData.h"
#include "../Common/translatormgr.h"
#include "settingdialog.h"
#include "diagonedialog.h"
#include "WarringWidget.h"
#include "statewidget.h"
#include "../include/IdDefine.h"
#include "../Common/eos.h"
#include <QPainter>
#include <QDebug>

QLeftWidget::QLeftWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setFixedWidth(250);

	m_subLayout = new QVBoxLayout;
	m_stateWidget = new QStateWidget;
	m_subLayout->addWidget(m_stateWidget);
	m_subLayout->setContentsMargins(0,0,0,0);
	ui.frame->setLayout(m_subLayout);
	ui.frame->setContentsMargins(0,0,0,0);
	ui.widget->setContentsMargins(0,0,0,0);
	
	//ui.widget_leftbk->setWindowOpacity(0.7);
	//ui.widget_leftbk->setAttribute(Qt::WA_TranslucentBackground, true);	

	QString user;
	int level = 0;
	System->getUser(user,level);
	ui.lineEdit->setText(user);
	enableButton(level);

	QStringList ls =  QTranslatorMgr::getLangeKeys();
	ui.comboBox->addItems(ls);
	QTranslatorMgr * p = QTranslatorMgr::instance();
	if(p)
	{
		QString lang = p->getLangName(System->getLangConfig());
		ui.comboBox->setCurrentText(lang);
	}

	connect(ui.pushButton_2,SIGNAL(clicked()),this,SLOT(onDiagonseClick()));
	connect(ui.pushButton,SIGNAL(clicked()),this,SLOT(onSettingClick()));
	connect(ui.pushButton_3,SIGNAL(clicked()),this,SLOT(onShowWarring()));
	connect(ui.pushButton_4,SIGNAL(clicked()),this,SLOT(onGoHome()));

	connect(ui.pushButton_5,SIGNAL(clicked()),this,SLOT(onChangeUser()));
	connect(ui.pushButton_6,SIGNAL(clicked()),this,SLOT(onManageUser()));
	connect(ui.comboBox,SIGNAL(currentIndexChanged(const QString &)),this,SLOT(onChangeLang(const QString &)));

	QEos::Attach(EVENT_RUN_STATE,this,SLOT(onRunState(const QVariantList &)));

	m_timerId = this->startTimer(1000);
	ui.pushButton->setText("");
	ui.pushButton->setObjectName("setting_btn");	

	ui.pushButton_3->setText("");
	ui.pushButton_3->setObjectName("log_btn");

	ui.pushButton_2->setText("");
	ui.pushButton_2->setObjectName("debug_btn");

	ui.pushButton_4->setText("");
	ui.pushButton_4->setObjectName("home_btn");

	//m_dlgSetting.setWindowModality(Qt::ApplicationModal);
}

QLeftWidget::~QLeftWidget()
{
	this->killTimer(m_timerId);
}

void QLeftWidget::paintEvent(QPaintEvent *event)
{  
	QStyleOption option;
	option.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
	QWidget::paintEvent(event);

}

void QLeftWidget::setStateWidget(QWidget * w)
{
	//if(m_stateWidget)
	//{
	//	delete m_stateWidget;
	//	m_stateWidget = w;
	//	m_subLayout->addWidget(m_stateWidget);
	//}
}

void QLeftWidget::onDiagonseClick()
{
	QDiagoneDialog dlg;
	dlg.exec();
}

void QLeftWidget::onSettingClick()
{
	//QSettingDialog dlg;
	//dlg.exec();
	//Qt::WindowFlags flags = m_dlgSetting.windowFlags();
	//flags |= Qt::WindowStaysOnTopHint;
	//m_dlgSetting.setWindowFlags(flags);
	m_dlgSetting.show();
}

void QLeftWidget::onShowWarring()
{
	QWarringWidget widget;
	widget.exec();
}

void QLeftWidget::onGoHome()
{
	System->userGoHome();
}

void QLeftWidget::onChangeUser()
{
	if(QSystem::ChangeUser())
	{
		QString user;
		int level;
		System->getUser(user,level);

		System->setTrackInfo(QString(QStringLiteral("切换到用户%0").arg(user)));

		ui.lineEdit->setText(user);
		enableButton(level);

		QVariantList data;
		QEos::Notify(EVENT_CHANGE_USER,data);
	}
}

void QLeftWidget::enableButton(int iLevel)
{
	ui.pushButton_6->setEnabled(false);
	//ui.pushButton->setEnabled(false);
	ui.pushButton_2->setEnabled(false);
	if(USER_LEVEL_MANAGER == iLevel)
	{		
		ui.pushButton->setEnabled(true);
	}	
	else if(USER_LEVEL_TECH == iLevel)
	{		
		ui.pushButton->setEnabled(true);
		ui.pushButton_6->setEnabled(true);
	}
	else if(USER_LEVEL_DEBUG == iLevel)
	{
		ui.pushButton_6->setEnabled(true);
		ui.pushButton->setEnabled(true);
		ui.pushButton_2->setEnabled(true);
	}	
}

void QLeftWidget::onManageUser()
{
	QSystem::ManagerUser();
}

void QLeftWidget::timerEvent(QTimerEvent * event)
{
	if(event->timerId() == m_timerId)
	{
		QDateTime dtm = QDateTime::currentDateTime();

		QString strTime = dtm.toString("yyyy-MM-dd,hh:mm:ss");
		QStringList ls = strTime.split(",");
		ui.label->setText(ls[0]);
		ui.label_3->setText(ls[1]);
	}
}

void QLeftWidget::onChangeLang(const QString &text)
{
	QTranslatorMgr * p = QTranslatorMgr::instance();
	if(p)
	{
		QString str = p->getLangPostfix(text);
		p->changeLange(str);

		System->setLangConfig(str);
	}
}

void QLeftWidget::onRunState(const QVariantList &data)
{
	if(data.size() == 1)
	{
		int iState = data[0].toInt();
		if(RUN_STATE_RUNING == iState)
		{
			if(USER_LEVEL_MANAGER == System->getUserLevel())
			{
				ui.pushButton->setEnabled(false);				
			}
			else if(USER_LEVEL_TECH == System->getUserLevel())
			{
				ui.pushButton->setEnabled(false);
			}
			else if(USER_LEVEL_DEBUG == System->getUserLevel())
			{
				ui.pushButton->setEnabled(false);
				ui.pushButton_2->setEnabled(false);
			}
		}
		else 
		{
			if(USER_LEVEL_MANAGER == System->getUserLevel())
			{
				ui.pushButton->setEnabled(true);				
			}
			else if(USER_LEVEL_TECH == System->getUserLevel())
			{
				ui.pushButton->setEnabled(true);
			}
			else if(USER_LEVEL_DEBUG == System->getUserLevel())
			{
				ui.pushButton->setEnabled(true);
				ui.pushButton_2->setEnabled(true);
			}
		}
	}
}