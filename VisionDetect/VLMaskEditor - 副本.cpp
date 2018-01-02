#include "VLMaskEditor.h"
#include "../Common/SystemData.h"
#include "../include/IdDefine.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMath.h>
#include  <QMouseEvent>
#include  <QWheelEvent>

#include "../lib/VisionLibrary/include/VisionAPI.h"

#define ToInt(value)                (static_cast<int>(value))

const int LABEL_IMAGE_WIDTH = 800;
const int LABEL_IMAGE_HEIGHT = 800;

const int MODE_VIEW_NONE = 1;
const int MODE_VIEW_SELECT = 2;
const int MODE_VIEW_ADD_SELECT = 3;
const int MODE_VIEW_ADD_MASK = 4;

using namespace AOI;

QVLMaskEditor::QVLMaskEditor(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.comboBox_selectIndex, SIGNAL(currentIndexChanged(int)), SLOT(onSelectIndexChanged(int)));
	connect(ui.pushButton_addSelect, SIGNAL(clicked()), SLOT(onAddSelect()));
	connect(ui.pushButton_deleteSelect, SIGNAL(clicked()), SLOT(onDeleteSelect()));

	connect(ui.comboBox_maskIndex, SIGNAL(currentIndexChanged(int)), SLOT(onMaskIndexChanged(int)));
	connect(ui.pushButton_addMask, SIGNAL(clicked()), SLOT(onAddMask()));
	connect(ui.pushButton_deleteMask, SIGNAL(clicked()), SLOT(onDeleteMask()));


	m_stateView = MODE_VIEW_SELECT;
	m_windowWidth = LABEL_IMAGE_WIDTH;
	m_windowHeight = LABEL_IMAGE_HEIGHT;
	m_startX = 0;
	m_startY = 0;

	m_imageHeight = 1;
	m_imageWidth = 1;

	m_mouseLeftPressed = false;
	m_mouseRightPressed = false;

	m_nSelectIndex = -1;

	m_bSelectRect = false;

	initValue();
}

QVLMaskEditor::~QVLMaskEditor()
{
	//if (m_ActionDelete)
	//{
	//	delete m_ActionDelete;
	//	m_ActionDelete = NULL;
	//}
	//if (m_ActionMove)
	//{
	//	delete m_ActionMove;
	//	m_ActionMove = NULL;
	//}

	m_selectRects.clear();

	for (int i = 0; i < m_maskObjs.size(); i++)
	{
		delete m_maskObjs[i];
	}
	m_maskObjs.clear();
}

void QVLMaskEditor::closeEvent(QCloseEvent *e){
	//qDebug() << "关闭事件";
	//e->ignore();

	this->hide();
}

void QVLMaskEditor::initValue()
{
	ui.comboBox_selectIndex->addItem(QString("%1").arg(QStringLiteral("请选择区域")));
	ui.comboBox_maskIndex->addItem(QString("%1").arg(QStringLiteral("请选择区域")));

	ui.radioButton_rect->setChecked(true);
	ui.radioButton_circle->setChecked(false);
	ui.radioButton_polyline->setChecked(false);

	ui.label_Img->setFixedSize(LABEL_IMAGE_WIDTH, LABEL_IMAGE_HEIGHT);
	ui.label_Img->setMouseTracking(true);
	setMouseTracking(true);

	//QString path = QApplication::applicationDirPath();
	//path += "/image/";	

	//m_ActionMove = new QAction(tr("&Move"), this);	
	//QIcon moveIcon(path + "moveView.png");
	//m_ActionMove->setIcon(moveIcon);
	//m_ActionMove->setShortcut(QKeySequence::MoveToEndOfDocument);
	//connect(m_ActionMove, SIGNAL(triggered()), this, SLOT(actionMove()));
	//addAction(m_ActionMove);

	//m_ActionDelete = new QAction(tr("&Delete"), this);
	//QIcon deleteIcon(path + "close-hove.png");
	//m_ActionDelete->setIcon(deleteIcon);
	//m_ActionDelete->setShortcut(QKeySequence::Delete);
	//connect(m_ActionDelete, SIGNAL(triggered()), this, SLOT(actionDelete()));
	//addAction(m_ActionDelete);

	//setContextMenuPolicy(Qt::ActionsContextMenu);
}

void QVLMaskEditor::setImage(cv::Mat& matImage, bool bClearAll)
{
	if (bClearAll) clear();

	m_hoImage = matImage.clone();
	m_imageWidth = m_hoImage.size().width;
	m_imageHeight = m_hoImage.size().height;

	displayImage(m_hoImage);
}

int QVLMaskEditor::getSelectNum()
{
	return m_selectRects.size();
}

RotatedRect QVLMaskEditor::getSelect(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_selectRects.size())
	{
		return m_selectRects[nIndex];
	}
	return RotatedRect();
}

Rect QVLMaskEditor::getSelectRect(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_selectRects.size())
	{
		return m_selectRects[nIndex].boundingRect();
	}
	return Rect();
}

