#include "ErrorCode.h"
#include <qsqlquery>
#include <qvariant>
#include <QMessageBox> 
#include <qapplication.h>
#include "sysmessagebox.h"
#include "track.h"
#include "SystemData.h"
#include "userdialog.h"
#include "usermanager.h"

QErrorCode::QErrorCode(void)
{
}


QErrorCode::~QErrorCode(void)
{
}

const QString QErrorCode::getErrorString(unsigned int error)
{
	if(m_errorMap.contains(error))
	{
		return m_errorMap[error];
	}

	return "";
}

void QErrorCode::init()
{
	m_errorMap[ERROR_NO_ERROR]				= QStringLiteral("");
	m_errorMap[ERROR_START_UP]				= QStringLiteral("系统启动。");
	m_errorMap[ERROR_SHUTDOWN]				= QStringLiteral("系统退出。");
	m_errorMap[ERROR_CVS_OPEN_ALM]          = QStringLiteral("测量结果记录失败，请检查配置路劲是否正确，是否有写权限");
}

void QErrorCode::addErrorMap(unsigned int id,const QString &descr)
{
	m_errorMap[id] = descr;
}

//----------------------------------------------------------------------------------------------------
static QSysMessageBox *g_box = NULL;
static bool g_bInitBox = false;
static bool g_bShowed = false;
static QWidget * g_mainWidget = NULL;

void QSystem::showMessage(const QString &title,const QString &msg,int ErrorLevel)
{
	if(!g_bInitBox)
	{
		g_box = new QSysMessageBox;
		g_bInitBox = true;
		
		g_box->setWindowModality(Qt::ApplicationModal);
		Qt::WindowFlags fs = g_box->windowFlags();
		fs = Qt::CustomizeWindowHint|Qt::FramelessWindowHint;
		g_box->setWindowFlags(fs);

		//connect(QApplication::instance(),SIGNAL(aboutToQuit()),SLOT(onQiut()));
	}

	if(g_bShowed)
	{
		g_box->setText(msg);
		g_box->setTitle(title);
		if(ErrorLevel > 3)g_box->enableCloseBtn(true);
		else g_box->enableCloseBtn(false);

		g_box->show();
		g_bShowed = true;

		QApplication::processEvents();
	}
	else
	{
		g_box->setText(msg);
		g_box->setTitle(title);
		if(ErrorLevel > 3)g_box->enableCloseBtn(true);
		else g_box->enableCloseBtn(false);
		
		g_box->show();
		g_bShowed = true;
	}

	/*
	if(g_box)delete g_box;
	
	g_box = new QSysMessageBox(g_mainWidget);
		
		
		
	g_box->setWindowModality(Qt::ApplicationModal);
	Qt::WindowFlags fs = g_box->windowFlags();
	fs = Qt::CustomizeWindowHint|Qt::FramelessWindowHint;
	g_box->setWindowFlags(fs);

	g_box->setText(msg);
	g_box->setEnableCloseBtn(false);
		
	g_box->show();
	*/
}

void QSystem::closeMessage()
{
	if(g_box)
	{
		g_box->hide();
		g_bShowed = false;
		QApplication::processEvents();
	}
	
	/*
	if(g_box)
	{
		delete g_box;
		g_box = NULL;
		if(g_mainWidget)g_mainWidget->activateWindow();
	}
	*/
}

bool QSystem::isMessageShowed()
{
	return g_bShowed;
}

void QSystem::setMainWidget(QWidget * widget)
{
	g_mainWidget = widget;
}

QWidget * QSystem::getMainWidget()
{
	return g_mainWidget;
}

bool QSystem::ChangeUser()
{
	QUserDialog dlg;
	if(dlg.exec() == QDialog::Accepted)return true;

	return false;
}

void QSystem::ManagerUser()
{
	QUserManager dlg;
	dlg.exec();
}
//-----------------------------------------------------------

void QErrorModel::onErrorInfo(const QString &data,const QString &msg,unsigned int level)
{
	int row = rowCount();
	if(row > 30)
	{
		removeRow(0);
		row = rowCount();
	}
	
	unsigned int warringFlsg = (level & 0xf0000000) >> 28;
	QString path = QApplication::applicationDirPath();
	//if(level >= 0x40000000)path += "/image/tr_info.png";
	//else if(level >= 0x30000000)path += "/image/tr_warring.png";
	//else path += "/image/tr_error.png";

	if(0 == warringFlsg || 1 == warringFlsg) path += "/image/tr_error.png";
	else if(3 == warringFlsg || 5 == warringFlsg) path += "/image/tr_warring.png";
	else path += "/image/tr_info.png";
	
	QPixmap pixmap;
	pixmap.load(path);

	insertRow(row);
	setData(index(row,0),pixmap,Qt::DecorationRole);
	setData(index(row,1),data);
	setData(index(row,2),msg);
	//ui.treeView->scrollTo(m_model.index(row,0),QAbstractItemView::PositionAtBottom);
}
