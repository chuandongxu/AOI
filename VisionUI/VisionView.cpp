#include "VisionView.h"

#include <QFileDialog>
#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IData.h"
#include "../include/IVision.h"
#include "../include/ICamera.h"
#include "../Common/ThreadPrioc.h"
#include "../Common/eos.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMath.h>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMessageBox>
#include <qdatetime.h>
#include <QtOpenGL/QGLWidget>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPainter>

#include "../DataModule/QDetectObj.h"

#include "../3DViewUtility/DViewUtility.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#define ToInt(value)                (static_cast<int>(value))

const int LABEL_IMAGE_WIDTH = 1200;
const int LABEL_IMAGE_HEIGHT = 800;

/*static*/ const cv::Scalar VisionView::_constRedScalar    (0,   0,   255 );
/*static*/ const cv::Scalar VisionView::_constBlueScalar   (255, 0,   0   );
/*static*/ const cv::Scalar VisionView::_constCyanScalar   (255, 255, 0   );
/*static*/ const cv::Scalar VisionView::_constGreenScalar  (0,   255, 0   );
/*static*/ const cv::Scalar VisionView::_constYellowScalar (0,   255, 255 );

CameraOnLive::CameraOnLive(VisionView* pView)
	:m_pView(pView)
{
	m_bQuit = false;
	m_bRuning = false;
}

void CameraOnLive::run()
{
	m_bRuning = true;

	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return;

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return;

	bool bHardwareTrigger = System->getParam("camera_hw_tri_enable").toBool();
	bool bCaptureImage = System->getParam("camera_cap_image_enable").toBool();	

	cv::Mat image;
	QVector<cv::Mat> imageMats;
	while (!m_bQuit)
	{
		if (bHardwareTrigger)
		{	
			imageMats.clear();

			if (!pCam->startCapturing())
			{
				System->setTrackInfo(QString("startCapturing error"));			
				continue;
			}
			
			int nWaitTime = 5* 60 * 100;
			while (!pCam->isCaptureImageBufferDone() && nWaitTime-- > 0 && !m_bQuit)
			{
				QThread::msleep(10);
			}

			if (nWaitTime <= 0)
			{
				System->setTrackInfo(QString("CaptureImageBufferDone error"));				
				continue;
			}

			if (m_bQuit) break;		
		
			int nCaptureNum = pCam->getImageBufferCaptureNum();		
			for (int i = 0; i < nCaptureNum; i++)
			{			
				cv::Mat matImage = pCam->getImageItemBuffer(i);
				imageMats.push_back(matImage);				
			}

			if (imageMats.size() == 1)
			{
				showImageToScreen(imageMats[0]);
			}
			else if (imageMats.size() > 1)
			{
				if (pVision->generateGrayImage(imageMats, image))
				{
					showImageToScreen(image);					
				}
			}
		
			System->setTrackInfo(QString("System captureImages Image Num: %1").arg(nCaptureNum));		
		
			if (m_bQuit) break;
		}
		else if ((pCam->grabCamImage(0, image, true)))
		{
			try
			{
				if (image.type() == CV_8UC1)
				{
					//input image is grayscale
					cvtColor(image, image, CV_GRAY2RGB);

				}

				drawCross(image);
				if (m_bQuit)break;

				showImageToScreen(image);
				if (m_bQuit)break;
			}
			catch (...)
			{
				break;
			}
		}

		QThread::msleep(10);
		QApplication::processEvents();
	}

	m_bRuning = false;
}

void CameraOnLive::setQuitFlag()
{
	m_bQuit = true;
}

void CameraOnLive::showImageToScreen(cv::Mat& image)
{
	if (m_pView)
	{
		m_pView->displayImage(image);
	}
}

void CameraOnLive::drawCross(cv::Mat& image)
{
	cv::Point startPt, endPt;

	bool bShowCross = System->getParam("camera_capture_cross_enable").toBool();
	int nShowCrossWidth = System->getParam("camera_capture_cross_width").toInt();

	if (bShowCross)
	{
		int nCrossWidth = nShowCrossWidth;

		startPt.x = 0;
		startPt.y = image.size().height / 2 - nCrossWidth / 2;
		endPt.x = image.size().width;
		endPt.y = image.size().height / 2 - nCrossWidth / 2;
		cv::line(image, startPt, endPt, cv::Scalar(255, 255, 0), 1, 8);

		startPt.x = 0;
		startPt.y = image.size().height / 2 + nCrossWidth / 2;
		endPt.x = image.size().width;
		endPt.y = image.size().height / 2 + nCrossWidth / 2;
		cv::line(image, startPt, endPt, cv::Scalar(255, 255, 0), 1, 8);

		startPt.x = image.size().width / 2 - nCrossWidth / 2;
		startPt.y = 0;
		endPt.x = image.size().width / 2 - nCrossWidth / 2;
		endPt.y = image.size().height;
		cv::line(image, startPt, endPt, cv::Scalar(255, 255, 0), 2, 8);

		startPt.x = image.size().width / 2 + nCrossWidth / 2;
		startPt.y = 0;
		endPt.x = image.size().width / 2 + nCrossWidth / 2;
		endPt.y = image.size().height;
		cv::line(image, startPt, endPt, cv::Scalar(255, 255, 0), 2, 8);
	}

	startPt.x = 0;
	startPt.y = image.size().height / 2;
	endPt.x = image.size().width;
	endPt.y = image.size().height / 2;
	cv::line(image, startPt, endPt, cv::Scalar(255, 0, 0), 2, 8);

	startPt.x = image.size().width / 2;
	startPt.y = 0;
	endPt.x = image.size().width / 2;
	endPt.y = image.size().height;
	cv::line(image, startPt, endPt, cv::Scalar(255, 0, 0), 2, 8);
}

VisionView::VisionView(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QEos::Attach(EVENT_RESULT_DISPLAY, this, SLOT(onResultEvent(const QVariantList &)));

	setAcceptDrops(true);

	init();

	m_stateView = MODE_VIEW_NONE;
	m_windowWidth = LABEL_IMAGE_WIDTH;
	m_windowHeight = LABEL_IMAGE_HEIGHT;
	m_startX = 0;
	m_startY = 0;
	m_preMoveX = 0;
	m_preMoveY = 0;

	m_imageHeight = 1;
	m_imageWidth = 1;

	m_dScale = 1.0;
	m_dMovedX = 0.0;
	m_dMovedY = 0.0;

	m_mouseLeftPressed = false;
	m_mouseRightPressed = false;

	m_bShow3DInitial = false;
	m_bMainView3DInitial = false;

	m_pCameraOnLive = NULL;
	onStopAct->setEnabled(false);
}

