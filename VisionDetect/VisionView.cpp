#include "VisionView.h"
#include <QFileDialog>

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/ICamera.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMath.h>
#include  <QMouseEvent>
#include  <QWheelEvent>
#include <QMessageBox>

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "CryptLib.h"
#include <QFile>

#define ToInt(value)                (static_cast<int>(value))

const int LABEL_IMAGE_WIDTH = 680;
const int LABEL_IMAGE_HEIGHT = 680;

const int MODE_VIEW_NONE = 1;
const int MODE_VIEW_SELECT = 2;
const int MODE_VIEW_MOVE = 3;

VisionView::VisionView(VisionCtrl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QMainWindow(parent)
{
	ui.setupUi(this);
	init();

	m_stateView = MODE_VIEW_SELECT;
	m_windowWidth = LABEL_IMAGE_WIDTH;
	m_windowHeight = LABEL_IMAGE_HEIGHT;
	m_startX = 0;
	m_startY = 0;

	m_imageHeight = 1;
	m_imageWidth = 1;

	m_dScale = 1.0;
	m_dMovedX = 0.0;
	m_dMovedY = 0.0;

	m_mouseLeftPressed = false;
	m_mouseRightPressed = false;

	QRect rect = ui.label_Img->geometry();
	rect.setX(0); rect.setY(0);
	rect.setWidth(LABEL_IMAGE_WIDTH); rect.setHeight(LABEL_IMAGE_HEIGHT);
	ui.label_Img->setGeometry(rect);
	ui.label_Img->setStyleSheet("background-color:black");	
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

	openFolderAct = new QAction(QIcon("image/openFileFolder.png"), QStringLiteral("打开文件夹..."), this);
	openFolderAct->setShortcuts(QKeySequence::Open);
	openFolderAct->setStatusTip(tr("Open file folder"));
	connect(openFolderAct, SIGNAL(triggered()), this, SLOT(openFileFolder()));

	saveAsAct = new QAction(QIcon("image/saveAsFile.png"), QStringLiteral("另存为..."), this);
	saveAsAct->setShortcuts(QKeySequence::SaveAs);
	saveAsAct->setStatusTip(tr("Save the document under a new name"));
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAsFile()));

	zoomInAct = new QAction(QIcon("image/zoomOut.png"), QStringLiteral("缩小"), this);
	zoomInAct->setShortcuts(QKeySequence::ZoomIn);
	zoomInAct->setStatusTip(tr("Zoom in window"));
	connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

	zoomOutAct = new QAction(QIcon("image/zoomIn.png"), QStringLiteral("放大"), this);
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
}

void VisionView::createToolBars()
{
	//! [0]
	ui.mainToolBar->addAction(cameraAct);
	ui.mainToolBar->addAction(openFolderAct);

	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(openAct);	
	fileToolBar->addAction(saveAsAct);

	editToolBar = addToolBar(tr("Edit"));
	editToolBar->addAction(zoomInAct);
	editToolBar->addAction(zoomOutAct);
	editToolBar->addAction(fullScreenAct);
	editToolBar->addAction(moveAct);
}

void VisionView::createStatusBar()
{	
	statusBar()->showMessage(tr("Ready"));	
}

void VisionView::openFile()
{
	QString path = QApplication::applicationDirPath();
	path += "/";

	QString picFilter = "Image(*.tif *.tiff *.gif *.bmp *.jpg *.jpeg *.jp2 *.png *.pcx *.pgm *.ppm *.pbm *.xwd *.ima *.ent)";
	QString strFileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开图片"), "/"/*path*/, picFilter);

	if (!strFileName.isEmpty())
	{
		//if (strFileName.contains(".ent"))
		//{
		//	QString nameDecrypt = strFileName.left(strFileName.indexOf('.')) + ".bmp";
		//	AOI::Crypt::DecryptFileNfg(strFileName.toStdString(), nameDecrypt.toStdString());
		//	strFileName = nameDecrypt;

		//	loadImage(strFileName);

		//	QFile::remove(strFileName);
		//}	
		//else
		{
			loadImage(strFileName);
		}		
	}
}

