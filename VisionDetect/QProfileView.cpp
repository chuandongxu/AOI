#include "QProfileView.h"

#include <QFileDialog>

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IData.h"
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

#define ToInt(value)                (static_cast<int>(value))

const int LABEL_IMAGE_WIDTH = 800;
const int LABEL_IMAGE_HEIGHT = 400;

const int MODE_VIEW_NONE = 1;
const int MODE_VIEW_SELECT = 2;
const int MODE_VIEW_MOVE = 3;
const int MODE_VIEW_DRAG = 4;

QProfileView::QProfileView(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::Dialog);
	setWindowModality(Qt::ApplicationModal);

	init();

	m_stateView = MODE_VIEW_NONE;
	m_windowWidth = LABEL_IMAGE_WIDTH;
	m_windowHeight = LABEL_IMAGE_HEIGHT;

	ui.label_Img->setFixedSize(LABEL_IMAGE_WIDTH, LABEL_IMAGE_HEIGHT);
	ui.label_Img->setMouseTracking(true);

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

	m_bSelected = false;

	m_selectROI.width = 0;
	m_measRangeCurr.width = 0;

	QRect rect = ui.label_Img->geometry();
	rect.setX(0); rect.setY(0);
	rect.setWidth(LABEL_IMAGE_WIDTH); rect.setHeight(LABEL_IMAGE_HEIGHT);
	ui.label_Img->setGeometry(rect);
	ui.label_Img->setStyleSheet("background-color:black");

	connect(ui.pushButton_StartCapture, SIGNAL(clicked()), SLOT(onStart()));
	connect(ui.pushButton_StopCapture, SIGNAL(clicked()), SLOT(onReset()));
	connect(ui.pushButton_CaptureDone, SIGNAL(clicked()), SLOT(onDone()));

	setViewState(MODE_VIEW_DRAG);

	openAct->setEnabled(false);
	saveAsAct->setEnabled(false);
}

QProfileView::~QProfileView()
{
}

void QProfileView::closeEvent(QCloseEvent *e){
	//qDebug() << "关闭事件";
	//e->ignore();

	this->hide();
}

void QProfileView::init()
{
	createActions();
	createToolBars();
	createStatusBar();

	setUnifiedTitleAndToolBarOnMac(true);
}