VisionView::~VisionView()
{
	if (m_pView3D)
	{
		delete m_pView3D;
		m_pView3D = NULL;
	}

	if (m_pMainViewFull3D)
	{
		delete m_pMainViewFull3D;
		m_pMainViewFull3D = NULL;
	}

	if (m_pCameraOnLive) onClickPushbutton_stopLive();
}

void VisionView::init()
{
	createActions();
	createToolBars();
	createStatusBar();

	setUnifiedTitleAndToolBarOnMac(true);

	ui.label_Img->setStyleSheet("background-color:black");
	ui.label_Img->setFixedSize(LABEL_IMAGE_WIDTH, LABEL_IMAGE_HEIGHT);
	ui.label_Img->setMouseTracking(true);

	m_pMainViewFull3D = new DViewUtility();

	m_pView3D = new DViewUtility();// = NULL;

	m_pSelectView = new QDockWidget(this);
	m_pSelectView->setGeometry(QRect(0, 0, 500, 500));
	m_pSelectView->setWidget(m_pView3D->getQGLWidget());
	m_pSelectView->setFeatures(QDockWidget::DockWidgetClosable);
	m_pSelectView->setFloating(false);
	m_pSelectView->setVisible(false);
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

void VisionView::onResultEvent(const QVariantList &data)
{
	int iBoard = data[0].toInt();
	int iEvent = data[1].toInt();
	if (iEvent != STATION_RESULT_IMAGE_DISPLAY)return;

	displayImage(m_hoImage);
}

void VisionView::openFile()
{
	QString path = QApplication::applicationDirPath();
	path += "/";

	QString picFilter = "Image(*.tif *.tiff *.gif *.bmp *.jpg *.jpeg *.jp2 *.png *.pcx *.pgm *.ppm *.pbm *.xwd *.ima)";
	QString strFileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开图片"), path/*/*/, picFilter);

	if (!strFileName.isEmpty())
	{
		loadImage(strFileName);	
	}
}

void VisionView::cameraFile()
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (pCam)
	{
		cv::Mat img;
		if (pCam->grabCamImage(0, img, true))
		{
			m_hoImage = img;

			m_imageWidth = m_hoImage.size().width;
			m_imageHeight = m_hoImage.size().height;

			m_dScale = 1.0;
			m_dMovedX = 0.0;
			m_dMovedY = 0.0;

			displayImage(m_hoImage);		
			return;
		}
	}

	QMessageBox::warning(this, "", QStringLiteral("无发抓取图像"));
}

void VisionView::saveAsFile()
{
	if (!m_dispImage.empty())
	{
		QString picFilter = "Image( *.bmp )";
		QString strSave = QFileDialog::getSaveFileName(this, QStringLiteral("保存图片"), "/", picFilter);
		if (!strSave.isEmpty())
		{
			IplImage frameImg = IplImage(m_dispImage);
			cvSaveImage((strSave + ".bmp").toStdString().c_str(), &frameImg);
		}
		else
		{
			QMessageBox::warning(this, "", QStringLiteral("输入文件名"));
		}
	}
	else
	{
		QMessageBox::warning(this, "", QStringLiteral("无图像"));
	}
}

void VisionView::zoomIn()
{
	zoomImage(_constZoomInStep);
}

void VisionView::zoomOut()
{
	zoomImage(_constZoomOutStep);
}

void VisionView::fullScreen()
{
	fullImage();
}

void VisionView::moveScreen()
{
	setViewState(MODE_VIEW_MOVE);
}

void VisionView::onClickPushbutton_onLive()
{
	QAutoLocker loacker(&m_mutex);

	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return;


	int nCaptureMode = System->getParam("camera_capture_mode").toInt();
	if (2 == nCaptureMode)
	{
		QMessageBox::warning(this, "", QStringLiteral("手动采集模式不支持实时图像显示"));
		return;
	}

	bool bHardwareTrigger = System->getParam("camera_hw_tri_enable").toBool();
	bool bCaptureImage = System->getParam("camera_cap_image_enable").toBool();
	if (!bHardwareTrigger && bCaptureImage)
	{
		QMessageBox::warning(this, "", QStringLiteral("实时图像不支持【非硬触发】保存图像，取消保存图像或者启用硬触发模式"));
		return;
	}	

	QString capturePath = System->getParam("camera_cap_image_path").toString();
	if (bHardwareTrigger && bCaptureImage)
	{
		QDir dirDefaultPath;
		dirDefaultPath.setPath(capturePath);

		if (!dirDefaultPath.exists())
		{			
			QMessageBox::warning(this, "", QStringLiteral("Default Capturing path is not exit! Please browser to set the path!"));
			return;
		}
	}

	if (pCam->getCameraNum() > 0)
	{
		if (!pCam->startUpCapture())
		{
			QSystem::closeMessage();
			QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("相机初始化问题。"));
			return;
		}
	}
	else
	{
		QSystem::closeMessage();
		QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("请检查相机是否连接。"));
		return;
	}

	if (!m_pCameraOnLive)
	{
		setButtonsEnable(false, true);
		onLiveAct->setEnabled(false);
		onStopAct->setEnabled(true);
		clearImage();

		m_pCameraOnLive = new CameraOnLive(this);
		m_pCameraOnLive->start();
	}
}

void VisionView::onClickPushbutton_stopLive()
{
	QAutoLocker loacker(&m_mutex);

	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return;

	if (pCam->getCameraNum() > 0)
	{
		pCam->endUpCapture();
	}
	else
	{
		QSystem::closeMessage();
		QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("请检查相机是否连接。"));
	}

	if (m_pCameraOnLive)
	{
		onLiveAct->setEnabled(true);
		onStopAct->setEnabled(false);

		m_pCameraOnLive->setQuitFlag();
		while (m_pCameraOnLive->isRuning())
		{
			QThread::msleep(10);
			QApplication::processEvents();
		}
		QThread::msleep(200);

		delete m_pCameraOnLive;
		m_pCameraOnLive = NULL;

		setButtonsEnable(true, true);
	}
}

