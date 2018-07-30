#include "VisionView.h"

#include <QFileDialog>
#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/ILight.h"
#include "../include/IVision.h"
#include "../include/IMotion.h"
#include "../Common/eos.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMath.h>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>

#include "VisionViewWidget.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define ToInt(value)                (static_cast<int>(value))

VisionView::VisionView(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	init();

	setButtonsEnable(true);
	setLiveButtonEnable(true);
}

VisionView::~VisionView() {
}

void VisionView::init()
{
	createActions();
	createToolBars();
	createStatusBar();

	setUnifiedTitleAndToolBarOnMac(true);	

	m_pViewWidget = new VisionViewWidget(this);
	this->setCentralWidget(m_pViewWidget);

	m_pLightWidget = NULL;
	ILight* pLight = getModule<ILight>(LIGHT_MODEL);
	if (pLight)
	{
		m_pLightWidget = pLight->getLightWidget();
	}

	m_pColorWidget = NULL;
	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (pVision)
	{
		m_pColorWidget = pVision->getColorWeightView();
	}
}

void VisionView::createActions()
{
	cameraAct = new QAction(QIcon("image/cameraFile.png"), QStringLiteral("抓取"), this);
	cameraAct->setShortcuts(QKeySequence::Open);
	cameraAct->setStatusTip(tr("Grab file from camera"));
	connect(cameraAct, SIGNAL(triggered()), this, SLOT(cameraFile()));

	openAct = new QAction(QIcon("image/openFile.png"), QStringLiteral("打开..."), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

	saveAsAct = new QAction(QIcon("image/saveAsFile.png"), QStringLiteral("另存为..."), this);
	saveAsAct->setShortcuts(QKeySequence::SaveAs);
	saveAsAct->setStatusTip(tr("Save the document under a new name"));
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAsFile()));

	m_pZoomInAct = std::make_unique<QAction>(QIcon("image/zoomIn.png"), QStringLiteral("放大"), this);
	m_pZoomInAct->setShortcuts(QKeySequence::ZoomIn);
	m_pZoomInAct->setStatusTip(tr("Zoom in window"));
	connect(m_pZoomInAct.get(), SIGNAL(triggered()), this, SLOT(zoomIn()));

	m_pZoomOutAct = std::make_unique<QAction>(QIcon("image/zoomOut.png"), QStringLiteral("缩小"), this);
	m_pZoomOutAct->setShortcuts(QKeySequence::ZoomOut);
	m_pZoomOutAct->setStatusTip(tr("Zoom out window"));
	connect(m_pZoomOutAct.get(), SIGNAL(triggered()), this, SLOT(zoomOut()));

	m_pFullScreenAct = std::make_unique<QAction>(QIcon("image/fullScreen.png"), QStringLiteral("全屏"), this);
	m_pFullScreenAct->setShortcuts(QKeySequence::FullScreen);
	m_pFullScreenAct->setStatusTip(tr("Full screen"));
	connect(m_pFullScreenAct.get(), SIGNAL(triggered()), this, SLOT(fullScreen()));

	m_pMoveAct = std::make_unique<QAction>(QIcon("image/moveView.png"), QStringLiteral("移动"), this);
	m_pMoveAct->setShortcuts(QKeySequence::MoveToEndOfBlock);
	m_pMoveAct->setStatusTip(tr("Move screen"));
	connect(m_pMoveAct.get(), SIGNAL(triggered()), this, SLOT(moveScreen()));

	onLiveAct = new QAction(QIcon("image/onLive.png"), QStringLiteral("实时图像"), this);
	onLiveAct->setShortcuts(QKeySequence::MoveToStartOfDocument);
	onLiveAct->setStatusTip(tr("Live Video"));
	connect(onLiveAct, SIGNAL(triggered()), this, SLOT(onClickPushbutton_onLive()));

	onStopAct = new QAction(QIcon("image/onStop.png"), QStringLiteral("停止采集"), this);
	onStopAct->setShortcuts(QKeySequence::MoveToEndOfDocument);
	onStopAct->setStatusTip(tr("Stop Video"));
	connect(onStopAct, SIGNAL(triggered()), this, SLOT(onClickPushbutton_stopLive()));

	show3DAct = new QAction(QIcon("image/cutSurface.png"), QStringLiteral("显示三维数据"), this);
	show3DAct->setShortcuts(QKeySequence::Underline);
	show3DAct->setStatusTip(tr("Show 3D"));
	connect(show3DAct, SIGNAL(triggered()), this, SLOT(show3D()));

    m_pSelectROI = std::make_unique<QAction>(QIcon("image/SelectROI.png"), tr("&Select ROI"), this);
    m_pSelectROI->setShortcut(QKeySequence::Bold);
    connect(m_pSelectROI.get(), SIGNAL(triggered()), this, SLOT(selectROI()));
    addAction(m_pSelectROI.get());

	m_pSelect3DROI = std::make_unique<QAction>(QIcon("image/cutSurface.png"), tr("&Select 3D"), this);
	m_pSelect3DROI->setShortcuts(QKeySequence::Italic);
	m_pSelect3DROI->setStatusTip(tr("Show Select 3D Data"));
	connect(m_pSelect3DROI.get(), SIGNAL(triggered()), this, SLOT(showSelectROI3D()));
	addAction(m_pSelect3DROI.get());

    m_pSelectInspectROI = std::make_unique<QAction>(QIcon("image/profImage.png"), tr("&Select Inspect"), this);
    m_pSelectInspectROI->setShortcuts(QKeySequence::Underline);
    m_pSelectInspectROI->setStatusTip(tr("Show Select Inspect Data"));
    connect(m_pSelectInspectROI.get(), SIGNAL(triggered()), this, SLOT(showInspectROI()));
    addAction(m_pSelectInspectROI.get());

    m_pCopy = std::make_unique<QAction>(QIcon("image/copy.jpg"), tr("&Copy Device"), this);
    m_pCopy->setShortcuts(QKeySequence::Copy);
    m_pCopy->setStatusTip(tr("Copy to clipboard"));
    connect(m_pCopy.get(), SIGNAL(triggered()), this, SLOT(copyDeviceWindow()));
    addAction(m_pCopy.get());

    m_pPaste = std::make_unique<QAction>(QIcon("image/paste.jpg"), tr("&Paste Device"), this);
    m_pPaste->setShortcuts(QKeySequence::Paste);
    m_pPaste->setStatusTip(tr("Paste from clipboard"));
    connect(m_pPaste.get(), SIGNAL(triggered()), this, SLOT(pasteDevice()));
    addAction(m_pPaste.get());

	setContextMenuPolicy(Qt::ActionsContextMenu);

	showLightAct = new QAction(QIcon("image/busy.png"), QStringLiteral("显示灯光设置"), this);
	showLightAct->setShortcuts(QKeySequence::MoveToNextChar);
	showLightAct->setStatusTip(tr("Show Lighting"));
	connect(showLightAct, SIGNAL(triggered()), this, SLOT(showLight()));

	showColorSpaceAct = new QAction(QIcon("image/colorSpace.png"), QStringLiteral("显示颜色框设置"), this);
	showColorSpaceAct->setShortcuts(QKeySequence::MoveToPreviousChar);
	showColorSpaceAct->setStatusTip(tr("Show Color Space"));
	connect(showColorSpaceAct, SIGNAL(triggered()), this, SLOT(showColorSpace()));

	showJoystick = new QAction(QIcon("image/joystick.png"), QStringLiteral("移动马达"), this);
	showJoystick->setShortcuts(QKeySequence::MoveToNextWord);
	showJoystick->setStatusTip(tr("Show Joystick Widget"));
	connect(showJoystick, SIGNAL(triggered()), this, SLOT(showJoystickWidget()));
}