cv::Mat QVLMaskEditor::getMaskImage()
{
	cv::Mat matImage(m_hoImage.rows, m_hoImage.cols, CV_8UC1, m_maskObjs.size() > 0 ? Scalar(0) : Scalar(255));

	for (int i = 0; i < m_maskObjs.size(); i++)
	{
		MaskObj* obj = m_maskObjs[i];
		switch (obj->getType())
		{
		case MASK_TYPE_RECT:
		{
			RectMask* maskObj = dynamic_cast<RectMask*>(obj);
			if (maskObj && !maskObj->isEmpty())
			{
				Point2f vertices[4];
				maskObj->_rect.points(vertices);

				int x = vertices[1].x;
				int y = vertices[1].y;
				int width = vertices[3].x - vertices[1].x;
				int height = vertices[3].y - vertices[1].y;
				
				rectangle(matImage, vertices[1], vertices[3], Scalar(255), -1);
			}
		}
		break;
		case MASK_TYPE_CIRCLE:
		{
			CircleMask* maskObj = dynamic_cast<CircleMask*>(obj);
			if (maskObj && !maskObj->isEmpty())
			{			
				circle(matImage, maskObj->_center, maskObj->_radius, Scalar(255), -1);
			}
		}
		break;
		case MASK_TYPE_POLYLINE:
		{
			PolyLineMask* maskObj = dynamic_cast<PolyLineMask*>(obj);
			if (maskObj && !maskObj->isEmpty() && maskObj->_polyPts.size() > 2)
			{
				Point *points = new Point[maskObj->_polyPts.size()];
				for (int i = 0; i < maskObj->_polyPts.size(); i++)
				{
					points[i].x = maskObj->_polyPts[i].x;
					points[i].y = maskObj->_polyPts[i].y;
				}

				const Point* pt[1] = { points };
				int npt[1] = { maskObj->_polyPts.size() };

				polylines(matImage, pt, npt, 1, 1, Scalar(255));
				fillPoly(matImage, pt, npt, 1, Scalar(255), 8);

				delete points;			
			}
		}
		break;
		case MASK_TYPE_NULL:
			break;
		default:
			break;
		}
	}

	return matImage;
}

void QVLMaskEditor::setEditorView(bool bSelectEnable, bool bMaskEnable)
{
	ui.groupBox->setEnabled(bSelectEnable);
	ui.groupBox_2->setEnabled(bMaskEnable);

	//setViewState(MODE_VIEW_ADD_SELECT);
}

void QVLMaskEditor::setSelectRtRect(bool bSelectRect)
{
	m_bSelectRect = bSelectRect;
}

void QVLMaskEditor::mouseMoveEvent(QMouseEvent * event)
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
		{
			if (m_nSelectIndex > -1)
			{
				cv::RotatedRect rect = m_selectRects[m_nSelectIndex];

				double dScaleX = 0, dScaleY = 0, dRotatedAngle = 0;
				if (rangeSelect(mouseX, mouseY, dScaleX, dScaleY))
				{
					rect.size.width *= dScaleX;
					rect.size.height *= dScaleY;

					drawSelectRect(rect);							
				}
				else if (!m_bSelectRect && rotateSelect(mouseX, mouseY, dRotatedAngle))
				{					
					rect.angle += dRotatedAngle;

					drawSelectRectRotated(rect, dRotatedAngle);
				}
				else if (findSelect(m_startX, m_startY) == m_nSelectIndex)
				{					
					rect.center.x += convertToImgX(motionX, true);
					rect.center.y += convertToImgY(motionY, true);

					drawSelectRect(rect);
				}
			}
		}
		break;
		case MODE_VIEW_ADD_SELECT:
		{			
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

					cv::Point2f slCenter;
					cv::Size2f slSize;
					slCenter.x = select.x + select.width / 2;
					slCenter.y = select.y + select.height / 2;
					slSize.width = select.width;
					slSize.height = select.height;
					m_selectRectCur = cv::RotatedRect(slCenter, slSize, 0);

					//displayImage(matImage);
					repaintAll();
				}
			}
			break;
		}
		case MODE_VIEW_ADD_MASK:
		{
			if ((0 != (int)motionX) || (0 != (int)motionY))
			{				
				if ((int)motionX >= 0 && (int)motionY >= 0)
				{
					double imageStartPosX = convertToImgX(m_startX);
					double imageStartPosY = convertToImgY(m_startY);
					if (imageStartPosX > 0 && imageStartPosX < m_imageWidth && imageStartPosY > 0 && imageStartPosY < m_imageHeight)
					{
						cv::Rect select;
						select.x = convertToImgX(m_startX);
						select.y = convertToImgY(m_startY);
						select.width = convertToImgX(motionX, true);
						select.height = convertToImgY(motionY, true);

						if (select.width > 10 && (select.x + select.width) < m_imageWidth && select.height > 10 && (select.y + select.height) < m_imageHeight)
						{
							MaskType maskType = getMaskType();
							switch (maskType)
							{
							case MASK_TYPE_RECT:
							{
								cv::Point2f slCenter;
								cv::Size2f slSize;
								slCenter.x = select.x + select.width / 2;
								slCenter.y = select.y + select.height / 2;
								slSize.width = select.width;
								slSize.height = select.height;
								m_maskRectCur._rect = cv::RotatedRect(slCenter, slSize, 0);
							}
							break;
							case MASK_TYPE_CIRCLE:
							{
								cv::Point2f maskCenter;
								maskCenter.x = select.x + select.width / 2;
								maskCenter.y = select.y + select.height / 2;

								double dRadius = select.width / 2 < select.height / 2 ? select.width / 2 : select.height / 2;

								m_maskCircleCur._center = maskCenter;
								m_maskCircleCur._radius = dRadius;
							}
							break;
							case MASK_TYPE_POLYLINE:
								break;
							case MASK_TYPE_NULL:
								break;
							default:
								break;
							}

							repaintAll();
						}
					}
				}
			}
			break;
		}
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
		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:
		{
			QString path = QApplication::applicationDirPath();
			path += "/image/";
			QCursor cursor;

			int nIndex = getLocFromSelect(mouseX, mouseY);
			if (nIndex > -1 && nIndex < 4)
			{
				if (0 == nIndex || 3 == nIndex)
				{
					QPixmap pixmap(path + "vsizeHand.png");
					cursor = QCursor(pixmap, -1, -1);
					setCursor(cursor);
				}
				else
				{
					QPixmap pixmap(path + "hsizeHand.png");
					cursor = QCursor(pixmap, -1, -1);
					setCursor(cursor);
				}				
			}
			else if (!m_bSelectRect && nIndex == 4)
			{				
				QPixmap pixmap(path + "rotateHand.png");
				cursor = QCursor(pixmap, -1, -1);
				setCursor(cursor);
			}
			else
			{
				if (m_nSelectIndex > -1 && findSelect(mouseX, mouseY) == m_nSelectIndex)
				{					
					QPixmap pixmap(path + "moveHand.png");
					cursor = QCursor(pixmap, -1, -1);
					setCursor(cursor);
				}
				else
				{
					setCursor(Qt::ArrowCursor);
				}				
			}
		}
		break;
		case MODE_VIEW_ADD_MASK:
		{
			MaskType maskType = getMaskType();
			if (MASK_TYPE_POLYLINE == maskType)
			{

			}
		}
		break;
		case MODE_VIEW_NONE:
			break;
		default:
			break;
		}
	}
}

