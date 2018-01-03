#include "HWndCtrl.h"
#include "QDebug"
#include  <QMouseEvent>
#include  <QWheelEvent>
#include "qlabel.h"
#include "qlineedit.h"
#include "qboxlayout.h"
#include "qpoint.h"
#include "../include/workflowDefine.h"
#include "../Common/eos.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

HWndCtrl::HWndCtrl( QWidget *parent  ) 
	:QWidget(parent) , m_parent(parent)
{
	//QRect rect = QRect(0 , 0 ,30,30);
 	//this->setGeometry( rect );

	//rectWindow = QRect(0,0 ,rect.width(), rect.height() );
	rectWindow = geometry();

	stateView = MODE_VIEW_NONE;
	windowWidth = rectWindow.width();
	windowHeight = rectWindow.height();

	imageHeight = 1 ;
	imageWidth = 1 ;

	mouseLeftPressed  = false;
	mouseRightPressed = false;

	zoomWndFactor = (double)imageWidth / rectWindow.width();
	zoomAddOn = std::pow(0.9, 5);
	zoomWndSize = 150;

	prevCompX = prevCompY = 0;

	dispROI = MODE_INCLUDE_ROI;//1;

	mGC =  GraphicsContext();

	QVBoxLayout *vboxLayout = new QVBoxLayout();
	hv_windowHandle = new QLabel(this);	
	vboxLayout->addWidget(hv_windowHandle);
	this->setLayout(vboxLayout);
	setAutoFillBackground(true);

	// 用于显示像素值的窗口
 	m_pWidgetValue = new QDialog( this );
	//m_pWidgetValue->setWindowFlags( Qt::FramelessWindowHint );  // 注意，这样设置后就无法显示了，昏 
	m_pWidgetValue->setWindowFlags( Qt::Dialog | Qt::FramelessWindowHint );
 	QLabel *pLabel1 = new QLabel(m_pWidgetValue);
 	pLabel1->setText("pos   : ");
	QLineEdit * pEditPos = new QLineEdit(m_pWidgetValue); 
	pEditPos->setObjectName( QStringLiteral("pos"));
	pEditPos->setEnabled(false);
	QLabel *pLabel2 = new QLabel(m_pWidgetValue);
	pLabel2->setText("value : ");
	QLineEdit *pEditValue = new QLineEdit(m_pWidgetValue); 
	pEditValue->setObjectName( QStringLiteral("value") ) ;
	pEditValue->setEnabled(false);
	QHBoxLayout *pLayout1 = new QHBoxLayout();
	pLayout1->addWidget(pLabel1);
	pLayout1->addWidget(pEditPos);
	QHBoxLayout *pLayout2 = new QHBoxLayout();
	pLayout2->addWidget(pLabel2);
	pLayout2->addWidget(pEditValue);
	QVBoxLayout *pLayoutWidget = new QVBoxLayout( m_pWidgetValue );
	pLayoutWidget->addLayout(pLayout1);
	pLayoutWidget->addLayout(pLayout2);
	m_pPosEdit    = m_pWidgetValue->findChild<QLineEdit*>("pos");
	m_pValueEdit  = m_pWidgetValue->findChild<QLineEdit*>("value");
	m_bShowFlag = true ;
}


HWndCtrl::~HWndCtrl(void)
{
}

void HWndCtrl::initial(QRect rect )
{
	

}

QLabel* HWndCtrl::getHWindowHandle()
{
	return hv_windowHandle;

}

int HWndCtrl::getImageWidth()
{
	return imageWidth;
}

int HWndCtrl::getImageHeight()
{
	return imageHeight;
}

void HWndCtrl::paintEvent( QPaintEvent *event )
{
	if (hv_windowHandle)
	{
		//QRect rect = this->geometry();
		//if (rect.width() > windowWidth)
		//{
		//	rectWindow = QRect(rect.x(), rect.y(), rect.width(), rect.height());
		//	windowWidth = rectWindow.width();
		//	windowHeight = rectWindow.height();
		//	qDebug() << "x " << rect.x() << " Y " << rect.y() << " width " << windowWidth << " height " << windowHeight;
		//}	
		
		QPalette palette;
		palette.setColor(QPalette::Background, QColor(0, 0, 0));
		this->setPalette(palette);
	}	
}

