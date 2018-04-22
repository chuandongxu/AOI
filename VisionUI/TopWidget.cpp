#include "TopWidget.h"
#include <QSettings>
#include <QApplication>
#include <QByteArray>
#include <QFileDialog>
#include <QPainter>
#include "../common/SystemData.h"
#include "../Common/eos.h"
#include "../include/IdDefine.h"
#include "../include/IData.h"
#include "../include/IVision.h"
#include "../Common/ModuleMgr.h"
#include "../include/constants.h"
#include "AboutDialog.h"

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
	m_toolLayout = new QHBoxLayout;
	m_verLayout = new QVBoxLayout;

	m_verLayout->addStretch();
	m_verLayout->addWidget(m_versionLabel,0,Qt::AlignBottom);
	m_verLayout->addSpacing(5);
 
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_titleLayout->setContentsMargins(0,0,0,0);
	m_toolLayout->setContentsMargins(0, 0, 0, 0);

	m_titleLayout->addSpacing(420);
	//m_titleLayout->addWidget(m_titleLabel,0,Qt::AlignHCenter);
	//m_titleLayout->addLayout(m_verLayout);
	m_titleLayout->addStretch(0);
    m_titleLayout->addWidget(m_exitBtn,0,Qt::AlignRight|Qt::AlignTop);

    const QSize szBtn(50, 50);
	QString stytleStr = "background-color: rgb(32, 105, 138);color:white;";

    m_toolBtnAbout = new QPushButton();
    m_toolBtnAbout->setFixedSize(szBtn);
    QIcon iconAbout("./image/About.png");
    m_toolBtnAbout->setIcon(iconAbout);
    m_toolBtnAbout->setFixedSize(szBtn);
    m_toolBtnAbout->setIconSize(szBtn);
    m_toolBtnAbout->setToolTip(QStringLiteral("About"));

    m_toolBtnNewProject = new QPushButton();
    QIcon iconNewProject("./image/NewProject_32x32.png");
    m_toolBtnNewProject->setIcon(iconNewProject);
    m_toolBtnNewProject->setFixedSize(szBtn);
    m_toolBtnNewProject->setIconSize(szBtn);
    m_toolBtnNewProject->setToolTip(QStringLiteral("New Project"));
    //m_toolBtnNewProject->setStyleSheet(stytleStr);

    m_toolBtnOpenProject = new QPushButton();
    m_toolBtnOpenProject->setFixedSize(szBtn);
    QIcon iconOpenProject("./image/OpenProject_32x32.png");
    m_toolBtnOpenProject->setIcon(iconOpenProject);
    m_toolBtnOpenProject->setFixedSize(szBtn);
    m_toolBtnOpenProject->setIconSize(szBtn);
    m_toolBtnOpenProject->setToolTip(QStringLiteral("Open Project"));

	m_toolBtnAutoRun = new QPushButton();
	m_toolBtnAutoRun->setFixedSize(100, 25);
	m_toolBtnAutoRun->setText(QStringLiteral("开始运行"));
	m_toolBtnAutoRun->setStyleSheet(stytleStr);
	m_toolBtnSys = new QPushButton();
	m_toolBtnSys->setFixedSize(100, 25);
	m_toolBtnSys->setText(QStringLiteral("系统标定"));
	m_toolBtnSys->setStyleSheet(stytleStr);
	m_toolBtnHw = new QPushButton();
	m_toolBtnHw->setFixedSize(100, 25);
	m_toolBtnHw->setText(QStringLiteral("保留选项（硬件）"));
	m_toolBtnHw->setStyleSheet(stytleStr);
	m_toolBtnTools = new QPushButton();
	m_toolBtnTools->setFixedSize(100, 25);
	m_toolBtnTools->setText(QStringLiteral("编辑检测框"));
	m_toolBtnTools->setStyleSheet(stytleStr);
	m_toolBtnSetting = new QPushButton();
	m_toolBtnSetting->setFixedSize(100, 25);
	m_toolBtnSetting->setText(QStringLiteral("检测设置"));
	m_toolBtnSetting->setStyleSheet(stytleStr);
	m_toolBtnData = new QPushButton();
	m_toolBtnData->setFixedSize(100, 25);
	m_toolBtnData->setText(QStringLiteral("元件资料"));
	m_toolBtnData->setStyleSheet(stytleStr);

    m_toolLayout->addWidget(m_toolBtnAbout, 0, Qt::AlignLeft | Qt::AlignBottom);
	m_toolLayout->addSpacing(500);
    m_toolLayout->addWidget(m_toolBtnNewProject, 0, Qt::AlignLeft | Qt::AlignBottom);
    m_toolLayout->addWidget(m_toolBtnOpenProject, 0, Qt::AlignLeft | Qt::AlignBottom);
	m_toolLayout->addWidget(m_toolBtnAutoRun, 0, Qt::AlignLeft | Qt::AlignBottom);
	m_toolLayout->addWidget(m_toolBtnSys, 0, Qt::AlignLeft | Qt::AlignBottom);
	m_toolLayout->addWidget(m_toolBtnHw, 0, Qt::AlignLeft | Qt::AlignBottom);
	m_toolLayout->addWidget(m_toolBtnTools, 0, Qt::AlignLeft | Qt::AlignBottom);
	m_toolLayout->addWidget(m_toolBtnSetting, 0, Qt::AlignLeft | Qt::AlignBottom);
	m_toolLayout->addWidget(m_toolBtnData, 0, Qt::AlignLeft | Qt::AlignBottom);

	m_mainLayout->addLayout(m_toolLayout);
	m_mainLayout->addLayout(m_titleLayout);
    this->setLayout(m_mainLayout);
	
	m_nTimerId = this->startTimer(100);
    connect(m_exitBtn,SIGNAL(clicked()),this,SIGNAL(closeBtnclick()));
	
    connect(m_toolBtnAbout, SIGNAL(clicked()), this, SLOT(onAbout()));
    connect(m_toolBtnNewProject, SIGNAL(clicked()), this, SLOT(onNewProject()));
    connect(m_toolBtnOpenProject, SIGNAL(clicked()), this, SLOT(onOpenProject()));
	connect(m_toolBtnAutoRun, SIGNAL(clicked()), this, SLOT(onAutoRun()));
	connect(m_toolBtnSys, SIGNAL(clicked()), this, SLOT(onSystem()));
	connect(m_toolBtnHw, SIGNAL(clicked()), this, SLOT(onHardware()));
	connect(m_toolBtnTools, SIGNAL(clicked()), this, SLOT(onTools()));
	connect(m_toolBtnSetting, SIGNAL(clicked()), this, SLOT(onSetting()));
	connect(m_toolBtnData, SIGNAL(clicked()), this, SLOT(onData()));	
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