void QVLMaskEditor::mousePressEvent(QMouseEvent * event)
{
	if (Qt::LeftButton == event->buttons())
	{
		m_mouseLeftPressed = true;
		m_startX = event->x();
		m_startY = event->y();

		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:
			{
				if (m_nSelectIndex < 0 )
				{
					int nIndex = findSelect(m_startX, m_startY);
					if (nIndex > -1)
					{
						drawSelectRect(m_selectRects[nIndex]);

						m_nSelectIndex = nIndex;
						ui.comboBox_selectIndex->setCurrentIndex(m_nSelectIndex + 1);
					}
				}
				/*else
				{
					m_nSelectIndex = -1;
					repaintAll();
				}*/
			}
			break;
		case MODE_VIEW_ADD_MASK:
		{
			MaskType maskType = getMaskType();
			if (MASK_TYPE_POLYLINE == maskType)
			{
				double imageStartPosX = convertToImgX(m_startX);
				double imageStartPosY = convertToImgY(m_startY);

				m_maskPolyLineCur._polyPts.push_back(cv::Point2f(imageStartPosX, imageStartPosY));

				repaintAll();
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
		if (MODE_VIEW_ADD_MASK == m_stateView)
		{
			MaskType maskType = getMaskType();
			if (MASK_TYPE_POLYLINE == maskType)
			{
				if (!m_maskPolyLineCur.isEmpty())
				{
					PolyLineMask* mask = new PolyLineMask();
					mask->_polyPts.assign(m_maskPolyLineCur._polyPts.begin(), m_maskPolyLineCur._polyPts.end());

					m_maskObjs.push_back(mask);
					ui.comboBox_maskIndex->addItem(QString("%1").arg(m_maskObjs.size()));

					m_maskPolyLineCur.clear();					
				}
			}
		}

		m_mouseRightPressed = true;
		setViewState(MODE_VIEW_SELECT);
		m_nSelectIndex = -1;
		ui.comboBox_selectIndex->setCurrentIndex(0);
		ui.comboBox_maskIndex->setCurrentIndex(0);
		unSelectMask();
		repaintAll();
	}
}

void QVLMaskEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton)
	{
		switch (m_stateView)
		{
		case MODE_VIEW_SELECT:
		{
			if (m_nSelectIndex > -1)
			{
				double dScaleX = 0, dScaleY = 0, dRotatedAngle = 0;
				if (rangeSelect(event->x(), event->y(), dScaleX, dScaleY))
				{
					m_selectRects[m_nSelectIndex].size.width *= dScaleX;
					m_selectRects[m_nSelectIndex].size.height *= dScaleY;
				}
				else if (!m_bSelectRect && rotateSelect(event->x(), event->y(), dRotatedAngle))
				{
					m_selectRects[m_nSelectIndex].angle += dRotatedAngle;
				}
				else if (findSelect(m_startX, m_startY) == m_nSelectIndex)
				{
					m_selectRects[m_nSelectIndex].center.x += convertToImgX(event->x() - m_startX, true);
					m_selectRects[m_nSelectIndex].center.y += convertToImgY(event->y() - m_startY, true);
				}

				drawSelectRect(m_selectRects[m_nSelectIndex]);
			}
		}
		break;
		case MODE_VIEW_ADD_SELECT:
			if (m_selectRectCur.size.width > 1 && m_selectRectCur.size.height > 1)
			{
				m_selectRects.push_back(m_selectRectCur);
				ui.comboBox_selectIndex->addItem(QString("%1").arg(m_selectRects.size()));
				m_selectRectCur.size.width = 0;
				m_selectRectCur.size.height = 0;

				repaintAll();
			}
			break;
		case MODE_VIEW_ADD_MASK:
			{
				MaskType maskType = getMaskType();
				switch (maskType)
				{
				case MASK_TYPE_RECT:
				{
					if (!m_maskRectCur.isEmpty())
					{
						RectMask* mask = new RectMask();
						mask->_rect = m_maskRectCur._rect;						

						m_maskObjs.push_back(mask);
						ui.comboBox_maskIndex->addItem(QString("%1").arg(m_maskObjs.size()));

						m_maskRectCur.clear();

						repaintAll();
					}
				}
				break;
				case MASK_TYPE_CIRCLE:
				{
					if (!m_maskCircleCur.isEmpty())
					{
						CircleMask* mask = new CircleMask();
						mask->_center = m_maskCircleCur._center;
						mask->_radius = m_maskCircleCur._radius;						

						m_maskObjs.push_back(mask);
						ui.comboBox_maskIndex->addItem(QString("%1").arg(m_maskObjs.size()));

						m_maskCircleCur.clear();

						repaintAll();
					}
				}
					break;
				case MASK_TYPE_POLYLINE:
					break;
				case MASK_TYPE_NULL:
					break;
				default:
					break;
				}				
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
	}
	
	m_mouseLeftPressed = false;
	m_mouseRightPressed = false;	
}

void QVLMaskEditor::wheelEvent(QWheelEvent * event)
{
	switch (m_stateView)
	{
	case MODE_VIEW_NONE:
		break;
	default:
		break;
	}

	//QWidget::wheelEvent(event);
}

void QVLMaskEditor::A_Transform(Mat& src, Mat& dst, int dx, int dy)
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

void QVLMaskEditor::setViewState(int state)
{
	m_stateView = state;

	switch (m_stateView)
	{
	case MODE_VIEW_SELECT:
		setCursor(Qt::ArrowCursor);
		break;
	case MODE_VIEW_ADD_SELECT:
	case MODE_VIEW_ADD_MASK:
		setCursor(Qt::CrossCursor);
		break;	
	case MODE_VIEW_NONE:		
	default:
		setCursor(Qt::ArrowCursor);
		break;
	}
}

void QVLMaskEditor::displayImage(cv::Mat& image)
{
	QRect rect = ui.label_Img->geometry();
	rect.setX(0); rect.setY(0);
	rect.setWidth(LABEL_IMAGE_WIDTH); rect.setHeight(LABEL_IMAGE_HEIGHT);

	cv::Mat mat, matMoved;
	double fScaleW = rect.width()*1.0 / image.size().width;
	double fScaleH = rect.height()*1.0 / image.size().height;

	double fScale = fScaleH < fScaleW ? fScaleH : fScaleW;

	if (!image.empty())
	{
		cv::resize(image, mat, cv::Size(image.size().width*fScale, image.size().height*fScale), (0, 0), (0, 0), 3);		

		if (mat.type() == CV_8UC3)
		{
			cvtColor(mat, mat, CV_BGR2RGB);
		}
		else if (mat.type() == CV_8UC1)
		{
			cvtColor(mat, mat, CV_GRAY2RGB);
		}

		cv::Mat matDisp = Mat::zeros(cv::Size(rect.width(), rect.height()), mat.type());		
		if (mat.cols < mat.rows)
		{
			Mat outImg;
			outImg = matDisp(Rect((mat.rows - mat.cols)/2, 0, mat.cols, mat.rows));
			mat.copyTo(outImg);
		}
		else if (mat.cols > mat.rows)
		{
			Mat outImg;
			outImg = matDisp(Rect(0, (mat.cols - mat.rows)/2, mat.cols, mat.rows));
			mat.copyTo(outImg);
		}
		else
		{
			mat.copyTo(matDisp);
		}		

		//A_Transform(mat, matMoved, rect.x(), rect.y());
		//QImage image = QImage((uchar*)matMoved.data, matMoved.cols, matMoved.rows, ToInt(matMoved.step), QImage::Format_RGB888);

		QImage image = QImage((uchar*)matDisp.data, matDisp.cols, matDisp.rows, ToInt(matDisp.step), QImage::Format_RGB888);
		ui.label_Img->setPixmap(QPixmap::fromImage(image));
	}
}

void QVLMaskEditor::repaintAll()
{
	cv::Mat matImage = m_hoImage.clone();

	for (int i = 0; i < m_selectRects.size(); i++)
	{
		Point2f vertices[4];
		m_selectRects[i].points(vertices);

		for (int i = 0; i < 4; i++)
		{
			line(matImage, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0), 2);
		}
	}

	if (m_selectRectCur.size.width > 1)
	{
		Point2f vertices[4];
		m_selectRectCur.points(vertices);

		for (int i = 0; i < 4; i++)
		{
			line(matImage, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0), 2);
		}
	}

	if (!m_maskRectCur.isEmpty())
	{
		Point2f vertices[4];
		m_maskRectCur._rect.points(vertices);

		int x = vertices[1].x;
		int y = vertices[1].y;
		int width = vertices[3].x - vertices[1].x;
		int height = vertices[3].y - vertices[1].y;


		cv::Mat matRect = matImage(Rect(x, y, width, height));
		//rectangle(matRect, vertices[1], vertices[3], Scalar(0,0,255, 100), -1);
		cv::Mat imgLayer(height, width, CV_8UC3, Scalar(0, 0, 255));

		double alpha = 0.3;
		addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);
	}

	if (!m_maskCircleCur.isEmpty())
	{
		int x = m_maskCircleCur._center.x - m_maskCircleCur._radius;
		int y = m_maskCircleCur._center.y - m_maskCircleCur._radius;
		int width = m_maskCircleCur._radius*2;
		int height = m_maskCircleCur._radius * 2;


		cv::Mat matRect = matImage(Rect(x, y, width, height));
		//rectangle(matRect, vertices[1], vertices[3], Scalar(0,0,255, 100), -1);
		cv::Mat imgLayer = matRect.clone();
		circle(imgLayer, cv::Point2f(m_maskCircleCur._radius, m_maskCircleCur._radius), m_maskCircleCur._radius, Scalar(0, 0, 255), -1);

		double alpha = 0.3;
		addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);
	}

	if (!m_maskPolyLineCur.isEmpty() && m_maskPolyLineCur._polyPts.size() > 0)
	{
		Point *points = new Point[m_maskPolyLineCur._polyPts.size()];
		for (int i = 0; i < m_maskPolyLineCur._polyPts.size(); i++)
		{
			points[i].x = m_maskPolyLineCur._polyPts[i].x;
			points[i].y = m_maskPolyLineCur._polyPts[i].y;
		}

		const Point* pt[1] = { points };
		int npt[1] = { m_maskPolyLineCur._polyPts.size() };

		cv::Mat imgLayer = matImage.clone();

		polylines(imgLayer, pt, npt, 1, 1, Scalar(0, 0, 255));
		fillPoly(imgLayer, pt, npt, 1, Scalar(0, 0, 255), 8);

		delete points;

		double alpha = 0.3;
		addWeighted(matImage, alpha, imgLayer, 1 - alpha, 0, matImage);
	}

	for (int i = 0; i < m_maskObjs.size(); i++)
	{
		MaskObj* obj = m_maskObjs[i];
		switch (obj->getType())
		{
		case MASK_TYPE_RECT:
		{
			RectMask* maskObj = dynamic_cast<RectMask*>(obj);
			if (maskObj && !maskObj->isEmpty())
			{
				Point2f vertices[4];
				maskObj->_rect.points(vertices);

				int x = vertices[1].x;
				int y = vertices[1].y;
				int width = vertices[3].x - vertices[1].x;
				int height = vertices[3].y - vertices[1].y;


				cv::Mat matRect = matImage(Rect(x, y, width, height));
				//rectangle(matRect, vertices[1], vertices[3], Scalar(0,0,255, 100), -1);
				cv::Mat imgLayer(height, width, CV_8UC3, maskObj->IsSelect() ? Scalar(128, 128, 255) : Scalar(0, 0, 255));

				double alpha = 0.3;
				addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);
			}
		}
		break;
		case MASK_TYPE_CIRCLE:
		{
			CircleMask* maskObj = dynamic_cast<CircleMask*>(obj);
			if (maskObj && !maskObj->isEmpty())
			{
				int x = maskObj->_center.x - maskObj->_radius;
				int y = maskObj->_center.y - maskObj->_radius;
				int width = maskObj->_radius * 2;
				int height = maskObj->_radius * 2;


				cv::Mat matRect = matImage(Rect(x, y, width, height));
				//rectangle(matRect, vertices[1], vertices[3], Scalar(0,0,255, 100), -1);
				cv::Mat imgLayer = matRect.clone();
				circle(imgLayer, cv::Point2f(maskObj->_radius, maskObj->_radius), maskObj->_radius, maskObj->IsSelect() ? Scalar(128, 128, 255) : Scalar(0, 0, 255), -1);

				double alpha = 0.3;
				addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);
			}
		}
			break;
		case MASK_TYPE_POLYLINE:
		{
			PolyLineMask* maskObj = dynamic_cast<PolyLineMask*>(obj);
			if (maskObj && !maskObj->isEmpty() && maskObj->_polyPts.size() > 2)
			{
				Point *points = new Point[maskObj->_polyPts.size()];
				for (int i = 0; i < maskObj->_polyPts.size(); i++)
				{
					points[i].x = maskObj->_polyPts[i].x;
					points[i].y = maskObj->_polyPts[i].y;
				}

				const Point* pt[1] = { points };
				int npt[1] = { maskObj->_polyPts.size() };

				cv::Mat imgLayer = matImage.clone();

				polylines(imgLayer, pt, npt, 1, 1, maskObj->IsSelect() ? Scalar(128, 128, 255) : Scalar(0, 0, 255));
				fillPoly(imgLayer, pt, npt, 1, maskObj->IsSelect() ? Scalar(128, 128, 255) : Scalar(0, 0, 255), 8);

				delete points;

				double alpha = 0.3;
				addWeighted(matImage, alpha, imgLayer, 1 - alpha, 0, matImage);
			}
		}
			break;
		case MASK_TYPE_NULL:
			break;
		default:
			break;	
		}
	}

	//m_ActionMove->setEnabled(false);
	//m_ActionDelete->setEnabled(false);
	displayImage(matImage);
}