void HWndCtrl::mouseMoveEvent(QMouseEvent * event)
{
	// 注意， mouseMoveEvent 的响应 与 mouse tracking 的状态有关系， 为避免影响，直接单独设置 mousePressed 状态
	// qDebug()<<"haha";
	if (mouseLeftPressed)
	{
		double motionX = 0, motionY = 0;

		double scale = 0;
		switch (stateView)
		{
		case MODE_VIEW_MOVE:
		{
			motionX = event->x() - startX;
			motionY = event->y() - startY;
			if ((0 != (int)motionX) || (0 != (int)motionY))
			{
				moveImage(motionX, motionY);
				startX = event->x();
				startY = event->y();
			}
			break;
		}		
		case MODE_VIEW_NONE:
			break;
		default:
			break;
		}
	}
	else if (mouseRightPressed)
	{
		showPixelValue(event->x(), event->y());
	}
}

void HWndCtrl::mousePressEvent(QMouseEvent * event)
{
	if ( Qt::LeftButton ==  event->buttons()   )
	{
		if (MODE_VIEW_MOVE == stateView)
		{
			setCursor(Qt::OpenHandCursor);
		}	

		mouseLeftPressed = true;
		startX = event->x();
		startY = event->y();
	}

	if ( Qt::RightButton ==  event->buttons() && m_bShowFlag  )
	{
		mouseRightPressed = true;

		QString strPos = "( " + QString::number( event->x() ) + " ,  " + QString::number( event->y() ) + " ) ";
		//showValue(event->x(), event->y(), strPos , QString("value : 210") );
		showPixelValue(event->x(), event->y());
		m_pWidgetValue->show();
		m_pWidgetValue->raise();		
	}
}

void HWndCtrl:: mouseReleaseEvent(QMouseEvent *event)
{
	mouseLeftPressed = false ;

	mouseRightPressed = false;
	m_pWidgetValue->close();

	setCursor(Qt::ArrowCursor);
}

void HWndCtrl::wheelEvent(QWheelEvent * event)
{
	double scale = 0 ;
	switch ( stateView )
	{
		case MODE_VIEW_MOVE:
			break;
		case MODE_VIEW_ZOOM_IN:
			{
				int numDegrees = event->delta()/8;
				int numStep = numDegrees / 15;
				if ( numStep > 0  )
				{
					scale = 0.9*1;
				} 
				else
				{
					scale = 1/(0.9*1);
				}
				zoomImage(event->x(), event->y(), scale);
			}
			break;
		case MODE_VIEW_ZOOM_OUT:
			if ( 1 )
			{
			}
			break;
		case MODE_VIEW_NONE :
			break;
		default:
			break;
	}

	//QWidget::wheelEvent(event);
}

void HWndCtrl::showPixelValue(double posX, double posY)
{	
	m_pWidgetValue->move( cursor().pos() + QPoint(5 , 5) ) ;

	// 这里使用 ; 判断当前HObject 是否是图像
	for ( int nTmp= 0 ; nTmp < HObjList.length();  nTmp ++ )
	{
		cv::Mat *pHoTmp = &( HObjList[nTmp]._Obj ) ;
		try
		{
			
			{
				int cPointR, cPointG, cPointB;
				cPointB = pHoTmp->at<Vec3b>(posX, posY)[0];
				cPointG = pHoTmp->at<Vec3b>(posX, posY)[1];
				cPointR = pHoTmp->at<Vec3b>(posX, posY)[2];
				
				QString strValue = "( " ;
				
				strValue += QString::number(cPointR) + " , ";
				strValue += QString::number(cPointG) + " , ";
				strValue += QString::number(cPointB) + " ";			
				
				strValue += ") ";
				QString strPos = "( " + QString::number(posX) + " ,  " + QString::number(posY) + " ) ";
				
				m_pPosEdit->setText( strPos );
				m_pValueEdit->setText( strValue );
				break;
			}
		}
		catch ( ... )
		{
		}
	}
}


 void HWndCtrl::resizeEvent(QResizeEvent *event)
 {
	 setHWindowSize();
	 repaint();
 }


