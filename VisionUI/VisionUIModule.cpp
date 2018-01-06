#include "visionuiModule.h"
#include "AppMainWidget.h"

#include "QMainView.h"

QVisionUIModule::QVisionUIModule(int id,const QString &name)
	:QModuleBase(id,name)
{
	int n = 0;

	m_pMainView = new QMainView();
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

QWidget* QVisionUIModule::getMainView()
{
	return m_pMainView;
}

void QVisionUIModule::setImage(cv::Mat& matImage, bool bDisplay)
{
	if (m_pMainView)
	{
		m_pMainView->setImage(matImage, bDisplay);
	}
}

void QVisionUIModule::setHeightData(cv::Mat& matHeight)
{
	if (m_pMainView)
	{
		m_pMainView->setHeightData(matHeight);
	}
}

bool QVisionUIModule::startUpCapture()
{
	if (m_pMainView)
	{
		return m_pMainView->startUpCapture();
	}

	return false;
}

bool QVisionUIModule::endUpCapture()
{
	if (m_pMainView)
	{
		return m_pMainView->endUpCapture();
	}

	return false;
}

cv::Mat QVisionUIModule::getImage()
{
	if (m_pMainView)
	{
		return m_pMainView->getImage();
	}

	return cv::Mat();
}

void QVisionUIModule::clearImage()
{
	if (m_pMainView)
	{
		m_pMainView->clearImage();
	}
}

void QVisionUIModule::addImageText(QString szText)
{
	if (m_pMainView)
	{
		m_pMainView->addImageText(szText);
	}
}

void QVisionUIModule::displayImage(cv::Mat& image)
{
	if (m_pMainView)
	{
		m_pMainView->displayImage(image);
	}
}

QMOUDLE_INSTANCE(QVisionUIModule)