void VisionView::createToolBars()
{
	//! [0]
	ui.mainToolBar->addAction(cameraAct);

	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAsAct);

	editToolBar = addToolBar(tr("Edit"));
	editToolBar->addAction(m_pZoomInAct.get());
	editToolBar->addAction(m_pZoomOutAct.get());
	editToolBar->addAction(m_pFullScreenAct.get());
	editToolBar->addAction(m_pMoveAct.get());

	videoToolBar = addToolBar(tr("Video"));
	videoToolBar->addAction(onLiveAct);
	videoToolBar->addAction(onStopAct);

	detectToolBar = addToolBar(tr("Detect"));
	detectToolBar->addAction(show3DAct);
	detectToolBar->addAction(showLightAct);
	detectToolBar->addAction(showColorSpaceAct);
	detectToolBar->addAction(showJoystick);
}

void VisionView::createStatusBar()
{
	//statusBar()->showMessage(tr("Ready"));
	//statusBar()->hide();
	//statusBar()->setSizeGripEnabled(false);
}

void VisionView::openFile()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->openFile();
	}
}

void VisionView::cameraFile()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->cameraFile();
	}
}

void VisionView::saveAsFile()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->saveAsFile();
	}
}

void VisionView::zoomIn()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->zoomIn();
	}
}