double QVLMaskEditor::convertToImgX(double dMouseValue, bool bZero)
{
	QRect rect = ui.label_Img->geometry();
	double fScaleW = m_windowWidth*1.0 / m_imageWidth;
	double fScaleH = m_windowHeight*1.0 / m_imageHeight;

	return (dMouseValue - (bZero ? 0 : rect.x())) / fScaleW;
}

double QVLMaskEditor::convertToImgY(double dMouseValue, bool bZero)
{
	QRect rect = ui.label_Img->geometry();
	double fScaleW = m_windowWidth*1.0 / m_imageWidth;
	double fScaleH = m_windowHeight*1.0 / m_imageHeight;

	return (dMouseValue - (bZero ? 0 : rect.y())) / fScaleH;
}

void QVLMaskEditor::addImageText(cv::Mat image, Point ptPos, QString szText)
{
	if (m_hoImage.empty()) return;

	const int ImageWidth = 800;
	double dScaleFactor = (double)m_imageWidth / ImageWidth;
	
	cv::String text = szText.toStdString();

	Point ptPos1;
	ptPos1.x = ptPos.x - 10 * dScaleFactor;
	ptPos1.y = ptPos.y + 10 * dScaleFactor;
	Point ptPos2;
	ptPos2.x = ptPos.x + 10 * dScaleFactor * text.length();
	ptPos2.y = ptPos.y - 20 * dScaleFactor;
	rectangle(image, ptPos1, ptPos2, Scalar(125, 125, 125), -1);
	rectangle(image, ptPos1, ptPos2, Scalar(200, 200, 200), 1);

	double fontScale = dScaleFactor*0.5f;
	cv::putText(image, text, ptPos, CV_FONT_HERSHEY_COMPLEX, fontScale, Scalar(0, 255, 255), 1);
}