void VisionView::show3D()
{
	if (m_pMainViewFull3D->isShown())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("3D界面已显示，请关闭后再打开"));
		return;
	}

	if (!m_bMainView3DInitial)
	{
		QVector<double> xValues, yValues, zValues;
		for (int i = 0; i < 100; i++)
		{
			int col = i % 10 + 1;
			int row = i / 10 + 1;

			xValues.push_back(col);
			yValues.push_back(row);
			zValues.push_back(0);
		}

		m_pMainViewFull3D->previousROIDisplay();
		m_pMainViewFull3D->loadFile(false, 10, 10, xValues, yValues, zValues);
		m_pMainViewFull3D->prepareROIDisplay(QImage(), false);
		//m_pMainViewFull3D->show();
		//m_pMainViewFull3D->hide();

		QApplication::processEvents();

		m_bMainView3DInitial = true;
	}

	m_pMainViewFull3D->hide();

	QApplication::processEvents();

	if (!m_3DMatHeight.empty())
	{
		//auto vecMatNewPhase = matToVector<float>(m_3DMatHeight);
		cv::Mat mat3DHeight = m_3DMatHeight.clone();
		cv::patchNaNs(mat3DHeight, 0);

		int nSizeY = mat3DHeight.rows;
		int nSizeX = mat3DHeight.cols;

		int nDataNum = nSizeX * nSizeY;

		double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();

		QVector<double> xValues, yValues, zValues;
		for (int i = 0; i < nDataNum; i++)
		{
			int col = i%nSizeY + 1;
			int row = i / nSizeY + 1;

			xValues.push_back(col - nSizeY / 2);
			yValues.push_back(row - nSizeX / 2);

			zValues.push_back(mat3DHeight.at<float>(col - 1, row - 1) * 1000 / dResolutionX);
		}

		m_pMainViewFull3D->show();
		m_pMainViewFull3D->loadFile(false, nSizeY, nSizeX, xValues, yValues, zValues);
		m_pMainViewFull3D->show();
		m_pMainViewFull3D->changeToMesh();

		m_selectROI.width = 0;
		m_selectROI.height = 0;
		m_pSelectView->setVisible(false);

		m_bShow3DInitial = false;
	}
}

void VisionView::showSelectROI3D()
{
	setViewState(MODE_VIEW_SELECT);
}

void VisionView::setImage(const cv::Mat& matImage, bool bDisplay)
{
    m_hoImage = matImage.clone();
    m_imageWidth = m_hoImage.size().width;
    m_imageHeight = m_hoImage.size().height;
    if (bDisplay)
    {
        zoomImage(0.25);
        repaintAll();
    }
}

cv::Mat VisionView::getImage()
{
	return m_hoImage;
}

void VisionView::clearImage()
{
	if (m_hoImage.empty()) return;
	m_hoImage.release();
	m_dispImage.release();
}

void VisionView::addImageText(QString szText)
{
	if (m_hoImage.empty()) return;

	const int ImageWidth = 2048;
	double dScaleFactor = (double)m_imageWidth / ImageWidth;

	cv::Point p1;
	p1.x = 10;
	p1.y = 80 * dScaleFactor;

	cv::String text;

	text = szText.toStdString();

	cv::Mat image = m_hoImage.clone();
	double fontScale = dScaleFactor*2.0f;
	cv::putText(image, text, p1, CV_FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(0, 0, 255), 2);

	// show
	displayImage(image);
}

void VisionView::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			//event->accept();
		}
		else {
			event->acceptProposedAction();
		}
	}
	else {
		event->ignore();
	}
}

void VisionView::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {

		QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
		QDataStream dataStream(&itemData, QIODevice::ReadOnly);

		QPixmap pixmap;
		QPoint offset;
		QString objectName;
		dataStream >> pixmap >> offset >> objectName;

		//qDebug() << objectName;

		QStringList indexLists = objectName.split('-');

		if (indexLists.size() >= 2)
		{
			displayAllObjs();
		}

		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		}
		else {
			event->acceptProposedAction();
		}
	}
	else {
		event->ignore();
	}
}

void VisionView::dropEvent(QDropEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
		QDataStream dataStream(&itemData, QIODevice::ReadOnly);

		QPixmap pixmap;
		QPoint offset;
		QString objectName;
		dataStream >> pixmap >> offset >> objectName;

		//qDebug() << objectName;

		QStringList indexLists = objectName.split('-');

		if (indexLists.size() >= 2)
		{
			addNodeByDrag(indexLists.at(0).toInt(), indexLists.at(1).toInt(), event->pos() - offset);
		}

		/*	QLabel *newIcon = new QLabel(this);
		newIcon->setPixmap(pixmap);
		newIcon->move(event->pos() - offset);
		newIcon->show();
		newIcon->setAttribute(Qt::WA_DeleteOnClose);*/

		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		}
		else {
			event->acceptProposedAction();
		}
	}
	else {
		event->ignore();
	}
}


void VisionView::mouseMoveEvent(QMouseEvent * event)
{
	double mouseX = event->x(), mouseY = event->y();

	double motionX = 0, motionY = 0;
	motionX = mouseX - m_startX;
	motionY = mouseY - m_startY;

	if (m_mouseLeftPressed)
	{
		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:
		case MODE_VIEW_SELECT_ROI:
			if ((0 != (int)motionX) || (0 != (int)motionY))
			{
				if ((int)motionX >= 0 && (int)motionY >= 0)
				{
					cv::Rect select;
					select.x = convertToImgX(m_startX);
					select.y = convertToImgY(m_startY);
					select.width = convertToImgX(motionX, true);
					select.height = convertToImgY(motionY, true);
					//cv::rectangle(matImage, select, Scalar(0, 255, 0), 1, 8, 0);

					if (select.x < 0 || select.y < 0 || (select.x + select.width) > m_hoImage.size().width || (select.y + select.height) > m_hoImage.size().height)
					{
						if (select.x < 0)
						{
							select.x = 0;
						}
						if (select.y < 0)
						{
							select.y = 0;
						}
						select.width = (select.x + select.width) > m_hoImage.size().width ? (m_hoImage.size().width - select.x) : select.width;
						select.height = (select.y + select.height) > m_hoImage.size().height ? (m_hoImage.size().height - select.y) : select.height;
					}

					if (MODE_VIEW_SELECT == m_stateView && select.width > 500) select.width = 500;
					if (MODE_VIEW_SELECT == m_stateView && select.height > 500) select.height = 500;

					m_selectROI = select;

					//qDebug() << m_selectROI.x << ":" << m_selectROI.y << ":" << m_selectROI.width << ":" << m_selectROI.height;

					repaintAll();
				}
			}
			break;
		case MODE_VIEW_MOVE:
			moveImage(mouseX - m_preMoveX, mouseY - m_preMoveY);
			break;
        case MODE_VIEW_SET_FIDUCIAL_MARK:
            m_szCadOffset.width  += ( mouseX - m_preMoveX ) / m_dScale;
            m_szCadOffset.height += ( mouseY - m_preMoveY ) / m_dScale;
            repaintAll();
            break;
		case MODE_VIEW_NONE:
			break;
		default:
			break;
		}
	}
	else if (m_mouseRightPressed)
	{
	}
	else
	{
	}

	m_preMoveX = event->x();
	m_preMoveY = event->y();
}