void QProfileView::createActions()
{
	profileAct = new QAction(QIcon("image/showMesh.png"), QStringLiteral("抓取Profile"), this);
	profileAct->setShortcuts(QKeySequence::Open);
	profileAct->setStatusTip(tr("Grab file from camera"));
	connect(profileAct, SIGNAL(triggered()), this, SLOT(profileFile()));

	openAct = new QAction(QIcon("image/openFile.png"), QStringLiteral("打开..."), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

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

void QProfileView::createToolBars()
{
	//! [0]
	//ui.mainToolBar->addAction(profileAct);

	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAsAct);

	editToolBar = addToolBar(tr("Edit"));
	editToolBar->addAction(zoomInAct);
	editToolBar->addAction(zoomOutAct);
	editToolBar->addAction(fullScreenAct);
	editToolBar->addAction(moveAct);
}

void QProfileView::createStatusBar()
{
	statusBar()->hide();
	statusBar()->setSizeGripEnabled(false);
}

void QProfileView::openFile()
{
	QString path = QApplication::applicationDirPath();
	path += "/";

	QString picFilter = "Image(*.tif *.tiff *.gif *.bmp *.jpg *.jpeg *.jp2 *.png *.pcx *.pgm *.ppm *.pbm *.xwd *.ima)";
	QString strFileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开图片"), "/"/*path*/, picFilter);

	if (!strFileName.isEmpty())
	{
		loadImage(strFileName);
	}
}

void QProfileView::profileFile()
{
	
}

void QProfileView::saveAsFile()
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

void QProfileView::zoomIn()
{
	zoomImage(0.8);
}

void QProfileView::zoomOut()
{
	zoomImage(1.2);
}

void QProfileView::fullScreen()
{
	fullImage();
}

void QProfileView::moveScreen()
{
	setViewState(MODE_VIEW_MOVE);
}

void QProfileView::setSelect()
{
	m_bSelected = false;
	setViewState(MODE_VIEW_SELECT);
}

bool QProfileView::isSelect()
{
	return m_bSelected;
}

cv::Mat QProfileView::getSelectImage()
{
	return m_selectROI.size().width <= 5 ? m_hoImage : m_hoImage(m_selectROI);
}

cv::Rect QProfileView::getSelectRect()
{
	return m_selectROI;
}

Rect2f QProfileView::getSelectScale()
{
	Rect2f scale;
	scale.x = m_selectROI.x;
	scale.y = m_selectROI.y;
	//scale.width = m_selectROI.width / m_imageWidth;
	//scale.height = m_selectROI.height / m_imageHeight;

	return scale;
}

void QProfileView::clearSelect()
{
	m_bSelected = false;
	m_selectROI.x = 0;
	m_selectROI.y = 0;
	m_selectROI.width = 0;
	m_selectROI.height = 0;
}

void QProfileView::setImage(cv::Mat& matImage)
{
	m_hoImage = matImage.clone();
	m_imageWidth = m_hoImage.size().width;
	m_imageHeight = m_hoImage.size().height;

	displayImage(m_hoImage);
}

cv::Mat QProfileView::getImage()
{
	return m_hoImage;
}

int QProfileView::getImageWidth()
{
	return m_imageWidth;
}

int QProfileView::getImageHeight()
{
	return m_imageHeight;
}

void QProfileView::clearImage()
{
	if (m_hoImage.empty()) return;
	m_hoImage.release();
	m_dispImage.release();
}

void QProfileView::addImageText(QString szText)
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

void QProfileView::mouseMoveEvent(QMouseEvent * event)
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
			if ((0 != (int)motionX) || (0 != (int)motionY))
			{
				if ((int)motionX >= 0 && (int)motionY >= 0)
				{
					//cv::Mat matImage = m_hoImage.clone();

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

					m_selectROI = select;

					repaintAll();
				}
			}
			break;
		case MODE_VIEW_MOVE:
			moveImage(mouseX - m_preMoveX, mouseY - m_preMoveY);
			break;
		case  MODE_VIEW_DRAG:
			if ((0 != (int)motionX))
			{
				if ((int)motionX >= 0)
				{
					double imageStartPosX = convertToImgX(m_startX);
					double imageStartPosY = convertToImgY(m_startY);
					if (imageStartPosX > 0 && imageStartPosX < m_imageWidth)
					{
						cv::Rect select;
						select.x = convertToImgX(m_startX);
						select.y = m_imageHeight / 8;
						select.width = convertToImgX(motionX, true);
						select.height = m_imageHeight - select.y;

						if (select.x < 0 || (select.x + select.width) > m_hoImage.size().width)
						{
							if (select.x < 0)
							{
								select.x = 0;
							}
						
							select.width = (select.x + select.width) > m_hoImage.size().width ? (m_hoImage.size().width - select.x) : select.width;							
						}

						m_measRangeCurr = select;

						repaintAll();
					}
				}
			}
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

void QProfileView::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (Qt::LeftButton == event->buttons())
	{		
		m_startX = event->x();
		m_startY = event->y();
	
		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:
			break;
		case MODE_VIEW_MOVE:
			break;
		case  MODE_VIEW_DRAG:			
			{
				double imageStartPosX = convertToImgX(m_startX);
				if (imageStartPosX > 0 && imageStartPosX < m_imageWidth)
				{
					cv::Rect select;
					select.x = convertToImgX(m_startX);
					select.y = m_imageHeight / 8;
					select.width = 1;
					select.height = m_imageHeight - select.y;

					if (select.x < 0 )
					{						
						select.x = 0;						
					}
					else if ((select.x + select.width) >= m_hoImage.size().width)
					{
						select.x = m_hoImage.size().width - 1;
					}

					m_measRangeCurr = select;

					repaintAll();

					if (m_measRangeCurr.width > 0)
					{					
						m_measRanges.push_back(m_measRangeCurr);
						m_measRangeCurr.width = 0;
						setViewState(MODE_VIEW_DRAG);

						repaintAll();
					}
				}
			}			
			break;
		case MODE_VIEW_NONE:
			break;
		default:
			break;
		}
	}
	else if (Qt::RightButton == event->buttons())
	{		
	}
}

void QProfileView::mousePressEvent(QMouseEvent * event)
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
			break;
		case MODE_VIEW_MOVE:
			break;
		case  MODE_VIEW_DRAG:
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
		//setViewState(MODE_VIEW_NONE);
		//repaintAll();
	}
}

void QProfileView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton)
	{
		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:
			if (m_selectROI.width > 0)
			{
				m_bSelected = true;
				setViewState(MODE_VIEW_NONE);
			}
			break;
		case MODE_VIEW_DRAG:
			if (m_measRangeCurr.width > 0)
			{
				m_measRanges.push_back(m_measRangeCurr);
				m_measRangeCurr.width = 0;
				setViewState(MODE_VIEW_DRAG);

				repaintAll();
			}
			break;
		case MODE_VIEW_NONE:
			break;
		default:
			break;
		}
	}
	else if (event->button() & Qt::RightButton)
	{
		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:
			if (m_selectROI.width <= 0)
			{
				m_selectROI.x = m_selectROI.y = 0;
				m_selectROI.width = m_hoImage.size().width;
				m_selectROI.height = m_hoImage.size().height;
			}
			m_bSelected = true;
			break;
		case MODE_VIEW_NONE:
			break;
		default:
			break;
		}
		setViewState(MODE_VIEW_NONE);
		repaintAll();
	}

	m_mouseLeftPressed = false;
	m_mouseRightPressed = false;
}