int QVLMaskEditor::findSelect(double mousePosX, double mousePosY)
{
	int nIndex = -1;
	for (int i = 0; i < m_selectRects.size(); i++)
	{
		Point2f vertices[4];
		m_selectRects[i].points(vertices);	

		double posX = convertToImgX(mousePosX);
		double posY = convertToImgY(mousePosY);

		std::vector<Point2f> vecTmp;
		for (int j = 0; j < 4; j++)
		{
			vecTmp.push_back(vertices[j]);
		}
		Mat matContour = Mat(vecTmp, true);

		double dDist = pointPolygonTest(matContour, Point2f(posX, posY), 0);

		if (dDist > 0)
		{
			nIndex = i;
			break;
		}
	}

	return nIndex;
}

bool QVLMaskEditor::getRotRectLocationPt(Mat& img, int nCornerNum, Point2f* ptCorners, int nLocationNum, Point2f* ptLocations, double& dRadius)
{
	if (!ptCorners || !ptLocations) return false;
	if (nCornerNum < 4) return false;
	if (nLocationNum != (nCornerNum + 1)) return false;

	double dCircleRadius = (double)img.rows / 100;
	dRadius = dCircleRadius;

	for (int i = 0; i < nCornerNum; i++)
	{
		ptLocations[i] = ptCorners[i];
	}

	//get rotate point
	double X1 = ptCorners[1].x;
	double Y1 = ptCorners[1].y;
	double X2 = ptCorners[2].x;
	double Y2 = ptCorners[2].y;

	double dDistVt = qSqrt(qPow(X1 - X2, 2) + qPow(Y1 - Y2, 2)) / 5;

	if (dDistVt > 1)
	{
		double A = Y2 - Y1;
		double B = X1 - X2;
		double C = X2*Y1 - X1*Y2;
		double angle = qAtan2((double)(Y2 - Y1), (double)(X2 - X1));
		
		double angleVt = -M_PI / 2 + angle;
		double XVt = (X1 + X2) / 2;
		double YVt = (Y1 + Y2) / 2;		

		double XRt = XVt + dDistVt*qCos(angleVt);
		double YRt = YVt + dDistVt*qSin(angleVt);

		ptLocations[nCornerNum].x = XRt;
		ptLocations[nCornerNum].y = YRt;
	}

	return true;
}

