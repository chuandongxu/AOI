#include "VisionView.h"

#include <QFileDialog>
#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
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

VisionView::~VisionView()
{	
}

void VisionView::init()
{
	createActions();
	createToolBars();
	createStatusBar();

	setUnifiedTitleAndToolBarOnMac(true);	

	m_pViewWidget = new VisionViewWidget(this);

	this->setCentralWidget(m_pViewWidget);
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

	zoomInAct = new QAction(QIcon("image/zoomIn.png"), QStringLiteral("放大"), this);
	zoomInAct->setShortcuts(QKeySequence::ZoomIn);
	zoomInAct->setStatusTip(tr("Zoom in window"));
	connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

	zoomOutAct = new QAction(QIcon("image/zoomOut.png"), QStringLiteral("缩小"), this);
	zoomOutAct->setShortcuts(QKeySequence::ZoomOut);
	zoomOutAct->setStatusTip(tr("Zoom out window"));
	connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

	fullScreenAct = new QAction(QIcon("image/fullScreen.png"), QStringLiteral("全屏"), this);
	fullScreenAct->setShortcuts(QKeySequence::FullScreen);
	fullScreenAct->setStatusTip(tr("Full screen"));
	connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(fullScreen()));

	moveAct = new QAction(QIcon("image/moveView.png"), QStringLiteral("移动"), this);
	moveAct->setShortcuts(QKeySequence::MoveToEndOfBlock);
	moveAct->setStatusTip(tr("Move screen"));
	connect(moveAct, SIGNAL(triggered()), this, SLOT(moveScreen()));

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

	selectROI = new QAction(QIcon("cutSurface.png"), tr("&Select 3D"), this);
	selectROI->setShortcuts(QKeySequence::Italic);
	selectROI->setStatusTip(tr("Show Select 3D Data"));
	connect(selectROI, SIGNAL(triggered()), this, SLOT(showSelectROI3D()));
	addAction(selectROI);
	setContextMenuPolicy(Qt::ActionsContextMenu);
}

void VisionView::createToolBars()
{
	//! [0]
	ui.mainToolBar->addAction(cameraAct);

	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAsAct);

	editToolBar = addToolBar(tr("Edit"));
	editToolBar->addAction(zoomInAct);
	editToolBar->addAction(zoomOutAct);
	editToolBar->addAction(fullScreenAct);
	editToolBar->addAction(moveAct);

	videoToolBar = addToolBar(tr("Video"));
	videoToolBar->addAction(onLiveAct);
	videoToolBar->addAction(onStopAct);

	detectToolBar = addToolBar(tr("Detect"));
	detectToolBar->addAction(show3DAct);
}

void VisionView::createStatusBar()
{
	//statusBar()->showMessage(tr("Ready"));
	statusBar()->hide();
	statusBar()->setSizeGripEnabled(false);
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
	if (m_pViewWidget && !m_pViewWidget->isLiving())
	{
		setLiveButtonEnable(false);
		if (m_pViewWidget->onLive())
		{
			setButtonsEnable(false);
		}
		else
		{
			setLiveButtonEnable(true);
		}		
	}
}

void VisionView::onClickPushbutton_stopLive()
{
	if (m_pViewWidget && m_pViewWidget->isLiving())
	{
		setLiveButtonEnable(true);
		m_pViewWidget->onStopLive();
		setButtonsEnable(true);
	}
}

void VisionView::show3D()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->show3D();
	}
}

void VisionView::showSelectROI3D()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->showSelectROI3D();
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

void VisionView::setSelect()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->setSelect();
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

cv::Rect2f VisionView::getSelectScale()
{
	if (m_pViewWidget)
	{
		return m_pViewWidget->getSelectScale();
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

void VisionView::setDeviceWindows(const QVector<cv::RotatedRect> &vecWindows)
{
	if (m_pViewWidget)
	{
		m_pViewWidget->setDeviceWindows(vecWindows);
	}
}

void VisionView::setSelectedFM(const QVector<cv::RotatedRect> &vecWindows)
{
	if (m_pViewWidget)
	{
		m_pViewWidget->setSelectedFM(vecWindows);
	}
}

void VisionView::getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) const {
	if (m_pViewWidget)
	{
		m_pViewWidget->getSelectDeviceWindow(rrectCadWindow, rrectImageWindow);
	}
}

void VisionView::setButtonsEnable(bool flag)
{
	openAct->setEnabled(flag);
	cameraAct->setEnabled(flag);
	saveAsAct->setEnabled(flag);
	zoomInAct->setEnabled(flag);
	zoomOutAct->setEnabled(flag);
	fullScreenAct->setEnabled(flag);
	moveAct->setEnabled(flag);
	show3DAct->setEnabled(flag);
}

void VisionView::setLiveButtonEnable(bool flag)
{
	onLiveAct->setEnabled(flag);
	onStopAct->setEnabled(!flag);
}

bool VisionView::startUpCapture()
{
	if (m_pViewWidget && m_pViewWidget->isLiving())
	{
		onClickPushbutton_stopLive();
	}

	onClickPushbutton_onLive();
	return true;
}

bool VisionView::endUpCapture()
{
	if (m_pViewWidget && m_pViewWidget->isLiving())
	{
		onClickPushbutton_stopLive();		
	}

	return true;
}

void VisionView::setHeightData(cv::Mat& matHeight)
{
	if (m_pViewWidget)
	{
		m_pViewWidget->setHeightData(matHeight);
	}
}