void VisionView::mousePressEvent(QMouseEvent * event)
{
	if (Qt::LeftButton == event->buttons())
	{
		m_mouseLeftPressed = true;
		m_startX = event->x();
		m_startY = event->y();
		m_preMoveX = m_startX;
		m_preMoveY = m_startY;

		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:
		case MODE_VIEW_SELECT_ROI:
			break;
		case MODE_VIEW_NONE:
			break;            
		default:
			break;
		}
	}
	else if (Qt::RightButton == event->buttons())
	{
		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:					
			break;
		case MODE_VIEW_SELECT_ROI:		
			break;
		case MODE_VIEW_NONE:
			break;
		default:
			break;
		}
		m_selectROI.width = 0;
		m_selectROI.height = 0;
		m_pSelectView->setVisible(false);
		m_mouseRightPressed = true;
		setViewState(MODE_VIEW_NONE);
		repaintAll();
	}
}

void VisionView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton)
	{
        const QPoint pos = event->pos();
        auto rectCentralWidget = ui.centralWidget->geometry();
        const QPoint posOnImageLabel ( pos.x() - rectCentralWidget.x(), pos.y() - rectCentralWidget.y() );
		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:
			if (m_selectROI.width > 0 && m_selectROI.height > 0)
			{
				show3DView(m_selectROI);
			}
			m_selectROI.width = 0;
			m_selectROI.height = 0;
			setViewState(MODE_VIEW_NONE);
			break;
		case MODE_VIEW_SELECT_ROI:	
			setViewState(MODE_VIEW_NONE);
			break;
        case MODE_VIEW_SET_FIDUCIAL_MARK:            
            _checkSelectedDevice( cv::Point ( posOnImageLabel.x(), posOnImageLabel.y() ) );
            break;
		case MODE_VIEW_NONE:
			break;
		default:
			break;
		}
	}
	else if (event->button() & Qt::RightButton)
	{
	}

	//repaintAll();
	m_mouseLeftPressed = false;
	m_mouseRightPressed = false;
}

void VisionView::wheelEvent(QWheelEvent * event)
{
	switch (m_stateView)
	{
	case MODE_VIEW_NONE:
		break;
	default:
		break;
	}
}

void VisionView::addNodeByDrag(int nType, int nObjID, QPoint ptPos)
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	for (int i = 0; i < pData->getObjNum(EM_DATA_TYPE_TMP); i++)
	{
		QDetectObj* pObj = pData->getObj(i, EM_DATA_TYPE_TMP);
		if (pObj && pObj->getID() == nObjID)
		{
			QDetectObj* pNewObj = new QDetectObj(*pObj);
			QString newName = pNewObj->getName() + QString("%1").arg(pData->getObjNum(EM_DATA_TYPE_OBJ) + 1);
			pNewObj->setName(newName);
			pNewObj->setObjPos(convertToImgX(ptPos.x() - ui.label_Img->pos().x(), true), convertToImgY(ptPos.y() - ui.label_Img->pos().y(), true));
			pData->pushObj(pNewObj, EM_DATA_TYPE_OBJ);
			//pData->saveDataBase(QString(""), EM_DATA_TYPE_OBJ);

			QEos::Notify(EVENT_OBJ_STATE, 0, RUN_OBJ_CHANGE);

			displayAllObjs();

			break;
		}
	}
}

void VisionView::displayAllObjs()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	cv::Mat matImage = m_hoImage.clone();

	if (matImage.type() == CV_8UC1)
	{
		cvtColor(matImage, matImage, CV_GRAY2RGB);
	}

	bool bShowNumber = false;
	for (int i = 0; i < pData->getObjNum(EM_DATA_TYPE_OBJ); i++)
	{
		QDetectObj* pObj = pData->getObj(i, EM_DATA_TYPE_OBJ);
		if (pObj)
		{
			cv::Point2f vertices[4];
			pObj->getFrame().points(vertices);

			if (true)
			{
				const int ImageWidth = 2048;
				double dScaleFactor = (double)m_imageWidth / ImageWidth;

				cv::Point p1;
				p1.x = pObj->getFrame().center.x - pObj->getFrame().size.width;
				p1.y = pObj->getFrame().center.y;

				cv::String text = QString("%1").arg(i + 1).toStdString();

				double fontScale = dScaleFactor*3.0f;
				cv::putText(matImage, text, p1, CV_FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(0, 255, 255), 2);
			}

			for (int i = 0; i < 4; i++)
			{
				line(matImage, vertices[i], vertices[(i + 1) % 4], cv::Scalar(128, 255, 255), 5);
			}

			pObj->getLoc().points(vertices);

			for (int i = 0; i < 4; i++)
			{
				line(matImage, vertices[i], vertices[(i + 1) % 4], cv::Scalar(255, 255, 0), 5);
			}

			for (int j = 0; j < pObj->getHeightBaseNum(); j++)
			{
				pObj->getHeightBase(j).points(vertices);

				if (bShowNumber)
				{
					const int ImageWidth = 2048;
					double dScaleFactor = (double)m_imageWidth / ImageWidth;

					cv::Point p1;
					p1.x = pObj->getHeightBase(j).center.x;
					p1.y = pObj->getHeightBase(j).center.y;

					cv::String text = QString("%1").arg(j + 1).toStdString();

					double fontScale = dScaleFactor*2.0f;
					cv::putText(matImage, text, p1, CV_FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(0, 0, 255), 2);
				}

				for (int i = 0; i < 4; i++)
				{
					line(matImage, vertices[i], vertices[(i + 1) % 4], cv::Scalar(255, 0, 0), 5);
				}
			}

			for (int j = 0; j < pObj->getHeightDetectNum(); j++)
			{
				pObj->getHeightDetect(j).points(vertices);

				if (bShowNumber)
				{
					const int ImageWidth = 2048;
					double dScaleFactor = (double)m_imageWidth / ImageWidth;

					cv::Point p1;
					p1.x = pObj->getHeightDetect(j).center.x;
					p1.y = pObj->getHeightDetect(j).center.y;

					cv::String text = QString("%1").arg(j + 1).toStdString();

					double fontScale = dScaleFactor*2.0f;
					cv::putText(matImage, text, p1, CV_FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(0, 0, 255), 2);
				}

				for (int i = 0; i < 4; i++)
				{
					line(matImage, vertices[i], vertices[(i + 1) % 4], cv::Scalar(0, 255, 0), 5);
				}
			}
		}
	}

	displayImage(matImage);
}

