#include "viewctrl.h"
#include "caramemodel_global.h"
#include "QMessageBox"
#include "QFileDialog"
#include "qboxlayout.h"
#include "..\Common\eos.h"
#include "../include/ICamera.h"
#include "../Common/SystemData.h"
#include "qmenu.h"
#include "qdebug.h"
#include "qstring.h"
#include <QApplication>
#include <qdatetime.h>

#include "../3DViewUtility/DViewUtility.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

CameraOnLive::CameraOnLive(ViewCtrl* pCtrl, CameraDevice *pCameraTmp, QLabel* hv_windowHandle)
	:m_pCtrl(pCtrl), m_pCameraTmp(pCameraTmp), m_hv_windowHandle(hv_windowHandle)
{	
	m_bQuit = false;
	m_bRuning = false;
}

void CameraOnLive::run()
{
	m_bRuning = true;

	bool bHardwareTrigger = System->getParam("camera_hw_tri_enable").toBool();
	//bool bHardwareTriggerContinuous = System->getParam("camera_hw_tri_continuous").toBool();	
	//m_pCameraTmp->setHardwareTrigger(bHardwareTrigger);

	QString capturePath = System->getParam("camera_cap_image_path").toString();
	bool bCaptureImage = System->getParam("camera_cap_image_enable").toBool();	

	if (bHardwareTrigger && bCaptureImage)
	{
		QDir dirDefaultPath;
		dirDefaultPath.setPath(capturePath);

		if (!dirDefaultPath.exists())
		{
			System->setTrackInfo("Default Caputuring path is not exit! Please browser to set the path!");
		}
	}	

	bool bHardwareFirstCaptured = false;
	cv::Mat image;
	while (!m_bQuit)
	{
		if (bHardwareTrigger)
		{
			if (bHardwareFirstCaptured)
			{
				bHardwareFirstCaptured = false;

				m_pCameraTmp->startGrabing(1);

				m_pCameraTmp->softwareTrigger();
				QThread::msleep(100);
				m_pCameraTmp->softwareTrigger();

				QVector<cv::Mat> buffers;
				if (m_pCameraTmp->captureImageByFrameTrig(buffers))
				{					
					m_pCameraTmp->stopGrabing();
				}
				else
				{
					m_pCameraTmp->stopGrabing();
				}
				m_pCtrl->clearImage();
				if (m_bQuit)break;
			}

			m_pCameraTmp->startGrabing(m_pCtrl->getImageBufferNum());

			QVector<cv::Mat> buffers;
			if (m_pCameraTmp->captureImageByFrameTrig(buffers))
			{
				m_pCtrl->setImageBuffer(buffers);
				m_pCameraTmp->stopGrabing();

				//showImageToScreen(buffers.last());			
				if (bCaptureImage && buffers.size() > 0)
				{
					QDateTime dtm = QDateTime::currentDateTime();
					QString fileDir = capturePath + "/" + dtm.toString("MMddhhmmss") + "/";
					QDir dir; dir.mkdir(fileDir);

					bool bCaptureImageAsMatlab = System->getParam("camera_cap_image_matlab").toBool();
					for (int i = 0; i < buffers.size(); i++)
					{
						cv::Mat image = buffers.at(i);
						if (bCaptureImageAsMatlab)
						{
							int nIndex = i + 1;
							if (6 == nIndex)
							{
								nIndex += 1;
							}
							else if (7 == nIndex)
							{
								nIndex -= 1;
							}
							QString name = QString("%1").arg(nIndex, 2, 10, QChar('0')) + QStringLiteral(".bmp");
							m_pCameraTmp->saveImage(image, name, fileDir);
						}
						else
						{
							m_pCameraTmp->saveImage(image);
						}

						//QThread::msleep(1000);
					}					
				}				
			}
			else
			{
				m_pCameraTmp->stopGrabing();
				System->setTrackInfo("Capture Images Time Out. Try again!");
			}

			if (m_bQuit)break;
		}
		else if ((m_pCameraTmp->captureImage(image)))
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
	bool bHardwareTrigger = System->getParam("camera_hw_tri_enable").toBool();
	if (bHardwareTrigger)
	{
		m_pCameraTmp->stopGrabing();
	}
	m_bQuit = true;
}

void CameraOnLive::showImageToScreen(cv::Mat& image)
{
	if (!m_hv_windowHandle) return;

	QRect rect = m_hv_windowHandle->geometry();

	cv::Mat mat;
	double fScaleW = rect.width()*1.0 / image.size().width;
	double fScaleH = rect.height()*1.0 / image.size().height;
	cv::resize(image, mat, Size(image.size().width*fScaleW, image.size().height*fScaleH), (0, 0), (0, 0), 3);

	if (mat.type() == CV_8UC3)
	{
		cvtColor(mat, mat, CV_BGR2RGB);
	}
	else if (mat.type() == CV_8UC1)
	{
		cvtColor(mat, mat, CV_GRAY2RGB);
	}
	QImage imageNew = QImage((uchar*)mat.data, mat.cols, mat.rows, ToInt(mat.step), QImage::Format_RGB888);
	m_hv_windowHandle->setPixmap(QPixmap::fromImage(imageNew));
}

void CameraOnLive::drawCross(cv::Mat& image)
{
	cv::Point startPt, endPt;

	if (m_pCtrl->getCameraCtrl()->isCrossEnable())
	{
		int nCrossWidth = m_pCtrl->getCameraCtrl()->getCrossGap();

		startPt.x = 0;
		startPt.y = image.size().height / 2 - nCrossWidth/2;
		endPt.x = image.size().width;
		endPt.y = image.size().height / 2 - nCrossWidth/2;
		cv::line(image, startPt, endPt, Scalar(255, 255, 0), 1, 8);

		startPt.x = 0;
		startPt.y = image.size().height / 2 + nCrossWidth / 2;
		endPt.x = image.size().width;
		endPt.y = image.size().height / 2 + nCrossWidth / 2;
		cv::line(image, startPt, endPt, Scalar(255, 255, 0), 1, 8);

		startPt.x = image.size().width / 2 - nCrossWidth / 2;
		startPt.y = 0;
		endPt.x = image.size().width / 2 - nCrossWidth / 2;
		endPt.y = image.size().height;
		cv::line(image, startPt, endPt, Scalar(255, 255, 0), 2, 8);

		startPt.x = image.size().width / 2 + nCrossWidth / 2;
		startPt.y = 0;
		endPt.x = image.size().width / 2 + nCrossWidth / 2;
		endPt.y = image.size().height;
		cv::line(image, startPt, endPt, Scalar(255, 255, 0), 2, 8);
	}
	
	startPt.x = 0;
	startPt.y = image.size().height / 2;
	endPt.x = image.size().width;
	endPt.y = image.size().height / 2;
	cv::line(image, startPt, endPt, Scalar(255, 0, 0), 2, 8);

	startPt.x = image.size().width / 2;
	startPt.y = 0;
	endPt.x = image.size().width / 2;
	endPt.y = image.size().height;
	cv::line(image, startPt, endPt, Scalar(255, 0, 0), 2, 8);
}

ViewCtrl::ViewCtrl(  QWidget *parent )
	: QWidget(parent), m_hwndCtrl( this )
{
	m_hv_windowHandle = m_hwndCtrl.getHWindowHandle() ;

    ui.setupUi(this);

	// 信号槽toolButton_openImages
	connect( ui.toolButton_openImage, SIGNAL(clicked()) ,  this,   SLOT(onClickOpenFile())  );
	connect( ui.toolButton_openImages, SIGNAL(clicked()) ,  this,   SLOT(onToolButton_openImages())  );
	connect( ui.toolButton_snapImage, SIGNAL(clicked()),  this, SLOT(onClickSnapImage()));
	connect( ui.toolButton_saveImage, SIGNAL(clicked()) , this, SLOT( onClickSaveImage() )  );
	connect( ui.toolButton_onLive, SIGNAL(clicked()) , this, SLOT( onClickPushbutton_onLive() ) );
	connect( ui.toolButton_stop, SIGNAL(clicked()) , this, SLOT( onClickPushbutton_stopLive() ) );
	connect( ui.toolButton_zoom, SIGNAL(clicked()) , this , SLOT(onClickZoom() ) );
	connect( ui.toolButton_adapt, SIGNAL(clicked()) , this , SLOT(onClickAdaptWindow() )  );
	connect( ui.toolButton_adaptImage, SIGNAL(clicked()) , this, SLOT(onToolButton_adaptImage()) );
	connect( ui.toolButton_moveImage, SIGNAL(clicked()) , this ,  SLOT( onToolButton_moveImage() )  );
	connect( ui.comboBox_cameras, SIGNAL(currentIndexChanged(int)) , this , SLOT(onComboBox_cameras() ) );
	
	connect( ui.toolButton_firstPic, SIGNAL(clicked() ) , this , SLOT( onToolButton_firstPic() ) );
	connect( ui.toolButton_forwardPic, SIGNAL(clicked() ) , this , SLOT( onToolButton_forwardPic() ) );
	connect( ui.toolButton_nextPic, SIGNAL(clicked() ) , this , SLOT( onToolButton_nextPic() ) );
	connect(ui.toolButton_LastPic, SIGNAL(clicked()), this, SLOT(onToolButton_LastPic()));

	connect(ui.toolButton_3DView, SIGNAL(clicked()), this, SLOT(onToolButton_3DView()));


	// 布局
	m_pLayoutToolsMain = new QHBoxLayout();
	m_pLayoutTools = new QHBoxLayout();
	m_pLayoutToolsAnd = new QHBoxLayout();
	m_pLayoutToolsOther = new QHBoxLayout();
	m_pLayoutHWindow = new QHBoxLayout();
	m_pLayoutMain = new QVBoxLayout(this);

	// 初始化
	// initial(); // 由外面初始化
	m_nCaptureNum = DLP_SEQ_PATTERN_IMG_NUM;

	m_pView3D = new DViewUtility();// = NULL;
}

ViewCtrl::~ViewCtrl()
{
	if (m_pCameraOnLive) onClickPushbutton_stopLive();

	//if (m_pView3D)
	//{
	//	delete m_pView3D;
	//	m_pView3D = NULL;
	//}
}

void ViewCtrl::initial( int nWindow )
{
	// 其他
	freshCur();
	
	// 相机线程
	m_pCameraOnLive = NULL;
	m_pCameraCur = NULL;

	// button 设置
	setButtonsEnable(true);
	ui.toolButton_stop->setEnabled(false);
	//ui.toolButton_Draw->setEnabled(false);
	// 布局
	setMyLayout();	
}

void ViewCtrl::setMyLayout()
{
	 // tools
	 m_pLayoutTools->addWidget( ui.toolButton_openImage );
	 m_pLayoutTools->addWidget( ui.toolButton_openImages );
	 m_pLayoutTools->addWidget( ui.toolButton_snapImage );
	 m_pLayoutTools->addWidget( ui.toolButton_saveImage );
	 m_pLayoutTools->addWidget( ui.toolButton_onLive );
	 m_pLayoutTools->addWidget( ui.toolButton_stop );
	 m_pLayoutTools->addWidget( ui.toolButton_zoom);
	 m_pLayoutTools->addWidget( ui.toolButton_adapt );
	 m_pLayoutTools->addWidget( ui.toolButton_adaptImage );
	 m_pLayoutTools->addWidget( ui.toolButton_moveImage );
	 m_pLayoutTools->addWidget(ui.toolButton_Draw);
	 m_pLayoutTools->addWidget( ui.comboBox_cameras );
	 m_pLayoutTools->addStretch(0); // 哈哈！！！！， 加上这个控件之间的间距就是所需要的， 否则间距很大
	 m_pLayoutTools->setContentsMargins(0,0,0,0);
	 m_pLayoutTools->setSpacing(1);
     m_pLayoutTools->setSizeConstraint( QLayout::SetDefaultConstraint );

	 //工具按钮制作弹出菜单  
	 QMenu *menuDraw =  new QMenu();  
	 menuDraw = new QMenu();
	 connect(menuDraw,SIGNAL(triggered(QAction *)),this,SLOT(onMenu_Draw_Triggered(QAction *)));
	 m_ActionMenuDrawRect   = new QAction(menuDraw);
	 m_ActionMenuDrawRect2  = new QAction(menuDraw);
	 m_ActionMenuDrawCircle = new QAction(menuDraw);
	 m_ActionMenuDrawPoly   = new QAction(menuDraw);
	 menuDraw->addAction(m_ActionMenuDrawRect);
	 menuDraw->addAction(m_ActionMenuDrawRect2);
	 menuDraw->addAction(m_ActionMenuDrawCircle);
	 menuDraw->addAction(m_ActionMenuDrawPoly);

	 m_ActionMenuDrawRect->setText(QStringLiteral("绘制矩形"));	 
	 QIcon icon1;
	 icon1.addFile(QStringLiteral(":/Images/Resources/images/v_rect.png"), QSize(), QIcon::Normal, QIcon::Off);
	 m_ActionMenuDrawRect->setIcon(icon1);

	 m_ActionMenuDrawRect2->setText(QStringLiteral("绘制矩形2"));
	 QIcon icon2;
	 icon2.addFile(QStringLiteral(":/Images/Resources/images/v_rect2.png"), QSize(), QIcon::Normal, QIcon::Off);
	 m_ActionMenuDrawRect2->setIcon(icon2);

	 m_ActionMenuDrawCircle->setText(QStringLiteral("绘制圆形"));
	 QIcon icon3;
	 icon3.addFile(QStringLiteral(":/Images/Resources/images/v_circle.png"), QSize(), QIcon::Normal, QIcon::Off);
	 m_ActionMenuDrawCircle->setIcon(icon3);

	 m_ActionMenuDrawPoly->setText(QStringLiteral("绘制多边形"));
	 QIcon icon4;
	 icon4.addFile(QStringLiteral(":/Images/Resources/images/v_poly.png"), QSize(), QIcon::Normal, QIcon::Off);
	 m_ActionMenuDrawPoly->setIcon(icon4);

	 ui.toolButton_Draw->setMenu(menuDraw);

	 // navigation
	 m_pLayoutToolsAnd->addWidget( ui.toolButton_firstPic );
	 m_pLayoutToolsAnd->addWidget( ui.toolButton_forwardPic );
	 m_pLayoutToolsAnd->addWidget( ui.lineEdit_picName );
	 m_pLayoutToolsAnd->addWidget( ui.toolButton_nextPic );
	 m_pLayoutToolsAnd->addWidget( ui.toolButton_LastPic );
	 m_pLayoutToolsAnd->addStretch(0); // 哈哈！！！！， 加上这个控件之间的间距就是所需要的， 否则间距很大
	 m_pLayoutToolsAnd->setContentsMargins(0,0,0,0);
	 m_pLayoutToolsAnd->setSpacing(0);

	 // navigation
	 m_pLayoutToolsOther->addWidget(ui.toolButton_3DView, 0, Qt::AlignRight);	
	 m_pLayoutToolsOther->addStretch(0); 
	 m_pLayoutToolsOther->setContentsMargins(0, 0, 0, 0);
	 m_pLayoutToolsOther->setSpacing(0);

	 // HWindows
	 m_pLayoutHWindow->addWidget( &m_hwndCtrl );

	 m_pLayoutToolsMain->addLayout(m_pLayoutTools);
	 m_pLayoutToolsMain->addLayout(m_pLayoutToolsAnd);
	 m_pLayoutToolsMain->addLayout(m_pLayoutToolsOther);
	 m_pLayoutToolsMain->addStretch(0);
	 m_pLayoutToolsMain->setAlignment(Qt::AlignLeft);
	 m_pLayoutMain->addLayout(m_pLayoutToolsMain);

	 //m_pLayoutMain->addLayout( m_pLayoutTools );
	 //m_pLayoutMain->addLayout( m_pLayoutToolsAnd );
	 m_pLayoutMain->addLayout( m_pLayoutHWindow );
	 
}

void ViewCtrl::connectCameraCtrl( CameraCtrl* pCameralCtrl )
{
	// 相机
	m_pCameralCtrl = pCameralCtrl ;

	ui.comboBox_cameras->clear();
	if ( 0 < m_pCameralCtrl->getCameraCount()   )
	{
		for ( int nTmp = 0 ; nTmp < m_pCameralCtrl->getCameraCount() ; nTmp ++)
		{
			ui.comboBox_cameras->addItem(  QString("Cam") + QString::number( nTmp )  );
		}
		m_pCameraCur = m_pCameralCtrl->getCamera(0);
	}
	else
	{
		m_pCameraCur = NULL;
	}
	
}


void ViewCtrl::onMenu_Draw_Triggered(QAction * action)
{
	//Mat* pHo = NULL;
	//Mat* pHo2 = NULL;
	//cv::Mat * pHo = &(m_hoRegionDrawObject);
	//cv::Mat * pHo2 = &(m_hoImageDrawObject);
	//cv::Mat * pHv = &(m_hvDrawObject);
	//pHo->release();
	//pHo2->release();
	//pHv->release();

	if (m_hoImage.empty()) return;
	
	m_hwndCtrl.setShowFlag(false);	

	if ( action == m_ActionMenuDrawRect)
	{
		cv::Rect select;
		select.x = 50;
		select.y = 50;
		select.width = 1000;
		select.height = 1000;
		cv::rectangle(m_hoImage, select, Scalar(0, 255, 0), 3, 8, 0);		
	}
	else if ( action == m_ActionMenuDrawRect2 )
	{
		cv::Rect select;
		select.x = 500;
		select.y = 500;
		select.width = 1000;
		select.height = 1000;
		cv::rectangle(m_hoImage, select, Scalar(0, 255, 0), 3, 8, 0);
	}
	else if ( action == m_ActionMenuDrawCircle)
	{
		cv::Point p1;
		p1.x = 1000;
		p1.y = 500;
		cv::circle(m_hoImage, p1, 300, Scalar(0, 255, 0), 3, 8, 0);
	}
	else if ( action == m_ActionMenuDrawPoly )
	{
		Point rook_points[1][20];
		rook_points[0][3] = Point(300, 1000);
		rook_points[0][0] = Point(1000, 100);
		rook_points[0][1] = Point(100,500);
		rook_points[0][2] = Point(100, 10);

		const Point* ppt[1] = { rook_points[0] };
		int npt[] = { 4 };

		cv::polylines(m_hoImage, ppt, npt, 1, 1, Scalar(0, 255, 0), 3, 8, 0);
	}

	// show
	m_hwndCtrl.clearList();
	m_hwndCtrl.addObj( m_hoImage, true );
	//m_hwndCtrl.changeGraphicSettings( "DrawMode", "margin");
	//m_hwndCtrl.changeGraphicSettings( "Color", "red");
	//m_hwndCtrl.addObj(*pHo);
	m_hwndCtrl.repaint();

	m_hwndCtrl.setShowFlag(true);
}

void ViewCtrl::freshCur()
{
	
}

void ViewCtrl::freshNavigation()
{
	/******/
	m_hwndCtrl.clearList();
	m_hwndCtrl.repaint();

	int indPicCur = m_picIndCur ;
	if ( indPicCur < 0  )
	{
		return;
	}

	QString filePath = m_picPath[ indPicCur ];
	QString strTmp = QString::number( indPicCur ) + "\\" +  QString::number( m_picNum ) +  " :" ;
	strTmp += filePath.right( filePath.length() - filePath.lastIndexOf("/") - 1 ); 
	ui.lineEdit_picName->setText( strTmp );

	m_hoImage.release();	
	
	m_hoImage = imread(filePath.toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_GRAYSCALE | CV_LOAD_IMAGE_UNCHANGED);

	m_hwndCtrl.clearList();
	m_hwndCtrl.addObj(m_hoImage, true);
	m_hwndCtrl.repaint();	
}

void ViewCtrl::resizeEvent(QResizeEvent *event)
{
	setHWindowSize();
}

void ViewCtrl::setHWindowSize()
{
	m_hwndCtrl.setGeometry( getHWindowRect() );	
}

QRect  ViewCtrl::getHWindowRect()
{
	QRect tmp = ui.toolButton_firstPic->geometry();
	return QRect( tmp.left(), tmp.top() + tmp.height() + 3 , this->width() - 2 *tmp.left() , this->height() - tmp.top() - tmp.height() - 6  );
}

void   ViewCtrl::getDrawObjects(  int nCamera,   void *pHo_Region , void * pHv_ofRegion, void *pHo_Image ) 
{
	if (!pHo_Region)
	{
		return;
	}

	cv::Mat *pHo_RegionTmp = (cv::Mat*)pHo_Region;
	cv::Mat  *pHv_ofRegionTmp = (cv::Mat*)pHv_ofRegion;
	cv::Mat *pHo_ImageTmp = (cv::Mat*)pHo_Image;
	pHo_RegionTmp->release();

	*pHo_RegionTmp = m_hoRegionDrawObject;
	if (NULL != pHv_ofRegion)
	{
		*pHv_ofRegionTmp = m_hvDrawObject;
	}

	if (NULL != pHo_Image)
	{
		pHo_ImageTmp->release();
		*pHo_ImageTmp = m_hoImageDrawObject;
	}
}

void ViewCtrl::addImageText(QString szText)
{
	if (m_hoImage.empty()) return;

	const int ImageWidth = 2048;
	double dScaleFactor = (double)m_hwndCtrl.getImageWidth() / ImageWidth;

	cv::Point p1;
	p1.x = 10;
	p1.y = 50 * dScaleFactor;

	cv::String text;

	text = szText.toStdString();

	cv::Mat image = m_hoImage.clone();
	double fontScale = dScaleFactor*2.0f;
	cv::putText(image, text, p1, CV_FONT_HERSHEY_COMPLEX, fontScale, Scalar(0, 0, 255), 2);

	// show
	m_hwndCtrl.clearList();
	m_hwndCtrl.addObj(image, true);
	m_hwndCtrl.repaint();	
}

void ViewCtrl::clearImage()
{
	if (m_hoImage.empty()) return;

	// show
	m_hwndCtrl.clearList();
	m_hwndCtrl.addObj(m_hoImage, true);
	m_hwndCtrl.repaint();
}

void ViewCtrl::setImage(cv::Mat image)
{
	// show
	m_hwndCtrl.clearList();
	m_hwndCtrl.addObj(image, true);
	m_hwndCtrl.repaint();
}

void ViewCtrl::show3D()
{
	//if (!m_pView3D)
	//{
	//	m_pView3D = new DViewUtility();
	//}

	if (m_pView3D)
	{
		m_pView3D->show();
	}		
}

bool ViewCtrl::startUpCapture()
{
	System->setParam("camera_hw_tri_enable", true);
	//System->setParam("camera_cap_image_enable", false);
	emit onClickPushbutton_onLive();

	return true;
}

bool ViewCtrl::endUpCapture()
{
	emit onClickPushbutton_stopLive();
	//System->setParam("camera_cap_image_enable", true);

	return true;
}

void ViewCtrl::load3DViewData(int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues)
{
	if (m_pView3D)
	{		
		m_pView3D->loadFile(false, nSizeX, nSizeY, xValues, yValues, zValues);
	}
}

void  ViewCtrl::onClickOpenFile()
{
	
	QString picFilter = "Image(*.tif *.tiff *.gif *.bmp *.jpg *.jpeg *.jp2 *.png *.pcx *.pgm *.ppm *.pbm *.xwd *.ima)";
	QString strFileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开图片"),"/",picFilter);
	if ( !strFileName.isEmpty() )
	{
		try
		{
			m_picPath.clear();
			m_picPath<< strFileName ;
			m_picNum = 1;                      // 图片个数
			m_picIndCur = 0 ;                   // 当前索引 
			freshNavigation();
		}
		catch (std::exception& e)
		{
			QMessageBox::warning(this, "", QStringLiteral("Halcon打开图片失败:") + e.what());
		}
		catch( ... )
		{
			QMessageBox::warning(this, "", QStringLiteral("打开图片失败"));
		}
	}
}

void ViewCtrl::onToolButton_openImages()
{
	QString dirPath =  QFileDialog::getExistingDirectory(this, "", "");
	if ( dirPath.isEmpty() )
	{
		return;
	}

	//判断路径是否存在
	QDir dir(dirPath);
	if (!dir.exists())
	{
		return;
	}

	QStringList filters;
	filters << QString("*.jpeg") << QString("*.jpg") << QString("*.png") << QString("*.tiff") << QString("*.gif") << QString("*.bmp");
	dir.setFilter(QDir::Files | QDir::NoSymLinks); //设置类型过滤器，只为文件格式
	dir.setNameFilters(filters);  //设置文件名称过滤器，只为filters格式（后缀为.jpeg等图片格式）

	int dir_count = dir.count();
	if (dir_count <= 0)
	{
		return;
	}

	// 遍历文件夹获取图片
	(m_picPath).clear();

	//获取分隔符	
	QChar separator = QChar('/');
	if (!dirPath.contains(separator))
	{
		separator = QChar('\\');
	}
	QChar last_char = dirPath.at(dirPath.length() - 1);
	if (last_char == separator)
	{
		separator = QChar();
	}

	for (uint i = 0; i < dir_count; i++)
	{
		QString file_name = dir[i];  //文件名称
		QString file_path = dirPath + separator + file_name;   //文件全路径
		m_picPath << file_path;
	}

	m_picNum = dir_count;  // 图片个数
	m_picIndCur = 0;                   // 当前索引 
	freshNavigation();
}

void  ViewCtrl::onClickSnapImage()
{
	if (!m_pCameraCur || !m_pCameraCur->getStatus())
	{
		QMessageBox::warning(this, "", QStringLiteral("无相机") );
		return;
	}

	//bool bHardwareTrigger = System->getParam("camera_hw_tri_enable").toBool();
	//if (bHardwareTrigger)
	//{
	//	QMessageBox::warning(this, "", QStringLiteral("不支持硬件触发"));
	//	return;
	//}
	
	m_hoImage.release();
	if ( (m_pCameraCur->captureImage(m_hoImage)) )
	{
		m_hwndCtrl.clearList();
		m_hwndCtrl.addObj(m_hoImage, true);
		m_hwndCtrl.repaint();
	}	
	else
	{
		QMessageBox::warning(this, "", QStringLiteral("获取图像失败") );
	}
}


void  ViewCtrl::onClickPushbutton_onLive()
{
	if (!m_pCameraCur || !m_pCameraCur->getStatus())
	{
		QMessageBox::warning(this, "", QStringLiteral("无相机") );
		return;
	}	

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

	if (!m_pCameraOnLive)
	{
		setButtonsEnable(false);
		ui.toolButton_stop->setEnabled(true);
		m_hwndCtrl.resetAll(1);
		m_pCameraOnLive = new CameraOnLive(this, m_pCameraCur, m_hv_windowHandle);
		m_pCameraOnLive->start();
	}	
}

void  ViewCtrl::onClickPushbutton_stopLive()
{
	if (!m_pCameraCur || !m_pCameraCur->getStatus())
	{
		QMessageBox::warning(this, "", QStringLiteral("无相机或者没打开"));
		return;
	}

	if ( m_pCameraOnLive )
	{
		m_pCameraCur->stopGrabing();

		ui.toolButton_stop->setEnabled(false);
		m_pCameraOnLive->setQuitFlag();
		while (m_pCameraOnLive->isRuning())
		{
			QThread::msleep(10);
			QApplication::processEvents();
		}
		QThread::msleep(200);
		delete m_pCameraOnLive;
		m_pCameraOnLive = NULL;

		setButtonsEnable(true);		
	}
}

void  ViewCtrl::onClickZoom()
{
	m_hwndCtrl.setViewState( MODE_VIEW_ZOOM_IN );
}

void ViewCtrl:: onClickSaveImage()
{
	if (!m_hoImage.empty())
	{
		QString picFilter = "Image( *.bmp )";
		QString strSave = QFileDialog::getSaveFileName(this, QStringLiteral("打开图片"), "/", picFilter);
		if (!strSave.isEmpty())
		{
			strSave += ".bmp";
			IplImage frameImg = IplImage(m_hoImage);
			cvSaveImage(strSave.toStdString().c_str(), &frameImg);
			/*std::vector<int> comp;
			comp.push_back(CV_IMWRITE_PNG_BILEVEL);
			comp.push_back(98);
			imwrite(strSave.toStdString(), m_hoImage, comp);*/
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

void ViewCtrl:: onClickAdaptWindow()
{
	m_hwndCtrl.setViewState(MODE_VIEW_NONE);
	m_hwndCtrl.resetAll( 1 );
	m_hwndCtrl.repaint();
}

void ViewCtrl::onToolButton_adaptImage()
{
	m_hwndCtrl.setViewState(MODE_VIEW_NONE);
	m_hwndCtrl.resetAll( 0 );
	m_hwndCtrl.repaint();
}

void ViewCtrl:: onToolButton_moveImage()
{
	m_hwndCtrl.setViewState( MODE_VIEW_MOVE );
}

void ViewCtrl::onComboBox_cameras()
{
	/****/
	int indCamera = ui.comboBox_cameras->currentIndex();
	if ( indCamera >= 0  )
	{
		m_pCameraCur = m_pCameralCtrl->getCamera(indCamera);
	}
	/****/
}

void ViewCtrl::setCurDevice( int indCam)
{
	if ( indCam < ui.comboBox_cameras->count() )
	{
		ui.comboBox_cameras->setCurrentIndex( indCam );
	}
}

void  ViewCtrl::onToolButton_firstPic() 
{
	if ( m_picNum > 0  )
	{
		m_picIndCur = 0 ;                   // 当前索引 
		freshNavigation();
	}
	
}

void  ViewCtrl::onToolButton_forwardPic() 
{
	if ( m_picNum > 0  )
	{
		m_picIndCur -= 1 ;                   // 当前索引 
		m_picIndCur = ( m_picIndCur< 0 ? m_picNum -1 :  m_picIndCur );
		freshNavigation();
	}
}

void ViewCtrl:: onToolButton_nextPic() 
{
	if ( m_picNum > 0  )
	{
		m_picIndCur += 1 ;                   // 当前索引 
		m_picIndCur = ( m_picIndCur >= m_picNum ? 0 :  m_picIndCur );
		freshNavigation();
	}
}

void  ViewCtrl::onToolButton_LastPic()
{
	if ( m_picNum > 0  )
	{
		m_picIndCur = m_picNum - 1  ;                   // 当前索引 
		freshNavigation();
	}
}

void  ViewCtrl::onToolButton_3DView()
{
	show3D();
}


HWndCtrl *   ViewCtrl::getHwndCtrl( )
{
	return &m_hwndCtrl;
	 
}

CameraCtrl * ViewCtrl::getCameraCtrl()
{
	return m_pCameralCtrl;
}

cv::Mat   ViewCtrl::getImage( int indPic  )
{
	if ( -1 == indPic )
	{
		return m_hoImage ;
	}
	else
	{
		int num = m_picNum;
		if ( indPic >= 0 && indPic <= num -1  )
		{
			Mat ho_tmp = Mat();			
			QString filePath = m_picPath[ indPic ];			
			ho_tmp = imread(filePath.toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);
			return ho_tmp;
		}
		else
		{
			return Mat();
		}
	}
	
}


void ViewCtrl::setImageBuffer(QVector<cv::Mat>& matImages)
{
	for (int i = 0; i < matImages.size(); i++)
	{
		m_bufferImages.push_back(matImages[i].clone());
	}
}

const QVector<cv::Mat>& ViewCtrl::getImageBuffer()
{
	return m_bufferImages;
}

int	ViewCtrl::getImageBufferNum()
{
	return m_nCaptureNum;
}

void ViewCtrl::clearImageBuffer()
{
	m_bufferImages.clear();
}

void  ViewCtrl:: setButtonsEnable( bool flag )
{	
	ui.toolButton_openImage->setEnabled(flag);
	ui.toolButton_openImages->setEnabled(flag);
	ui.toolButton_snapImage->setEnabled(flag);
	ui.toolButton_saveImage->setEnabled(flag);
	ui.toolButton_onLive->setEnabled(flag);
	ui.toolButton_stop->setEnabled(flag);
	ui.toolButton_zoom->setEnabled(flag);
	ui.toolButton_adapt->setEnabled(flag);
	ui.toolButton_adaptImage->setEnabled(flag);
	ui.comboBox_cameras->setEnabled(flag);
	ui.toolButton_moveImage->setEnabled(flag);
	ui.toolButton_Draw->setEnabled(flag);

	ui.toolButton_firstPic->setEnabled(flag);
    ui.toolButton_forwardPic->setEnabled(flag);
	ui.lineEdit_picName ->setEnabled(flag);
	ui.toolButton_nextPic ->setEnabled(flag);
	ui.toolButton_LastPic ->setEnabled(flag);
}

