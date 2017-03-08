#include "visionuiModule.h"
#include "AppMainWidget.h"

QVisionUIModule::QVisionUIModule(int id,const QString &name)
	:QModuleBase(id,name)
{
	int n = 0;
}

QVisionUIModule::~QVisionUIModule()
{

}

void QVisionUIModule::init()
{
	m_mainWidget = new QAppMainWidget;
}

void QVisionUIModule::Show()
{
	QAppMainWidget * p = (QAppMainWidget*)m_mainWidget;
	if(p)
	{
		p->showFullScreen();
	}
}

void QVisionUIModule::unInit()
{
	QAppMainWidget * p = (QAppMainWidget*)m_mainWidget;
	if(p)delete p;
}

void QVisionUIModule::setStateWidget(QWidget * stateWidget)
{
	QAppMainWidget * p = (QAppMainWidget*)m_mainWidget;
	if(p)
	{
		p->setStateWidget(stateWidget);
	}
}

void QVisionUIModule::setCneterwidget(QWidget * centWidget)
{
	QAppMainWidget * p = (QAppMainWidget*)m_mainWidget;
	if(p)
	{
		p->setStateWidget(centWidget);
	}
}

void QVisionUIModule::setTitle(const QString & str,const QString &ver)
{
	QAppMainWidget * p = (QAppMainWidget*)m_mainWidget;
	if(p)
	{
		p->setTitle(str,ver);
	}
}

QMOUDLE_INSTANCE(QVisionUIModule)