void VisionView::loadImage(QString& fileName)
{
	m_hoImage = cv::imread(fileName.toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);

	m_imageWidth = m_hoImage.size().width;
	m_imageHeight = m_hoImage.size().height;

	m_dScale = 1.0;
	m_dMovedX = 0.0;
	m_dMovedY = 0.0;

	displayImage(m_hoImage);
}

void VisionView::repaintAll()
{
	cv::Mat matImage = m_hoImage.clone();

	if (m_selectROI.width > 0 && m_selectROI.height > 0)
	{
		cv::Mat matRect = matImage(m_selectROI);
		//rectangle(matRect, vertices[1], vertices[3], Scalar(0,0,255, 100), -1);
		cv::Mat imgLayer(m_selectROI.height, m_selectROI.width, matImage.type()/*CV_8UC3*/, cv::Scalar(255, 128, 0));

		double alpha = 0.3;
		addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);

		cv::rectangle(matImage, m_selectROI, cv::Scalar(128, 64, 0), 1, 8, 0);
	}

	displayImage(matImage);
}

void VisionView::A_Transform(cv::Mat& src, cv::Mat& dst, int dx, int dy)
{
	CV_Assert(src.depth() == CV_8U);//CV_Assert（）若括号中的表达式值为false，则返回一个错误信息。  
	const int rows = src.rows;
	const int cols = src.cols;
	dst.create(rows, cols, src.type());

	//dst.row(i).setTo(Scalar(255));
	//dst.col(j).setTo(Scalar(255));

	dst.setTo(cv::Scalar(0, 0, 0));

	cv::Vec3b *p;   //定义一个存放3通道的容器指针p  
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			p = dst.ptr<cv::Vec3b>(i);//指向行数的容器p  
			int x = j - dx;
			int y = i - dy;
			if (x>0 && y>0 && x < cols&&y < rows)//平移后的像素坐标在原图像的行数和列数内  
			{
				p[i, j] = src.ptr<cv::Vec3b>(y)[x];//平移后的图像（i,j)对应于原图像的（y,x)  
			}
		}
	}
}

void VisionView::setViewState(VISION_VIEW_MODE state)
{
	m_stateView = state;

	switch (m_stateView)
	{
	case MODE_VIEW_SELECT:
		setCursor(Qt::CrossCursor);
		break;
	case MODE_VIEW_SELECT_ROI:
		setCursor(Qt::CrossCursor);
		break;
	case MODE_VIEW_MOVE:
		setCursor(Qt::OpenHandCursor);
		break;
	case MODE_VIEW_NONE:
		setCursor(Qt::ArrowCursor);
		break;
	default:
		setCursor(Qt::ArrowCursor);
		break;
	}
}

void VisionView::displayImage(cv::Mat& image)
{
    if (image.empty())
        return;

    m_dispImage = image;
    _calcMoveRange();

    cv::Mat matDisplay;
    _drawDeviceWindows( m_dispImage );
    _cutImageForDisplay ( m_dispImage, matDisplay );
	if (matDisplay.type() == CV_8UC3)
	{
		cvtColor(matDisplay, matDisplay, CV_BGR2RGB);
	}
	else if (matDisplay.type() == CV_8UC1)
	{
		cvtColor(matDisplay, matDisplay, CV_GRAY2RGB);
	}
    QImage imagePixmap = QImage((uchar*)matDisplay.data, matDisplay.cols, matDisplay.rows, ToInt(matDisplay.step), QImage::Format_RGB888);
    ui.label_Img->setPixmap(QPixmap::fromImage(imagePixmap));
}

void VisionView::load3DViewData(int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues)
{
	if (m_pView3D)
	{
		m_pView3D->loadFile(false, nSizeX, nSizeY, xValues, yValues, zValues);
	}
}

void VisionView::show3DView()
{
	if (m_pView3D)
	{
		m_pView3D->show();
	}
}

void VisionView::setSelect()
{
	setViewState(MODE_VIEW_SELECT_ROI);
}

cv::Mat VisionView::getSelectImage()
{
	return (m_selectROI.size().width <= 5 || m_selectROI.size().height <= 5) ? m_hoImage : m_hoImage(m_selectROI);
}

void VisionView::clearSelect()
{
	m_selectROI.x = 0;
	m_selectROI.y = 0;
	m_selectROI.width = 0;
	m_selectROI.height = 0;
}

cv::Rect2f VisionView::getSelectScale()
{
	//cv::Rect2f scale;
	//scale.x = m_selectROI.x;
	//scale.y = m_selectROI.y;
	//scale.width = m_selectROI.width / m_imageWidth;
	//scale.height = m_selectROI.height / m_imageHeight;

	return m_selectROI;
}

void VisionView::displayObjs(QVector<QDetectObj*> objs, bool bShowNumber)
{
	cv::Mat matImage = m_hoImage.clone();	

	for (int i = 0; i < objs.size(); i++)
	{
		QDetectObj* pObj = objs[i];
		if (pObj)
		{
			cv::Point2f vertices[4];
			pObj->getFrame().points(vertices);

			for (int i = 0; i < 4; i++)
			{
				line(matImage, vertices[i], vertices[(i + 1) % 4], cv::Scalar(128, 255, 255), 5);
			}

			pObj->getLoc().points(vertices);

			for (int i = 0; i < 4; i++)
			{
				line(matImage, vertices[i], vertices[(i + 1) % 4], cv::Scalar(255, 255, 0), 5);
			}

			for (int j = 0; j < pObj->getHeightBaseNum(); j++)
			{
				pObj->getHeightBase(j).points(vertices);

				if (bShowNumber)
				{
					const int ImageWidth = 2048;
					double dScaleFactor = (double)m_imageWidth / ImageWidth;

					cv::Point p1;
					p1.x = pObj->getHeightBase(j).center.x;
					p1.y = pObj->getHeightBase(j).center.y;

					cv::String text = QString("%1").arg(j + 1).toStdString();

					double fontScale = dScaleFactor*2.0f;
					cv::putText(matImage, text, p1, CV_FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(0, 0, 255), 2);
				}

				for (int i = 0; i < 4; i++)
				{
					line(matImage, vertices[i], vertices[(i + 1) % 4], cv::Scalar(255, 0, 0), 5);
				}
			}

			for (int j = 0; j < pObj->getHeightDetectNum(); j++)
			{
				pObj->getHeightDetect(j).points(vertices);

				if (bShowNumber)
				{
					const int ImageWidth = 2048;
					double dScaleFactor = (double)m_imageWidth / ImageWidth;

					cv::Point p1;
					p1.x = pObj->getHeightDetect(j).center.x;
					p1.y = pObj->getHeightDetect(j).center.y;

					cv::String text = QString("%1").arg(j + 1).toStdString();

					double fontScale = dScaleFactor*2.0f;
					cv::putText(matImage, text, p1, CV_FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(0, 0, 255), 2);
				}

				for (int i = 0; i < 4; i++)
				{
					line(matImage, vertices[i], vertices[(i + 1) % 4], cv::Scalar(0, 255, 0), 5);
				}
			}
		}
	}

	displayImage(matImage);
}