void QProfileView::wheelEvent(QWheelEvent * event)
{
	switch (m_stateView)
	{
	case MODE_VIEW_NONE:
		break;
	default:
		break;
	}
}

void QProfileView::loadImage(QString& fileName)
{
	m_hoImage = imread(fileName.toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);

	m_imageWidth = m_hoImage.size().width;
	m_imageHeight = m_hoImage.size().height;

	m_dScale = 1.0;
	m_dMovedX = 0.0;
	m_dMovedY = 0.0;

	setImage(m_hoImage);
}

void QProfileView::repaintAll()
{
	cv::Mat matImage = m_hoImage.clone();

	if (m_selectROI.width > 0)
	{
		cv::rectangle(matImage, m_selectROI, Scalar(255, 255, 0), 5, 8, 0);
	}

	if (m_measRangeCurr.width > 0)
	{
		cv::Rect rect = m_measRangeCurr;

		int width = rect.width;
		int height = rect.height;

		cv::Mat matRect = matImage(rect);
		//rectangle(matRect, vertices[1], vertices[3], Scalar(0,0,255, 100), -1);
		cv::Mat imgLayer(height, width, matImage.type(), Scalar(255, 0, 0));

		double alpha = 0.3;
		addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);
	}

	for (int i = 0; i < m_measRanges.size(); i++)
	{
		cv::Rect rect = m_measRanges.at(i);

		int width = rect.width;
		int height = rect.height;

		cv::Mat matRect = matImage(rect);
		//rectangle(matRect, vertices[1], vertices[3], Scalar(0,0,255, 100), -1);
		cv::Mat imgLayer(height, width, matImage.type(), Scalar(0, 0, 255));

		double alpha = 0.3;
		addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);
	}

	ui.lcdNumber_Count->display(m_measRanges.size());

	displayImage(matImage);
}

void QProfileView::A_Transform(Mat& src, Mat& dst, int dx, int dy)
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

void QProfileView::setViewState(int state)
{
	m_stateView = state;

	switch (m_stateView)
	{
	case MODE_VIEW_SELECT:
		setCursor(Qt::CrossCursor);
		break;
	case MODE_VIEW_MOVE:
		setCursor(Qt::OpenHandCursor);
		break;
	case MODE_VIEW_DRAG:
		setCursor(Qt::ArrowCursor);
		break;
	case MODE_VIEW_NONE:
		setCursor(Qt::ArrowCursor);
		break;
	default:
		setCursor(Qt::ArrowCursor);
		break;
	}
}

void QProfileView::displayImage(cv::Mat& image)
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

double QProfileView::convertToImgX(double dMouseValue, bool bLen)
{
	QRect rect = ui.label_Img->geometry();
	double fScaleW = m_windowWidth*1.0 / m_imageWidth;
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

double QProfileView::convertToImgY(double dMouseValue, bool bLen)
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

void QProfileView::fullImage()
{
	m_dScale = 1.0;
	m_dMovedX = 0.0;
	m_dMovedY = 0.0;

	if (!m_dispImage.empty())
		displayImage(m_dispImage);
	else
		displayImage(m_hoImage);
}

void QProfileView::zoomImage(double scale)
{
	m_dScale *= scale;
	if (m_dScale < 1.0) m_dScale = 1.0;

	if (!m_dispImage.empty())
		displayImage(m_dispImage);
	else
		displayImage(m_hoImage);
}

void QProfileView::moveImage(double motionX, double motionY)
{
	m_dMovedX += motionX;
	m_dMovedY += motionY;

	if (!m_dispImage.empty())
		displayImage(m_dispImage);
	else
		displayImage(m_hoImage);
}

void QProfileView::onStart()
{
	setViewState(MODE_VIEW_DRAG);
}

void QProfileView::onReset()
{
	clearMeasure();
	repaintAll();
}

void QProfileView::onDone()
{
	//clearMeasure();
	hide();	
}

void QProfileView::clearMeasure()
{
	m_measRanges.clear();

	m_measRangeCurr.x = 0;
	m_measRangeCurr.y = 0;
	m_measRangeCurr.width = 0;
	m_measRangeCurr.height = 0;

	repaintAll();
}