void VisionView::zoomOut()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->zoomOut();
	}
}

void VisionView::fullScreen()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->fullScreen();
	}
}

void VisionView::moveScreen()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->moveScreen();
	}
}

void VisionView::onClickPushbutton_onLive()
{
	onLive(true);
}

void VisionView::onClickPushbutton_stopLive()
{
	onStopLive();
}

void VisionView::show3D()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->show3D();
	}
}

void VisionView::selectROI()
{
    m_pViewWidget->setViewState(MODE_VIEW_SELECT_ROI);
}

void VisionView::showSelectROI3D()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->showSelectROI3D();
	}
}

void VisionView::showInspectROI()
{
    if (m_pViewWidget)
    {
        m_pViewWidget->showInspectROI();
    }
}

void VisionView::copyDeviceWindow()
{
    if (m_pViewWidget)
    {
        m_pViewWidget->copyDeviceWindow();
    }
}

void VisionView::pasteDevice()
{
    if (m_pViewWidget)
    {
        m_pViewWidget->pasteDevice();
    }
}

void VisionView::setImage(const cv::Mat& matImage, bool bDisplay)
{
	if (m_pViewWidget)
	{
		m_pViewWidget->setImage(matImage, bDisplay);
	}
}

cv::Mat VisionView::getImage()
{
	if (m_pViewWidget)
	{
		return m_pViewWidget->getImage();
	}
	return cv::Mat();
}

void VisionView::clearImage()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->clearImage();
	}
}

void VisionView::addImageText(QString szText)
{
	if (m_pViewWidget)
	{
		m_pViewWidget->addImageText(szText);
	}
}

void VisionView::setViewState(VISION_VIEW_MODE state)
{
	if (m_pViewWidget)
	{
		m_pViewWidget->setViewState(state);
	}
}

void VisionView::displayImage(cv::Mat& image)
{
	if (m_pViewWidget)
	{
		m_pViewWidget->displayImage(image);
	}
}

void VisionView::load3DViewData(int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues)
{
	if (m_pViewWidget)
	{
		m_pViewWidget->load3DViewData(nSizeX, nSizeY, xValues, yValues, zValues);
	}
}

void VisionView::show3DView()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->show();
	}
}

void VisionView::showLight()
{
	if (m_pLightWidget)
	{
        m_pLightWidget->setWindowFlags(Qt::WindowStaysOnTopHint);
		m_pLightWidget->show();
	}
}

void VisionView::showColorSpace()
{
	if (m_pColorWidget)
	{
		m_pColorWidget->show();
	}
}

void VisionView::showJoystickWidget()
{
	IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (pMotion)
	{
		pMotion->setJoystickXMotor(AXIS_MOTOR_X, 0.1, NULL);
		pMotion->setJoystickYMotor(AXIS_MOTOR_Y, 0.1, NULL);

		pMotion->startJoystick();
	}
}