void VisionView::setDeviceWindows(const QVector<cv::RotatedRect> &vecWindows)
{
    m_vecDeviceWindows = vecWindows;
    //Reset the device window offset.
    m_szCadOffset.width = 0;
    m_szCadOffset.height = 0;
    repaintAll();
}

void VisionView::setSelectedFM(const QVector<cv::RotatedRect> &vecWindows)
{
    m_vecSelectedFM = vecWindows;
    repaintAll();
}

void VisionView::getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) const {
    rrectImageWindow = rrectCadWindow = m_selectedDevice;
    rrectImageWindow.center.x += m_szCadOffset.width;
    rrectImageWindow.center.y += m_szCadOffset.height;
}

double VisionView::convertToImgX(double dMouseValue, bool bLen)
{
	QRect rect = ui.label_Img->geometry();
	double fScaleW = m_windowWidth*1.0  / m_imageWidth;
	double fScaleH = m_windowHeight*1.0 / m_imageHeight;

	double scaleOffset = m_windowWidth*(m_dScale - 1.0) / 2;

	if (bLen)
	{
		return (dMouseValue) / fScaleW / m_dScale;
	}
	else
	{
		return (dMouseValue - rect.x() + scaleOffset - m_dMovedX) / fScaleW / m_dScale;
	}
}

double VisionView::convertToImgY(double dMouseValue, bool bLen)
{
	QRect rect = ui.label_Img->geometry();
	double fScaleW = m_windowWidth*1.0 / m_imageWidth;
	double fScaleH = m_windowHeight*1.0 / m_imageHeight;

	double scaleOffset = m_windowHeight*(m_dScale - 1.0) / 2;

	if (bLen)
	{
		return dMouseValue / fScaleH / m_dScale;
	}
	else
	{
		return (dMouseValue - (rect.y() + 40) + scaleOffset - m_dMovedY) / fScaleH / m_dScale;
	}
}

double VisionView::convertToMouseX(double dImgValue, bool bLen)
{
	QRect rect = ui.label_Img->geometry();
	double fScaleW = m_windowWidth*1.0 / m_imageWidth;
	double fScaleH = m_windowHeight*1.0 / m_imageHeight;

	double scaleOffset = m_windowWidth*(m_dScale - 1.0) / 2;

	if (bLen)
	{
		return (dImgValue)* fScaleW * m_dScale;
	}
	else
	{
		return dImgValue * fScaleW * m_dScale + rect.x() - scaleOffset + m_dMovedX;
	}
}

double VisionView::convertToMouseY(double dImgValue, bool bLen)
{
	QRect rect = ui.label_Img->geometry();
	double fScaleW = m_windowWidth*1.0 / m_imageWidth;
	double fScaleH = m_windowHeight*1.0 / m_imageHeight;

	double scaleOffset = m_windowHeight*(m_dScale - 1.0) / 2;

	if (bLen)
	{
		return dImgValue * fScaleH * m_dScale;
	}
	else
	{
		return dImgValue * fScaleH * m_dScale + (rect.y() + 40) - scaleOffset + m_dMovedY;
	}
}

void VisionView::show3DView(cv::Rect& rectROI)
{
	if (!m_bShow3DInitial)
	{
		QVector<double> xValues, yValues, zValues;
		for (int i = 0; i < 100; i++)
		{
			int col = i % 10 + 1;
			int row = i / 10 + 1;

			xValues.push_back(col);
			yValues.push_back(row);
			zValues.push_back(0);
		}

		m_pView3D->previousROIDisplay();
		m_pView3D->loadFile(false, 10, 10, xValues, yValues, zValues);
		m_pView3D->prepareROIDisplay(QImage(), false);
		m_pSelectView->setGeometry(QRect(0, 0, 10, 10));
		m_pSelectView->setVisible(true);

		QApplication::processEvents();

		m_bShow3DInitial = true;
	}

	if (!m_3DMatHeight.empty() && rectROI.width > 10 && rectROI.height > 10)
	{
		//rectROI.width = rectROI.height;
		cv::Mat matHeight = m_3DMatHeight(rectROI);

		cv::Mat matMask = matHeight == matHeight;

		int nSizeY = matHeight.rows;
		int nSizeX = matHeight.cols;

		int nDataNum = nSizeX * nSizeY;

		double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();

		QVector<double> xValues, yValues, zValues;
		for (int i = 0; i < nDataNum; i++)
		{
			int col = i%nSizeY + 1;
			int row = i / nSizeY + 1;

			xValues.push_back(col - nSizeY / 2);
			yValues.push_back(row - nSizeX / 2);

			if (matMask.at<bool>(col - 1, row - 1))
			{
				zValues.push_back(matHeight.at<float>(col - 1, row - 1) * 1000 / dResolutionX);
			}
			else
			{
				zValues.push_back(0);
			}
		}

		m_pView3D->previousROIDisplay();
		m_pView3D->loadFile(true, nSizeY, nSizeX, xValues, yValues, zValues);

		cv::Mat matTexture = m_hoImage(rectROI);
		/*	cv::Mat matTextureTranspose;
		cv::transpose(matTexture, matTextureTranspose);
		cv::Mat matTextureFlip;
		cv::flip(matTextureTranspose, matTextureFlip, nSizeY/2);
		QImage imageTexture = QImage((uchar*)matTextureFlip.data, matTextureFlip.cols, matTextureFlip.rows, ToInt(matTextureFlip.step), QImage::Format_RGB888);*/
		QImage imageTexture = QImage((uchar*)matTexture.data, matTexture.cols, matTexture.rows, ToInt(matTexture.step), QImage::Format_RGB888);
		//QMatrix matrix;
		//matrix.rotate(270);
		//QImage imageTextureTranspose = imageTexture.transformed(matrix);
		m_pView3D->prepareROIDisplay(imageTexture, true);

		QRect rectDisplay;
		rectDisplay.setX(convertToMouseX(rectROI.x));
		rectDisplay.setY(convertToMouseY(rectROI.y));
		rectDisplay.setWidth(convertToMouseX(rectROI.width, true));
		rectDisplay.setHeight(convertToMouseY(rectROI.height, true));

		const int DISP_SIZE = 350;
		if (rectDisplay.width() < DISP_SIZE)
		{
			rectDisplay.setX(rectDisplay.x() - (DISP_SIZE - rectDisplay.width()) / 2);
			rectDisplay.setWidth(DISP_SIZE);
		}
		if (rectDisplay.height() < DISP_SIZE)
		{
			rectDisplay.setY(rectDisplay.y() - (DISP_SIZE - rectDisplay.height()) / 2);
			rectDisplay.setHeight(DISP_SIZE);
		}

		m_pSelectView->setGeometry(rectDisplay);
		m_pSelectView->setVisible(true);
		m_bMainView3DInitial = false;
	}
}

