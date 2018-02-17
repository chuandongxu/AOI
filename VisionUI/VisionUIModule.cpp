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
        p->setWindowIcon(QIcon(":/VisionUI/AppIcon.png"));
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

void QVisionUIModule::setImage(const cv::Mat& matImage, bool bDisplay)
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

void QVisionUIModule::load3DViewData(int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues)
{
	if (m_pVisionView)
	{
		m_pVisionView->load3DViewData(nSizeX, nSizeY, xValues, yValues, zValues);
	}
}

void QVisionUIModule::show3DView()
{
	if (m_pVisionView)
	{
		m_pVisionView->show3DView();
	}
}

cv::Mat QVisionUIModule::getSelectImage()
{
	if (m_pVisionView)
	{
		return m_pVisionView->getSelectImage();
	}

	return cv::Mat();
}

void QVisionUIModule::clearSelect()
{
	if (m_pVisionView)
	{
		m_pVisionView->clearSelect();
	}
}

cv::Rect2f QVisionUIModule::getSelectedROI()
{
	if (m_pVisionView)
	{
		return m_pVisionView->getSelectedROI();
	}

	return cv::Rect2f();
}

void QVisionUIModule::displayObjs(QVector<QDetectObj*> objs, bool bShowNumber)
{
	if (m_pVisionView)
	{
		m_pVisionView->displayObjs(objs, bShowNumber);
	}
}

void QVisionUIModule::setDetectObjs(const QVector<QDetectObj> &vecDetectObjs)
{
    m_pVisionView->setDetectObjs ( vecDetectObjs );
}

QVector<QDetectObj> QVisionUIModule::getDetectObjs() const
{
    return m_pVisionView->getDetectObjs();
}

void QVisionUIModule::setDeviceWindows(const QVector<cv::RotatedRect> &vecWindows) {
    if (m_pVisionView)
	{
		m_pVisionView->setDeviceWindows(vecWindows);
	}
}

void QVisionUIModule::setSelectedFM(const QVector<cv::RotatedRect> &vecWindows) {
    if (m_pVisionView)
	{
		m_pVisionView->setSelectedFM(vecWindows);
	}
}

void QVisionUIModule::setViewState(VISION_VIEW_MODE enViewMode) {
    if (m_pVisionView)
	{
		m_pVisionView->setViewState(enViewMode);
	}
}

void QVisionUIModule::getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) {
    if (m_pVisionView)
	{
		m_pVisionView->getSelectDeviceWindow(rrectCadWindow, rrectImageWindow);
	}
}

QMOUDLE_INSTANCE(QVisionUIModule)