cv::Mat VisionView::getSelectImage()
{
	if (m_pViewWidget)
	{
		return m_pViewWidget->getSelectImage();
	}

	return cv::Mat();
}

void VisionView::clearSelect()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->clearSelect();
	}
}

cv::Rect2f VisionView::getSelectedROI()
{
	if (m_pViewWidget)
	{
		return m_pViewWidget->getSelectedROI();
	}

	return cv::Rect2f();
}

void VisionView::displayObjs(QVector<QDetectObj*> objs, bool bShowNumber)
{
	if (m_pViewWidget)
	{
		m_pViewWidget->displayObjs(objs, bShowNumber);
	}
}

void VisionView::setDetectObjs(const QVector<QDetectObj> &vecDetectObjs)
{
    m_pViewWidget->setDetectObjs(vecDetectObjs);
}

void VisionView::setCurrentDetectObj(const QDetectObj &detectObj)
{
    m_pViewWidget->setCurrentDetectObj(detectObj);
}

QVector<QDetectObj> VisionView::getDetectObjs() const
{
    return m_pViewWidget->getDetectObjs();
}

void VisionView::setDeviceWindows(const VisionViewDeviceVector &vecWindows)
{
    m_pViewWidget->setDeviceWindows(vecWindows);
}

void VisionView::getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) const {
	if (m_pViewWidget)
	{
		m_pViewWidget->getSelectDeviceWindow(rrectCadWindow, rrectImageWindow);
	}
}

VisionViewDevice VisionView::getSelectedDevice() const
{
    return m_pViewWidget->getSelectedDevice();
}

void VisionView::setConfirmedFM(const VisionViewFMVector &vecFM) {
    m_pViewWidget->setConfirmedFM(vecFM);
}

void VisionView::setCurrentFM(const VisionViewFM &fm) {
    m_pViewWidget->setCurrentFM(fm);
}

VisionViewFM VisionView::getCurrentFM() const {
    return m_pViewWidget->getCurrentFM();
}

void VisionView::setButtonsEnable(bool flag)
{
	openAct->setEnabled(flag);
	cameraAct->setEnabled(flag);
	saveAsAct->setEnabled(flag);

	m_pZoomInAct->setEnabled(flag);
	m_pZoomOutAct->setEnabled(flag);
	m_pFullScreenAct->setEnabled(flag);
	m_pMoveAct->setEnabled(flag);

	show3DAct->setEnabled(flag);
	//showLightAct->setEnabled(flag);
}

void VisionView::setLiveButtonEnable(bool flag)
{
	onLiveAct->setEnabled(flag);
	onStopAct->setEnabled(!flag);
}

bool VisionView::onLive(bool bPromptSelect)
{
	if (m_pViewWidget && !m_pViewWidget->isLiving())
	{
		setLiveButtonEnable(false);
		if (m_pViewWidget->onLive(bPromptSelect))
		{
			setButtonsEnable(false);
		}
		else
		{
			setLiveButtonEnable(true);
		}
	}

	return true;
}

void VisionView::onStopLive()
{
	if (m_pViewWidget && m_pViewWidget->isLiving())
	{
		setLiveButtonEnable(true);
		m_pViewWidget->onStopLive();
		setButtonsEnable(true);
	}
}

bool VisionView::startUpCapture(bool bPromptSelect)
{
	if (m_pViewWidget && m_pViewWidget->isLiving())
	{
		onStopLive();
	}	
	onLive(bPromptSelect);
	return true;
}

bool VisionView::endUpCapture()
{
	if (m_pViewWidget && m_pViewWidget->isLiving())
	{
		onStopLive();
	}

	return true;
}

void VisionView::setHeightData(const cv::Mat& matHeight)
{
	if (m_pViewWidget)
	{
		m_pViewWidget->setHeightData(matHeight);
	}
}

cv::Mat VisionView::getHeightData() const
{
	if (m_pViewWidget)
	{
		return m_pViewWidget->getHeightData();
	}
	return cv::Mat();
}