void VisionView::fullImage()
{
	m_dScale = 1.0;
	m_dMovedX = 0.0;
	m_dMovedY = 0.0;

	repaintAll();
}

void VisionView::zoomImage(double scale)
{
    if ( m_dScale >= _constMaxZoomScale && scale > 1. )
        return;

    if ( m_dScale <= _constMinZoomScale && scale < 1. )
        return;

	m_dScale *= scale;
	repaintAll();
}

void VisionView::moveImage(double motionX, double motionY)
{
	m_dMovedX += motionX;
	m_dMovedY += motionY;

    if ( m_dMovedX > _szMoveRange.width )
        m_dMovedX = _szMoveRange.width;
    else if ( m_dMovedX < -_szMoveRange.width )
        m_dMovedX = - _szMoveRange.width;

    if ( m_dMovedY > _szMoveRange.height )
        m_dMovedY = _szMoveRange.height;
    else if ( m_dMovedY < -_szMoveRange.height )
        m_dMovedY = - _szMoveRange.height;

    repaintAll();
}

void VisionView::setButtonsEnable(bool flag, bool bLiveVideo)
{
	if (bLiveVideo)
	{
		openAct->setEnabled(flag);
		cameraAct->setEnabled(flag);
		saveAsAct->setEnabled(flag);
		zoomInAct->setEnabled(flag);
		zoomOutAct->setEnabled(flag);
		fullScreenAct->setEnabled(flag);
		moveAct->setEnabled(flag);
		show3DAct->setEnabled(flag);
		//onLiveAct->setEnabled(flag);
		//onStopAct->setEnabled(flag);
	}
	else
	{
		openAct->setEnabled(flag);
		cameraAct->setEnabled(flag);
		saveAsAct->setEnabled(flag);
		//zoomInAct->setEnabled(flag);
		//zoomOutAct->setEnabled(flag);
		//fullScreenAct->setEnabled(flag);
		//moveAct->setEnabled(flag);
		//show3DAct->setEnabled(flag);
		onLiveAct->setEnabled(flag);
		//onStopAct->setEnabled(flag);
	}
}

bool VisionView::startUpCapture()
{
	if (m_pCameraOnLive) onClickPushbutton_stopLive();

	setButtonsEnable(false, false);
	fullImage();

	return true;
}

bool VisionView::endUpCapture()
{
	if (m_pCameraOnLive) onClickPushbutton_stopLive();

	setButtonsEnable(true, false);

	return true;
}

void VisionView::setHeightData(cv::Mat& matHeight)
{
	m_3DMatHeight = matHeight;
}

void VisionView::_zoomImageForDisplay(const cv::Mat &matImg, cv::Mat &matOutput) {
    QRect rect = ui.label_Img->geometry();
	rect.setX(0); rect.setY(0);
	rect.setWidth(LABEL_IMAGE_WIDTH); rect.setHeight(LABEL_IMAGE_HEIGHT);
	ui.label_Img->setGeometry(rect);

	cv::Mat mat;
	double fScaleW = rect.width()*1.0 / matImg.size().width;
	double fScaleH = rect.height()*1.0 / matImg.size().height;
	if (!matImg.empty())
	{
		cv::resize(matImg, mat, cv::Size(matImg.size().width*fScaleW*m_dScale, matImg.size().height*fScaleH*m_dScale), (0, 0), (0, 0), 3);

		if (mat.type() == CV_8UC3)
		{
			cvtColor(mat, mat, CV_BGR2RGB);
		}
		else if (mat.type() == CV_8UC1)
		{
			cvtColor(mat, mat, CV_GRAY2RGB);
		}
		/*	else if (mat.type() == CV_32F)
		{
		cvtColor(mat, mat, CV_HSV2RGB);
		}*/
		//qDebug() << "mat type " << mat.type();
		double scaleOffset = matImg.size().width*fScaleW*(m_dScale - 1.0) / 2;
		A_Transform ( mat, matOutput, m_dMovedX - scaleOffset, m_dMovedY );
	}
}