void VisionView::openFileFolder()
{	
	QString path = QApplication::applicationDirPath();
	path += "/capture/";

	QString filePath = QFileDialog::getExistingDirectory(this, QStringLiteral("打开图片文件夹"), path);

	if (!filePath.isEmpty())
	{
		cv::Mat matGray;
		if (convertToGrayImage(filePath, matGray))
		{			
			setImage(matGray);
		}
	}
}

void VisionView::cameraFile()
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (pCam)
	{
		cv::Mat img;
		if (pCam->grabCamImage(0, &img, true))
		{
			m_hoImage = img;

			m_imageWidth = m_hoImage.size().width;
			m_imageHeight = m_hoImage.size().height;

			m_dScale = 1.0;
			m_dMovedX = 0.0;
			m_dMovedY = 0.0;

			displayImage(m_hoImage);
		}
		else
		{
			QMessageBox::warning(this, "", QStringLiteral("无发抓取图像"));
		}
	}
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

			//QString nameEncrypt = strSave + ".ent";
			//AOI::Crypt::EncryptFileNfg((strSave + ".bmp").toStdString(), nameEncrypt.toStdString());
			//QFile::remove(strSave + ".bmp");
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
	zoomImage(0.8);
}

void VisionView::zoomOut()
{	
	zoomImage(1.2);
}

void VisionView::fullScreen()
{
	fullImage();
}

void VisionView::moveScreen()
{
	setViewState(MODE_VIEW_MOVE);
}

void VisionView::setImage(cv::Mat& matImage)
{
	m_hoImage.release();
	m_hoImage = matImage.clone();
	m_imageWidth = m_hoImage.size().width;
	m_imageHeight = m_hoImage.size().height;

	displayImage(m_hoImage);
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
	cv::putText(image, text, p1, CV_FONT_HERSHEY_COMPLEX, fontScale, Scalar(0, 0, 255), 2);

	// show
	displayImage(image);
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
			break;
		case MODE_VIEW_MOVE:
			moveImage(motionX, motionY);
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

	m_startX = event->x();
	m_startY = event->y();
}

void VisionView::mousePressEvent(QMouseEvent * event)
{
	if (Qt::LeftButton == event->buttons())
	{
		m_mouseLeftPressed = true;
		m_startX = event->x();
		m_startY = event->y();

		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:
			break;
		case MODE_VIEW_NONE:
			break;
		default:
			break;
		}
	}
	else if (Qt::RightButton == event->buttons())
	{
		m_mouseRightPressed = true;
		setViewState(MODE_VIEW_SELECT);
		repaintAll();
	}
}

void VisionView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton)
	{
		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:
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

void VisionView::loadImage(QString& fileName)
{
	m_hoImage = imread(fileName.toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);
	
	m_imageWidth = m_hoImage.size().width;
	m_imageHeight = m_hoImage.size().height;

	m_dScale = 1.0;
	m_dMovedX = 0.0;
	m_dMovedY = 0.0;

	displayImage(m_hoImage);
}

void VisionView::repaintAll()
{

}

void VisionView::A_Transform(Mat& src, Mat& dst, int dx, int dy)
{
	CV_Assert(src.depth() == CV_8U);//CV_Assert（）若括号中的表达式值为false，则返回一个错误信息。  
	const int rows = src.rows;
	const int cols = src.cols;
	dst.create(rows, cols, src.type());

	//dst.row(i).setTo(Scalar(255));
	//dst.col(j).setTo(Scalar(255));

	dst.setTo(Scalar(0, 0, 0));

	Vec3b *p;   //定义一个存放3通道的容器指针p  
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			p = dst.ptr<Vec3b>(i);//指向行数的容器p  
			int x = j - dx;
			int y = i - dy;
			if (x>0 && y>0 && x < cols&&y < rows)//平移后的像素坐标在原图像的行数和列数内  
			{
				p[i, j] = src.ptr<Vec3b>(y)[x];//平移后的图像（i,j)对应于原图像的（y,x)  
			}
		}
	}

}

