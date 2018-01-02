#include "TopWidget.h"
#include <qsettings.h>
#include <qapplication.h>
#include <qbytearray.h>
#include "../common/SystemData.h"
#include <QPainter>

#define HIDE_DEBUG_TOOL_BTN  0

QTopWidget::QTopWidget(QWidget *parent) :
    QWidget(parent)
{
    m_titleLabel = new QLabel();
    m_titleLabel->setObjectName("tp_title");
	m_titleLabel->setFixedHeight(50);
    //m_titleLabel->setFixedSize(800,50);

	m_versionLabel = new QLabel();
	m_versionLabel->setObjectName("tp_ver");

	QFont font = m_titleLabel->font();
	font.setPointSize(20);
	font.setWeight(62);
	m_titleLabel->setFont(font);
	m_titleLabel->setText(tr("vision soft"));
	m_versionLabel->setText(tr("soft ver"));

    m_exitBtn = new QPushButton();
    m_exitBtn->setObjectName("exit_btn");
    m_exitBtn->setFixedSize(24,24);

    m_mainLayout = new QHBoxLayout;
    m_titleLayout = new QHBoxLayout;
	m_verLayout = new QVBoxLayout;

	m_verLayout->addStretch();
	m_verLayout->addWidget(m_versionLabel,0,Qt::AlignBottom);
	m_verLayout->addSpacing(5);
 
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_titleLayout->setContentsMargins(0,0,0,0);
 
	m_titleLayout->addSpacing(350);
	//m_titleLayout->addWidget(m_titleLabel,0,Qt::AlignHCenter);
	//m_titleLayout->addLayout(m_verLayout);
	m_titleLayout->addStretch(0);
    m_titleLayout->addWidget(m_exitBtn,0,Qt::AlignRight|Qt::AlignTop);
	m_mainLayout->addLayout(m_titleLayout);
    this->setLayout(m_mainLayout);
	
	m_nTimerId = this->startTimer(100);
    connect(m_exitBtn,SIGNAL(clicked()),this,SIGNAL(closeBtnclick()));
	/*
    connect(m_debugBtn,SIGNAL(clicked()),this,SLOT(onDiagonseClick()));
    connect(m_settingBtn,SIGNAL(clicked()),this,SLOT(onSettingClick()));
	connect(m_warringLogBtn,SIGNAL(clicked()),this,SLOT(onShowWarring()));
	connect(m_goHomeBtn,SIGNAL(clicked()),this,SLOT(onGoHome()));
	*/
}

void QTopWidget::paintEvent(QPaintEvent *event)
{  
	QStyleOption option;
	option.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
	QWidget::paintEvent(event);

}


QString QTopWidget::getTitle()
{
	return m_titleLabel->text();
}
	
void QTopWidget::setTitle(const QString &title)
{
	m_titleLabel->setText(title);
}

void QTopWidget::onDiagonseClick()
{
 
}

void QTopWidget::onSettingClick()
{
   
}

void QTopWidget::onShowWarring()
{
	
}

void QTopWidget::onGoHome()
{
	
}

void QTopWidget::timerEvent(QTimerEvent * event)
{
	
}

void QTopWidget::setTitle(const QString &title,const QString &ver)
{
	m_titleLabel->setText(title);
	m_versionLabel->setText(ver);
}