void VisionView::_cutImageForDisplay(const cv::Mat &matInputImg, cv::Mat &matOutput)
{
    if ( matInputImg.empty() )
        return;

    QRect rect = ui.label_Img->geometry();
    auto displayWidth  = rect.width();
    auto displayHeight = rect.height();
    matOutput = cv::Mat::ones( displayHeight, displayWidth, matInputImg.type() ) * 255;
    matOutput.setTo(cv::Scalar(255, 255, 255));

    cv::Mat matZoomResult;
    cv::resize ( matInputImg, matZoomResult, cv::Size(), m_dScale, m_dScale );

    if (matZoomResult.cols >= displayWidth && matZoomResult.rows >= displayHeight) {
        cv::Rect rectROISrc( (matZoomResult.cols - displayWidth) / 2 - m_dMovedX, (matZoomResult.rows - displayHeight) / 2 - m_dMovedY, displayWidth, displayHeight);
        cv::Mat matSrc( matZoomResult, rectROISrc);
        matSrc.copyTo ( matOutput );
    }
    else if (matZoomResult.cols >= displayWidth && matZoomResult.rows <= displayHeight) {
        cv::Rect rectROISrc( (matZoomResult.cols - displayWidth) / 2 - m_dMovedX, 0, displayWidth, matZoomResult.rows);
        cv::Mat matSrc(matZoomResult, rectROISrc);
        cv::Rect rectROIDst(0, ( displayHeight - matZoomResult.rows ) / 2, displayWidth, matZoomResult.rows );
        cv::Mat matDst(matOutput, rectROIDst);
        matSrc.copyTo(matDst);
    }else if (matZoomResult.cols <= displayWidth && matZoomResult.rows >= displayHeight) {
        cv::Rect rectROISrc(0, (matZoomResult.rows - displayHeight) / 2 - m_dMovedY, matZoomResult.cols, displayHeight);
        cv::Mat matSrc(matZoomResult, rectROISrc);
        cv::Rect rectROIDst(( displayWidth - matZoomResult.cols ) / 2, 0, matZoomResult.cols, displayHeight );
        cv::Mat matDst(matOutput, rectROIDst);
        matSrc.copyTo(matDst);
    }else if (matZoomResult.cols <= displayWidth && matZoomResult.rows <= displayHeight) {
        cv::Rect rectROIDst((displayWidth - matZoomResult.cols) / 2, (displayHeight - matZoomResult.rows) / 2, matZoomResult.cols, matZoomResult.rows);
        cv::Mat matDst(matOutput, rectROIDst);
        matZoomResult.copyTo(matDst);
    }
    else
    {
        QMessageBox::critical(nullptr, "Display", "This should not happen, please contact software engineer!", "Quit");
    }
}

static VectorOfPoint getCornerOfRotatedRect (const cv::RotatedRect &rotatedRect)
{
    VectorOfPoint vecPoint;
    cv::Point2f arrPt[4];
    rotatedRect.points(arrPt);
    for (int i = 0; i < 4; ++i)
        vecPoint.push_back(arrPt[i]);
    return vecPoint;
}

void VisionView::_drawDeviceWindows(cv::Mat &matImg)
{
    auto getCornerOfRotatedRect = [](const cv::RotatedRect &rotatedRect) {
        VectorOfPoint vecPoint;
        cv::Point2f arrPt[4];
        rotatedRect.points( arrPt );
        for ( int i = 0; i < 4; ++ i )
            vecPoint.push_back ( arrPt[i] );
        return vecPoint;
    };
    VectorOfVectorOfPoint vecContours;
    cv::Point ptCtrOfImage ( 0, 0 );
    ptCtrOfImage.x += m_szCadOffset.width;
    ptCtrOfImage.y += m_szCadOffset.height;
    vecContours.reserve ( m_vecDeviceWindows.size() );

    if ( ! m_vecDeviceWindows.empty() ) {
        for ( auto rotatedRect : m_vecDeviceWindows ) {
            rotatedRect.center.x += ptCtrOfImage.x;
            rotatedRect.center.y += ptCtrOfImage.y;
            auto contour = getCornerOfRotatedRect ( rotatedRect );
            contour.push_back ( contour.front() );
            vecContours.push_back ( contour );
        }
        cv::polylines ( matImg, vecContours, true, _constBlueScalar, _constDeviceWindowLineWidth );
    }

    if ( ! m_vecSelectedFM.empty() ) {
        vecContours.clear();
        for ( auto rotatedRect : m_vecSelectedFM ) {
            rotatedRect.center.x += ptCtrOfImage.x;
            rotatedRect.center.y += ptCtrOfImage.y;
            auto contour = getCornerOfRotatedRect ( rotatedRect );
            contour.push_back ( contour.front() );
            vecContours.push_back ( contour );
        }
        cv::polylines ( matImg, vecContours, true, _constGreenScalar, _constDeviceWindowLineWidth );
    }

    auto localSelectedDevice ( m_selectedDevice );
    localSelectedDevice.center.x += ptCtrOfImage.x;
    localSelectedDevice.center.y += ptCtrOfImage.y;
    auto contour = getCornerOfRotatedRect ( localSelectedDevice );
    contour.push_back ( contour.front() );
    cv::polylines ( matImg, VectorOfVectorOfPoint ( 1, contour ), true, _constCyanScalar, _constDeviceWindowLineWidth );
}

void VisionView::_calcMoveRange()
{
    if ( m_dispImage.empty() ) {
        _szMoveRange = cv::Size(0, 0);
        return;
    }

    int rows = m_dispImage.rows * m_dScale;
    int cols = m_dispImage.cols * m_dScale;
    auto displayWidth = this->size().width();
    auto displayHeight = this->size().height();

    _szMoveRange.width = ( cols - displayWidth ) / 2;
    if ( _szMoveRange.width < 0 ) _szMoveRange.width = 0;

    _szMoveRange.height = ( rows - displayHeight ) / 2;
    if ( _szMoveRange.height < 0 ) _szMoveRange.height = 0;

    if ( m_dMovedX > _szMoveRange.width )
        m_dMovedX = _szMoveRange.width;
    else if ( m_dMovedX < -_szMoveRange.width )
        m_dMovedX = - _szMoveRange.width;

    if ( m_dMovedY > _szMoveRange.height )
        m_dMovedY = _szMoveRange.height;
    else if ( m_dMovedY < -_szMoveRange.height )
        m_dMovedY = - _szMoveRange.height;
}

void VisionView::_checkSelectedDevice(const cv::Point &ptMousePos) {
    const auto COLS = m_hoImage.cols;
    const auto ROWS = m_hoImage.rows;
    cv::Point ptOnImage;
    ptOnImage.x = ( ptMousePos.x - m_dMovedX - ( LABEL_IMAGE_WIDTH  - COLS * m_dScale ) / 2 ) / m_dScale;
    ptOnImage.y = ( ptMousePos.y - m_dMovedY - ( LABEL_IMAGE_HEIGHT - ROWS * m_dScale ) / 2 ) / m_dScale;
    bool bFoundDevice = false;
    for ( const auto &rotatedRect : m_vecDeviceWindows ) {
        auto localRotateRect ( rotatedRect );
        localRotateRect.center.x += m_szCadOffset.width;
        localRotateRect.center.y += m_szCadOffset.height;
        auto contour = getCornerOfRotatedRect ( localRotateRect );
        contour.push_back ( contour.front() );
        auto distance = cv::pointPolygonTest ( contour, ptOnImage, false );
        if ( distance >= 0 ) {
            bFoundDevice = true;
            m_selectedDevice = rotatedRect;
            break;
        }
    }
    if ( bFoundDevice )
        repaintAll();
}