int QVLMaskEditor::getLocFromSelect(double mousePosX, double mousePosY)
{
	if (m_nSelectIndex < 0) return -1;
	cv::Mat matImage = m_hoImage;

	double imgPosX = convertToImgX(mousePosX);
	double imgPosY = convertToImgY(mousePosY);

	Point2f vertices[4];
	m_selectRects[m_nSelectIndex].points(vertices);

	Point2f locations[5]; double dCircleRadius = 0;
	if (getRotRectLocationPt(matImage, 4, vertices, 5, locations, dCircleRadius))
	{
		for (int i = 0; i < 5; i++)
		{
			Point2f ver = locations[i];
			double dDist = qSqrt(qPow(ver.x - imgPosX, 2) + qPow(ver.y - imgPosY, 2));
			if (dDist <= dCircleRadius * 2)
			{
				return i;
			}
		}
	}

	return -1;
}

bool QVLMaskEditor::rangeSelect(double mousePosX, double mousePosY, double& dScaleX, double& dScaleY)
{
	if (m_nSelectIndex < 0) return false;

	cv::Mat matImage = m_hoImage;

	double imgStartPosX = convertToImgX(m_startX);
	double imgStartPosY = convertToImgY(m_startY);

	double imgPosX = convertToImgX(mousePosX);
	double imgPosY = convertToImgY(mousePosY);

	double angleMouse = qAtan2((double)(imgPosY - imgStartPosY), (double)(imgPosX - imgStartPosX));
	double mousedist = qSqrt(qPow(imgPosX - imgStartPosX, 2) + qPow(imgPosY - imgStartPosY, 2));

	Point2f vertices[4];
	m_selectRects[m_nSelectIndex].points(vertices);
	double width = m_selectRects[m_nSelectIndex].size.width;
	double height = m_selectRects[m_nSelectIndex].size.height;
	double angle = m_selectRects[m_nSelectIndex].angle*M_PI / 180;

	int nIndex = getLocFromSelect(m_startX, m_startY);
	if (nIndex > -1 && nIndex < 4)
	{
		double angleSize = angleMouse - angle;
		double xRange = mousedist*qCos(angleSize) * 2;
		double yRange = mousedist*qSin(angleSize) * 2;

		if (0 == nIndex)
		{
			dScaleX = 1 - xRange / width;
			dScaleY = 1 + yRange / height;
		}
		else if (1 == nIndex)
		{
			dScaleX = 1 - xRange / width;
			dScaleY = 1 - yRange / height;
		}
		else if (2 == nIndex)
		{
			dScaleX = 1 + xRange / width;
			dScaleY = 1 - yRange / height;
		}
		else if (3 == nIndex)
		{
			dScaleX = 1 + xRange / width;
			dScaleY = 1 + yRange / height;
		}
		
		return true;
	}	

	return false;
}