void HWndCtrl::addObj(cv::Mat &obj, bool bImage)
{
	if ( bImage ) 
	{
		if ((obj.size().width != imageWidth) || (obj.size().height != imageHeight))
		{
			imageHeight = obj.size().height;
			imageWidth = obj.size().width;
			resetAll( 1 ); // 默认情况下 设置显示的比例为图像的比例  resetAll(1) 则是适应窗口
		}
	}

	HObjectEntry entry = HObjectEntry(obj, mGC.copyContextList());
	HObjList<<entry;

	if (HObjList.length() > MAXNUMOBJLIST)
	{
		HObjList[0].clear();
		HObjList.removeFirst();
	}
}

void HWndCtrl:: resetAll( int flag )
{
	if ( 0 == flag)
	{
		// 保持比例
		double rateW = 1.0 * imageWidth  / windowWidth ;
		double rateH = 1.0 * imageHeight / windowHeight ;
		double rateUse = ( rateW > rateH ? rateW : rateH);
	
		// 移动到显示中心
		double midRowImage = imageHeight / 2.0 ;
		double midColImage = imageWidth  / 2.0 ;
		double midRowImageShow = windowHeight / 2.0;
		double midColImageShow = windowWidth / 2.0;
		double rowMove = midRowImageShow - midRowImage ;
		double colMove = midColImageShow - midColImage ;

		setImagePart(rowMove, colMove, (imageHeight + rowMove), (imageWidth + colMove));
		zoomWndFactor =  rateUse ;
	} 
	else
	{
		ImgRow1 = 0 ;
		ImgCol1 = 0 ;
		ImgRow2 = windowHeight;
		ImgCol2 = windowWidth;
		setImagePart( ImgRow1, ImgCol1, ImgRow2, ImgCol2 );
		zoomWndFactor = (double)imageWidth / rectWindow.width() ;
	}	
}

void HWndCtrl::clearList()
{
	for ( int i=0; i<HObjList.size(); i++)
	{
		HObjList[i].clear();
	}
	HObjList.clear();
}