void QTopWidget::onAbout()
{
    AboutDialog aboutDialog;

    IData* pData = getModule<IData>(DATA_MODEL);
    aboutDialog.setDataStoreApiVersion(pData->getDataStoreApiVersion());

    auto pVision = getModule<IVision>(VISION_MODEL);
    aboutDialog.setVisionLibraryVersion(pVision->getVisionLibraryVersion());

    aboutDialog.exec();
}

void QTopWidget::onNewProject()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Project Files (*.aoi)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())  {
        fileNames = dialog.selectedFiles();
    }else
        return;

    IData* pData = getModule<IData>(DATA_MODEL);
    auto strFilePath = fileNames[0];
    if ( ! strFilePath.endsWith ( PROJECT_EXT.c_str(), Qt::CaseInsensitive) ) {
        strFilePath += PROJECT_EXT.c_str();
    }
    pData->createProject ( strFilePath );
}

void QTopWidget::onOpenProject()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilter(tr("Project Files (*.aoi)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())  {
        fileNames = dialog.selectedFiles();
    }else
        return;

    IData* pData = getModule<IData>(DATA_MODEL);
    pData->openProject(fileNames[0]);
}

void QTopWidget::onAutoRun()
{
	QEos::Notify(EVENT_UI_STATE, 0, RUN_UI_STATE_AUTORUN);
}

void QTopWidget::onSystem()
{
	QEos::Notify(EVENT_UI_STATE, 0, RUN_UI_STATE_CALI);
}

void QTopWidget::onHardware()
{
	QEos::Notify(EVENT_UI_STATE, 0, RUN_UI_STATE_HARDWARE);
}

void QTopWidget::onTools()
{
	QEos::Notify(EVENT_UI_STATE, 0, RUN_UI_STATE_TOOLS);
}

void QTopWidget::onSetting()
{
	QEos::Notify(EVENT_UI_STATE, 0, RUN_UI_STATE_SETTING);
}

void QTopWidget::onData()
{
	QEos::Notify(EVENT_UI_STATE, 0, RUN_UI_STATE_DATA);
}

void QTopWidget::timerEvent(QTimerEvent * event)
{
	
}

void QTopWidget::setTitle(const QString &title,const QString &ver)
{
	m_titleLabel->setText(title);
	m_versionLabel->setText(ver);
}