bool QVLMaskEditor::rotateSelect(double mousePosX, double mousePosY, double& dRotatedAngle)
{
	if (m_nSelectIndex < 0) return false;

	cv::Mat matImage = m_hoImage;

	Point2f vertices[4];
	m_selectRects[m_nSelectIndex].points(vertices);
	double width = m_selectRects[m_nSelectIndex].size.width;
	double height = m_selectRects[m_nSelectIndex].size.height;
	double angle = m_selectRects[m_nSelectIndex].angle*M_PI / 180;
	Point2f ptCenter = m_selectRects[m_nSelectIndex].center;

	double imgStartPosX = convertToImgX(m_startX);
	double imgStartPosY = convertToImgY(m_startY);

	double imgPosX = convertToImgX(mousePosX);
	double imgPosY = convertToImgY(mousePosY);

	double angleStart = qAtan2((double)(imgStartPosY - ptCenter.y), (double)(imgStartPosX - ptCenter.x));
	double angleMouse = qAtan2((double)(imgPosY - ptCenter.y), (double)(imgPosX - ptCenter.x));	

	int nIndex = getLocFromSelect(m_startX, m_startY);
	if (nIndex > -1 && nIndex == 4)
	{
		dRotatedAngle = (angleMouse - angleStart) *180.0 / M_PI;
		return true;
	}

	//double imgStartPosX = convertToImgX(m_startX);
	//double imgStartPosY = convertToImgY(m_startY);

	//double imgPosX = convertToImgX(mousePosX);
	//double imgPosY = convertToImgY(mousePosY);

	//double angleMouse = qAtan2((double)(imgPosY - imgStartPosY), (double)(imgPosX - imgStartPosX));
	//double mousedist = qSqrt(qPow(imgPosX - imgStartPosX, 2) + qPow(imgPosY - imgStartPosY, 2));

	//Point2f vertices[4];
	//m_selectRects[m_nSelectIndex].points(vertices);
	//double width = m_selectRects[m_nSelectIndex].size.width;
	//double height = m_selectRects[m_nSelectIndex].size.height;
	//double angle = m_selectRects[m_nSelectIndex].angle*M_PI / 180;

	//int nIndex = getLocFromSelect(m_startX, m_startY);
	//if (nIndex > -1 && nIndex == 4)
	//{
	//	double angleSize = angleMouse - angle;
	//	double xRange = mousedist*qCos(angleSize) * 2;
	//	double yRange = mousedist*qSin(angleSize) * 2;

	//	if (angle > -M_PI * 3 / 4 && angle <= -M_PI * 1/ 4)
	//	{
	//		dRotatedAngle = 90 - qAtan2(height, yRange) *180.0 / M_PI;
	//	}
	//	else if (angle > -M_PI * 1 / 4 && angle <= M_PI * 1 / 4)
	//	{
	//		dRotatedAngle = 90 - qAtan2(height, xRange) *180.0 / M_PI;
	//	}
	//	else if (angle > M_PI * 1 / 4 && angle <= M_PI * 3 / 4)
	//	{

	//		dRotatedAngle = 90 - qAtan2(height, yRange) *180.0 / M_PI;
	//	}
	//	else
	//	{
	//		dRotatedAngle = 90 - qAtan2(height, xRange) *180.0 / M_PI;
	//	}

	//	//dRotatedAngle = 90 - qAtan2( height, xRange) *180.0 / M_PI;
	//	//dRotatedAngle = 90 - qAtan2(height, yRange) *180.0 / M_PI;
	//	return true;
	//}

	return false;
}

void QVLMaskEditor::drawSelectRect(cv::RotatedRect& selectRect)
{
	cv::Mat matImage = m_hoImage.clone();

	//cv::Mat imgLayer = cv::Mat::ones(matImage.rows, matImage.cols, CV_8UC3);
	cv::Mat imgLayer(matImage.rows, matImage.cols, CV_8UC3, Scalar(231, 191, 200));

	double alpha = 0.7;
	addWeighted(matImage, alpha, imgLayer, 1 - alpha, 0, matImage);


	Point2f vertices[4];
	selectRect.points(vertices);

	Point2f locations[5]; double dCircleRadius = 0;
	if (getRotRectLocationPt(matImage, 4, vertices, 5, locations, dCircleRadius))
	{
		for (int i = 0; i < 4; i++)
		{			
			line(matImage, locations[i], locations[(i + 1) % 4], Scalar(0, 255, 0), 2);
		}

		for (int i = 0; i < 4; i++)
		{
			circle(matImage, locations[i], dCircleRadius - 0, Scalar(255, 255, 255), -1);
			circle(matImage, locations[i], dCircleRadius, Scalar(0, 0, 0), 1);
		}

		double XVt = (vertices[1].x + vertices[2].x) / 2;
		double YVt = (vertices[1].y + vertices[2].y) / 2;
		line(matImage, Point2f(XVt, YVt), Point2f(locations[4].x, locations[4].y), Scalar(0, 255, 0), 2);

		circle(matImage, Point2f(locations[4].x, locations[4].y), dCircleRadius - 1, Scalar(255, 255, 255), -1);
		circle(matImage, Point2f(locations[4].x, locations[4].y), dCircleRadius, Scalar(0, 0, 0), 1);
	}
	//m_ActionMove->setEnabled(true);
	//m_ActionDelete->setEnabled(true);

	displayImage(matImage);
}