/// <summary>
/// Adjust window settings by the values supplied for the left 
/// upper corner and the right lower corner
/// </summary>
 void HWndCtrl::setImagePart(int r1, int c1, int r2, int c2)
{
	ImgRow1 = r1;
	ImgCol1 = c1;
	ImgRow2 = r2;
	ImgCol2 = c2;
	
	hv_windowHandle->setGeometry(QRect(c1, r1, abs(c2 - c1), abs(r2 - r1)));
	hv_windowHandle->setFixedSize(QSize(abs(c2 - c1), abs(r2 - r1)));
 }

 void HWndCtrl::repaint()
 {
	 int count = HObjList.size();
	 if (0 == count)
	 {
		 return;
	 }

	 hv_windowHandle->clear();

	 for (int i = 0; i < count; i++)
	 {
		 HObjectEntry entry = ((HObjectEntry)HObjList[i]);
		 mGC.applyContext(hv_windowHandle, entry.gContext);	

		 QRect rect = hv_windowHandle->geometry();

		 //qDebug() << rect.x() << " " << rect.y() << " " << rect.width() << " " << rect.height();

		 cv::Mat mat, matMoved;
		 double fScaleW = rect.width()*1.0 / entry._Obj.size().width;
		 double fScaleH = rect.height()*1.0 / entry._Obj.size().height;
		 if (!entry._Obj.empty())
		 {
			 cv::resize(entry._Obj, mat, Size(entry._Obj.size().width*fScaleW, entry._Obj.size().height*fScaleH), (0, 0), (0, 0), 3);

			 if (mat.type() == CV_8UC3)
			 {
				 cvtColor(mat, mat, CV_BGR2RGB);
			 }
			 else if (mat.type() == CV_8UC1)
			 {
				 cvtColor(mat, mat, CV_GRAY2RGB);
			 }
			 A_Transform(mat, matMoved, rect.x(), rect.y());
			
			 //Rect rects = Rect(0, 0, mat.cols, mat.rows);//建立与srcImage一样大小的矩形框（0,0）坐标是起点 
			 //matMoved = Mat::zeros(mat.rows * 2, mat.cols * 4, mat.type());//设置移动区域A  			 
			 //Mat ImageROI = matMoved(rects);
			 //addWeighted(ImageROI, 0.1, mat, 1, 0., ImageROI);

			 QImage image = QImage((uchar*)matMoved.data, matMoved.cols, matMoved.rows, ToInt(matMoved.step), QImage::Format_RGB888);
			 //QImage image = cvMat2QImage(matMoved);
			 hv_windowHandle->setPixmap(QPixmap::fromImage(image));
		 }
	 }
 }

 QImage HWndCtrl::cvMat2QImage(const cv::Mat& mat)
 {
	 // 8-bits unsigned, NO. OF CHANNELS = 1  
	 if (mat.type() == CV_8UC1)
	 {
		 QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
		 // Set the color table (used to translate colour indexes to qRgb values)  
		 image.setColorCount(256);
		 for (int i = 0; i < 256; i++)
		 {
			 image.setColor(i, qRgb(i, i, i));
		 }
		 // Copy input Mat  
		 uchar *pSrc = mat.data;
		 for (int row = 0; row < mat.rows; row++)
		 {
			 uchar *pDest = image.scanLine(row);
			 memcpy(pDest, pSrc, mat.cols);
			 pSrc += mat.step;
		 }
		 return image;
	 }
	 // 8-bits unsigned, NO. OF CHANNELS = 3  
	 else if (mat.type() == CV_8UC3)
	 {
		 // Copy input Mat  
		 const uchar *pSrc = (const uchar*)mat.data;
		 // Create QImage with same dimensions as input Mat  
		 QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
		 return image.rgbSwapped();
	 }
	 else if (mat.type() == CV_8UC4)
	 {
		 qDebug() << "CV_8UC4";
		 // Copy input Mat  
		 const uchar *pSrc = (const uchar*)mat.data;
		 // Create QImage with same dimensions as input Mat  
		 QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
		 return image.copy();
	 }
	 else
	 {
		 qDebug() << "ERROR: Mat could not be converted to QImage.";
		 return QImage();
	 }
 }

 cv::Mat HWndCtrl::QImage2cvMat(QImage image)
 {
	 cv::Mat mat;
	 qDebug() << image.format();
	 switch (image.format())
	 {
	 case QImage::Format_ARGB32:
	 case QImage::Format_RGB32:
	 case QImage::Format_ARGB32_Premultiplied:
		 mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
		 break;
	 case QImage::Format_RGB888:
		 mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
		 cv::cvtColor(mat, mat, CV_BGR2RGB);
		 break;
	 case QImage::Format_Indexed8:
		 mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
		 break;
	 }
	 return mat;
 }

 void HWndCtrl::A_Transform(Mat& src, Mat& dst, int dx, int dy)
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

 void HWndCtrl:: changeGraphicSettings(QString mode, QString val)
 {
	 mGC.addValue(mode, val);
 }

 void HWndCtrl::setViewState( int state)
 {
	 stateView = state;
 }

 void  HWndCtrl::zoomImage( double x, double y, double scale)
 {
	 double xInImage = 0 , yInImage = 0 ; 
	 xInImage = ImgCol1 + x / windowWidth * ( ImgCol2 - ImgCol1) ;
	 yInImage= ImgRow1  + y / windowHeight *(ImgRow2 - ImgRow1);

	 double lengthC, lengthR;
	 double pencentC, pencentR;

	 pencentC = ( 0 == ( ImgCol2 - ImgCol1) ? 0 : (xInImage - ImgCol1) / ( ImgCol2 - ImgCol1) );
	 pencentR = ( 0 == ( ImgRow2 - ImgRow1) ? 0 : (yInImage - ImgRow1) / ( ImgRow2 - ImgRow1));

	 lengthC = (ImgCol2 - ImgCol1) * scale;
	 lengthR = (ImgRow2 - ImgRow1) * scale;

	 double ImgCol1Tmp = xInImage - lengthC * pencentC;
	 double ImgCol2Tmp = xInImage + lengthC * (1 - pencentC);
	 double ImgRow1Tmp = yInImage - lengthR * pencentR;
	 double ImgRow2Tmp = yInImage + lengthR *(1 - pencentR);
	 if ( ImgCol1Tmp == ImgCol2Tmp)
	 {
		 ImgCol1Tmp = xInImage - 0.5 ;
		 ImgCol2Tmp = xInImage + 0.5;

		 ImgCol1 = ImgCol1Tmp ;
		 ImgCol2 = ImgCol2Tmp ;

	 }

	 if ( ImgRow1 == ImgRow2 )
	 {
		 ImgRow1Tmp = yInImage - 0.5;
		 ImgRow2Tmp = yInImage + 0.5 ;
		 ImgRow1 = ImgRow1Tmp ;
		 ImgRow2 = ImgRow2Tmp ;

	 }

	 double T = 30000;
	 if( fabs(ImgCol1Tmp) < T && fabs(ImgCol2Tmp) < T && fabs(ImgRow1Tmp) < T && fabs(ImgRow2Tmp) < T )
	 {
		 ImgCol1 = ImgCol1Tmp ;
		 ImgCol2 = ImgCol2Tmp ;
		 ImgRow1 = ImgRow1Tmp ;
		 ImgRow2 = ImgRow2Tmp ;

		 zoomWndFactor *= scale;
		 setImagePart(ImgRow1, ImgCol1, ImgRow2, ImgCol2);
		 repaint();

	 } 
	
 }

 void HWndCtrl::moveImage(double motionX, double motionY)
 {
	 //double deltaR =  motionY / windowHeight * (ImgRow2 - ImgRow1);
	 //double deltaC =  motionX / windowWidth  * (ImgCol2 - ImgCol1);	
	 double deltaR = motionY;
	 double deltaC = motionX;
	 ImgRow1 += deltaR;
	 ImgRow2 += deltaR;

	 ImgCol1 += deltaC;
	 ImgCol2 += deltaC;

	 setImagePart(ImgRow1, ImgCol1, ImgRow2, ImgCol2);
	 repaint();

	 qDebug() << QString("move %1,%2").arg(motionX).arg(motionY) << endl;
 }

 void HWndCtrl:: setHWindowSize( QRect rect  )
 {
	 if (!hv_windowHandle)
	 {
		 return ;
	 }

	 QRect rectTmp = rect;
	 if ( 0 == rect.width() )
	 {
		 rectTmp = this->geometry();
	 } 

	 if (rectTmp.width() > windowWidth)
	 {
		 //windowHeight = rectTmp.height();
		 //windowWidth = rectTmp.width();
		 hv_windowHandle->setGeometry(QRect(0, 0, rectTmp.width(), rectTmp.height()));
		 hv_windowHandle->setFixedSize(QSize(rectTmp.width(), rectTmp.height()));
		 //QRect rect = hv_windowHandle->geometry();

		 rectWindow = QRect(0, 0, rectTmp.width(), rectTmp.height());
		 windowWidth = rectWindow.width();
		 windowHeight = rectWindow.height();
		 zoomWndFactor = (double)imageWidth / rectWindow.width();
	 }	
 }

 void HWndCtrl:: setShowFlag( bool bShowFlag)
 {
	 m_bShowFlag = bShowFlag ;
 }