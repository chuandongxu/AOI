#include "visionuiModule.h"
#include "AppMainWidget.h"

#include "VisionView.h"

QVisionUIModule::QVisionUIModule(int id,const QString &name)
	:QModuleBase(id,name)
{
	int n = 0;

	m_pVisionView = new VisionView();
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

QWidget* QVisionUIModule::getVisionView()
{
	return m_pVisionView;
}

void QVisionUIModule::setImage(cv::Mat& matImage, bool bDisplay)
{
	if (m_pVisionView)
	{
		m_pVisionView->setImage(matImage, bDisplay);
	}
}

void QVisionUIModule::setHeightData(cv::Mat& matHeight)
{
	if (m_pVisionView)
	{
		m_pVisionView->setHeightData(matHeight);
	}
}

bool QVisionUIModule::startUpCapture()
{
	if (m_pVisionView)
	{
		return m_pVisionView->startUpCapture();
	}

	return false;
}

bool QVisionUIModule::endUpCapture()
{
	if (m_pVisionView)
	{
		return m_pVisionView->endUpCapture();
	}

	return false;
}

cv::Mat QVisionUIModule::getImage()
{
	if (m_pVisionView)
	{
		return m_pVisionView->getImage();
	}

	return cv::Mat();
}

void QVisionUIModule::clearImage()
{
	if (m_pVisionView)
	{
		m_pVisionView->clearImage();
	}
}

void QVisionUIModule::addImageText(QString szText)
{
	if (m_pVisionView)
	{
		m_pVisionView->addImageText(szText);
	}
}

void QVisionUIModule::displayImage(cv::Mat& image)
{
	if (m_pVisionView)
	{
		m_pVisionView->displayImage(image);
	}
}

QMOUDLE_INSTANCE(QVisionUIModule)