void QVLMaskEditor::drawSelectRectRotated(cv::RotatedRect& selectRect, double dRotatedAngle)
{
	cv::Mat matImage = m_hoImage.clone();

	//cv::Mat imgLayer = cv::Mat::ones(matImage.rows, matImage.cols, CV_8UC3);
	cv::Mat imgLayer(matImage.rows, matImage.cols, CV_8UC3, Scalar(231, 191, 200));

	double alpha = 0.7;
	addWeighted(matImage, alpha, imgLayer, 1 - alpha, 0, matImage);


	Point2f vertices[4];
	selectRect.points(vertices);

	Point2f locations[5]; double dCircleRadius = 0;
	if (getRotRectLocationPt(matImage, 4, vertices, 5, locations, dCircleRadius))
	{
		for (int i = 0; i < 4; i++)
		{
			line(matImage, locations[i], locations[(i + 1) % 4], Scalar(0, 255, 0), 1);
		}

		for (int i = 0; i < 4; i++)
		{
			circle(matImage, locations[i], dCircleRadius - 0, Scalar(255, 255, 255), -1);
			circle(matImage, locations[i], dCircleRadius, Scalar(0, 0, 0), 1);
		}

		double XVt = (vertices[1].x + vertices[2].x) / 2;
		double YVt = (vertices[1].y + vertices[2].y) / 2;
		line(matImage, Point2f(XVt, YVt), Point2f(locations[4].x, locations[4].y), Scalar(0, 255, 0), 1);

		circle(matImage, Point2f(locations[4].x, locations[4].y), dCircleRadius - 1, Scalar(255, 255, 255), -1);
		circle(matImage, Point2f(locations[4].x, locations[4].y), dCircleRadius, Scalar(0, 0, 0), 1);
	}

	// draw roated angle
	int radious = selectRect.size.height / 2 + selectRect.size.width / 5;

	Rect rectTmp = Rect(selectRect.center.x - radious, selectRect.center.y - radious, radious * 2, radious * 2);
	Point ptTmp  = Point(radious, radious);
	if (rectTmp.x < 0 || rectTmp.y < 0 || (rectTmp.x + rectTmp.width) > matImage.size().width || (rectTmp.y + rectTmp.height) > matImage.size().height)
	{
		if (rectTmp.x < 0)
		{
			ptTmp.x += rectTmp.x;
			rectTmp.x = 0;
		}
		if (rectTmp.y < 0)
		{
			ptTmp.y += rectTmp.y;
			rectTmp.y = 0;
		}
		//rectTmp.x = rectTmp.x < 0 ? 0 : rectTmp.x;
		//rectTmp.y = rectTmp.y < 0 ? 0 : rectTmp.y;
		rectTmp.width = (rectTmp.x + rectTmp.width) > matImage.size().width ? (matImage.size().width - rectTmp.x) : rectTmp.width;
		rectTmp.height = (rectTmp.y + rectTmp.height) > matImage.size().height ? (matImage.size().height - rectTmp.y) : rectTmp.height;
	}

	cv::Mat matRect = matImage(rectTmp);
	imgLayer = matRect.clone();

	ellipse(imgLayer, ptTmp, cv::Size(radious, radious), -90, selectRect.angle - dRotatedAngle, selectRect.angle, Scalar(255, 0, 0), -1, 8, 0);

	alpha = 0.7;
	addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);

	circle(matImage, selectRect.center, dCircleRadius - 1, Scalar(255, 255, 255), -1);
	circle(matImage, selectRect.center, dCircleRadius, Scalar(0, 0, 0), 1);

	Point ptPos = Point(selectRect.center.x + radious / 6, selectRect.center.y);
	addImageText(matImage, ptPos, QString::number(dRotatedAngle, 'f', 1) /*+ QStringLiteral("°")*/);


	displayImage(matImage);
}


MaskType QVLMaskEditor::getMaskType()
{
	MaskType maskType = MASK_TYPE_NULL;
	if (ui.radioButton_rect->isChecked())
	{
		maskType = MASK_TYPE_RECT;
	}
	else if (ui.radioButton_circle->isChecked())
	{
		maskType = MASK_TYPE_CIRCLE;
	}
	else if (ui.radioButton_polyline->isChecked())
	{
		maskType = MASK_TYPE_POLYLINE;
	}

	return maskType;
}

void QVLMaskEditor::unSelectMask()
{
	for (int i = 0; i < m_maskObjs.size(); i++)
	{
		m_maskObjs.at(i)->setSelect(false);
	}
	//ui.comboBox_maskIndex->setCurrentIndex(0);
}

void QVLMaskEditor::clear()
{
	m_selectRects.clear();

	for (int i = 0; i < m_maskObjs.size(); i++)
	{
		delete m_maskObjs[i];
	}
	m_maskObjs.clear();

	m_nSelectIndex = -1;
	
	m_hoImage.release();
}


void QVLMaskEditor::addSelect(RotatedRect& rect)
{
	m_selectRects.push_back(rect);
}

void QVLMaskEditor::onSelectIndexChanged(int iIndex)
{
	int nSelectIndex = ui.comboBox_selectIndex->currentIndex();

	if (nSelectIndex >= 1 && nSelectIndex <= m_selectRects.size())
	{
		drawSelectRect(m_selectRects[nSelectIndex - 1]);
		m_nSelectIndex = nSelectIndex - 1;
	}
}

void QVLMaskEditor::onAddSelect()
{
	setViewState(MODE_VIEW_ADD_SELECT);
}

void QVLMaskEditor::onDeleteSelect()
{
	int nSelectIndex = ui.comboBox_selectIndex->currentIndex();

	if (nSelectIndex >= 1 && nSelectIndex <= m_selectRects.size())
	{
		m_selectRects.erase(m_selectRects.begin() + nSelectIndex - 1);
		m_nSelectIndex = -1;

		ui.comboBox_selectIndex->removeItem(ui.comboBox_selectIndex->count() - 1);
		ui.comboBox_selectIndex->setCurrentIndex(0);

		repaintAll();
	}
}

void QVLMaskEditor::onMaskIndexChanged(int iIndex)
{
	int nMaskIndex = ui.comboBox_maskIndex->currentIndex();

	if (nMaskIndex >= 1 && nMaskIndex <= m_maskObjs.size())
	{
		unSelectMask();
		m_maskObjs.at(nMaskIndex - 1)->setSelect(true);		
		repaintAll();
	}
}

void QVLMaskEditor::onAddMask()
{
	setViewState(MODE_VIEW_ADD_MASK);
	ui.comboBox_maskIndex->setCurrentIndex(0);
	unSelectMask();
}

void QVLMaskEditor::onDeleteMask()
{
	int nMaskIndex = ui.comboBox_maskIndex->currentIndex();

	if (nMaskIndex >= 1 && nMaskIndex <= m_maskObjs.size())
	{
		delete m_maskObjs[nMaskIndex - 1];
		m_maskObjs.erase(m_maskObjs.begin() + nMaskIndex - 1);

		ui.comboBox_maskIndex->removeItem(ui.comboBox_maskIndex->count() - 1);
		ui.comboBox_maskIndex->setCurrentIndex(0);

		unSelectMask();
		repaintAll();
	}
}

void QVLMaskEditor::actionMove()
{

}

void QVLMaskEditor::actionDelete()
{

}