void VisionView::setViewState(int state)
{
	m_stateView = state;

	switch (m_stateView)
	{
	case MODE_VIEW_SELECT:
		setCursor(Qt::ArrowCursor);
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
	m_dispImage = image;

	QRect rect = ui.label_Img->geometry();
	rect.setX(0); rect.setY(0);
	rect.setWidth(LABEL_IMAGE_WIDTH); rect.setHeight(LABEL_IMAGE_HEIGHT);
	ui.label_Img->setGeometry(rect);

	cv::Mat mat, matMoved;
	double fScaleW = rect.width()*1.0 / image.size().width;
	double fScaleH = rect.height()*1.0 / image.size().height;
	if (!image.empty())
	{
		cv::resize(image, mat, cv::Size(image.size().width*fScaleW*m_dScale, image.size().height*fScaleH*m_dScale), (0, 0), (0, 0), 3);

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
		double scaleOffset = image.size().width*fScaleW*(m_dScale - 1.0) / 2;
		A_Transform(mat, matMoved, m_dMovedX - scaleOffset, m_dMovedY);

		QImage imagePixmap = QImage((uchar*)matMoved.data, matMoved.cols, matMoved.rows, ToInt(matMoved.step), QImage::Format_RGB888);
		ui.label_Img->setPixmap(QPixmap::fromImage(imagePixmap));
	}
}

double VisionView::convertToImgX(double dMouseValue)
{
	QRect rect = ui.label_Img->geometry();
	double fScaleW = m_windowWidth*1.0 / m_imageWidth;
	double fScaleH = m_windowHeight*1.0 / m_imageHeight;

	return (dMouseValue - rect.x()) / fScaleW;
}

double VisionView::convertToImgY(double dMouseValue)
{
	QRect rect = ui.label_Img->geometry();
	double fScaleW = m_windowWidth*1.0 / m_imageWidth;
	double fScaleH = m_windowHeight*1.0 / m_imageHeight;

	return (dMouseValue -  rect.y()) / fScaleH;
}

bool VisionView::convertToGrayImage(QString& szFilePath, cv::Mat &matGray)
{
	//判断路径是否存在
	QDir dir(szFilePath);
	if (!dir.exists())
	{
		return false;
	}
	dir.setFilter(QDir::Files | QDir::NoSymLinks);
	QFileInfoList list = dir.entryInfoList();

	int file_count = list.count();
	if (file_count <= 0)
	{
		return false;
	}

	QVector<cv::Mat> imageMats;
	for (int i = 0; i < file_count; i++)
	{
		QFileInfo file_info = list.at(i);
		QString suffix = file_info.suffix();
		if (QString::compare(suffix, QString("ent"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			QString nameDecrypt = file_info.absolutePath() + "/" + file_info.baseName() + ".bmp";
			AOI::Crypt::DecryptFileNfg(absolute_file_path.toStdString(), nameDecrypt.toStdString());

			cv::Mat mat = cv::imread(nameDecrypt.toStdString(), cv::IMREAD_GRAYSCALE);
			imageMats.push_back(mat);

			QFile::remove(nameDecrypt);
		}
		else if (QString::compare(suffix, QString("bmp"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			cv::Mat mat = cv::imread(absolute_file_path.toStdString(), cv::IMREAD_GRAYSCALE);
			imageMats.push_back(mat);
		}
		else
		{
			return false;
		}
	}

	m_pCtrl->generateGrayImage(imageMats, matGray);

	return true;
}


void VisionView::fullImage()
{
	m_dScale = 1.0;
	m_dMovedX = 0.0;
	m_dMovedY = 0.0;

	if (!m_dispImage.empty())
		displayImage(m_dispImage);
	else
		displayImage(m_hoImage);
}

void VisionView::zoomImage(double scale)
{
	m_dScale *= scale;
	if (m_dScale < 1.0) m_dScale = 1.0;

	if (!m_dispImage.empty())
		displayImage(m_dispImage);
	else
		displayImage(m_hoImage);
}

void VisionView::moveImage(double motionX, double motionY)
{	
	m_dMovedX += motionX;
	m_dMovedY += motionY;

	if (!m_dispImage.empty())
		displayImage(m_dispImage);
	else
		displayImage(m_hoImage);
}
