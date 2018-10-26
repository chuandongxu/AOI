#include "GL/glut.h"

#include "ViewWidget.h"
#include <qmath.h>

//#if defined(__APPLE_CC__) || defined(_APPLE_CC)
//#include <OpenGL/glu.h>
//#else
//#include <gl/glu.h>
//#endif

#include "./core/CSGWorld.h"
#include "./core/VectorSet.h"
#include "./core/BooleanModeller.h"

#include"RoamingScenceManager.h"
#include "CSGMesh.h"

#include<QMouseEvent>
#include<QWheelEvent>
#include<QDebug>
#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <fstream>

#include "../CurveEditor/IGraphicEditor.h"
#include "GLFont.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IVision.h"

#include"MathAPIKernel/Vector3D.h"
#include"MathAPIKernel/Position3D.h"
#include"MathAPIKernel/Matrix4D.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define VIEW_POINT_XY_SCALE 1.5
#define VIEW_POINT_Z_SCALE 1.2

using namespace cv;

ViewWidget::ViewWidget(QWidget *parent /* = 0 */)
	: QGLWidget(parent)
{
	m_width = this->geometry().width();
	m_height = this->geometry().height();
	m_nLeft = this->geometry().left();
	m_nTop = this->geometry().top();

	this->setFocusPolicy(Qt::StrongFocus);
	manager = new RoamingScenceManager(this);

	m_csgWorld = new CSGWorld();
	m_pBooleanModeller = NULL;
	m_pDifferenceModel = NULL;

	m_csgMesh = new CSGMesh();

	setViewAction(ACTION_SELECT);

	m_pBooleanModeller = NULL;
	m_pDifferenceModel = NULL;

	m_bShowCloudPoint = true;
	m_bShowMeshTriangulation = false;
	m_bShowMeshSurfaceCute = false;

	m_pGraphicsEditor = new IGraphicEditor();
	m_pGraphicsEditor->setViewPos(0, 0);
	m_pGraphicsEditor->setScale(16, 50);

	m_idCutMeshLine = 0;
	m_idCutMesh = 0;

    OldMouse = new Position3D();
    Mouse = new Position3D();

	setMouseTracking(true);
}

ViewWidget::~ViewWidget()
{
	delete manager;
	delete m_csgWorld;

	if(m_pBooleanModeller)
		delete m_pBooleanModeller;
	if (m_pDifferenceModel)
		delete m_pDifferenceModel;

	delete m_csgMesh;

	if (m_pGraphicsEditor)
	{
		delete m_pGraphicsEditor;
		m_pGraphicsEditor = NULL;
	}

    delete OldMouse;
    delete Mouse;
}

//void ViewWidget::paintEvent(QPaintEvent *event)
//{
//	QPainter painter(this);
//
//	if (ACTION_CSG_MESH == m_viewAction)
//	{
//		painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
//		painter.setBrush(Qt::NoBrush);
//		
//		QPointF ptTL, ptBR;
//		ptTL.setX(m_nPosXPre - 5);
//		ptTL.setY(m_nPosYPre - 5);
//		ptBR.setX(m_nPosXPre + 5);
//		ptBR.setY(m_nPosYPre + 5);
//
//		painter.drawLine(QPointF(m_nPosXPre, m_nPosYPre), QPointF(0,0));		
//	}
//}

void ViewWidget::initializeGL()
{
	CGameFont::SetDeviceContext(wglGetCurrentDC());

	manager->init();
	m_csgMesh->init();
	m_csgWorld->init();

	QString path = QApplication::applicationDirPath();
	path += "/3D/data/";
	QString fileName = path + "x.raw";
	loadFile(fileName);

	/*m_csgMesh->loadData(path + "x.csv", path + "y.csv", path + "z.csv", "csv");
	m_csgMesh->convetToMeshTriangulationExt();

	m_csgMesh->getLimit(m_dMeshSizeMaxX, m_dMeshSizeMinX, m_dMeshSizeMaxY, m_dMeshSizeMinY, m_dMeshSizeMaxZ, m_dMeshSizeMinZ);

	VectorSet* vertices = m_csgMesh->getVerticesMeshData();  IntSet* indices = m_csgMesh->getIndicesMeshData();
	if (vertices != NULL && indices != NULL)
	{
		m_csgWorld->LoadObj(vertices, indices, m_dMeshSizeMaxZ, m_dMeshSizeMinZ);
		m_csgWorld->LoadCube();

		if (!m_pBooleanModeller)
			m_pBooleanModeller = new BooleanModeller(m_csgWorld->GetSolid(0), m_csgWorld->GetSolid(1));

		if (!m_pDifferenceModel)
		{
			m_pDifferenceModel = m_pBooleanModeller->getDifference(false);			
		}	
	}*/	
}

void ViewWidget::paintGL()
{
	manager->render();	

	if (m_bShowCloudPoint)
	{
		m_csgMesh->Render();
	}

	if (m_bShowMeshTriangulation)
	{
		if(m_csgWorld->isBindTexture()) glEnable(GL_TEXTURE_2D);
		m_csgWorld->Render(1);
		glDisable(GL_TEXTURE_2D);
	}

	if (m_bShowMeshSurfaceCute)
	{
		if (isCSGBoolAlg())
		{
			m_pDifferenceModel->Render();
		}
	}

	if (m_idCutMeshLine > 0)
		glCallList(m_idCutMeshLine);

	if (m_idCutMesh > 0)
		glCallList(m_idCutMesh);
}

void ViewWidget::resizeGL(int width, int height)
{
	m_width = width;
	m_height = height;
	glViewport(0, -(width - height) / 2, width, width);
}

void ViewWidget::mousePressEvent(QMouseEvent *event)
{
	int posX = event->x(); int posY = event->y();
	manager->getInitPos(posX, posY);

	m_nPosXPre = posX;
	m_nPosYPre = posY;

	m_nPosXStart = posX;
	m_nPosYStart = posY;

	if (event->buttons() & Qt::LeftButton)
	{
		if (ACTION_SELECT == m_viewAction)
		{

		}
		else if (ACTION_CSG_MESH == m_viewAction)
		{			
		}
	}
}

void ViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
	int posX = event->x(); int posY = event->y();
	if (event->button() & Qt::LeftButton)
	{	
		if (ACTION_MOVE == m_viewAction)
		{
			// having cutting mesh
			if (m_idCutMesh > 0 && m_nCutMeshSelect > -1)
			{
				double x1 = m_cutMeshP1.x;
				double y1 = m_cutMeshP1.y;
				double z1 = m_cutMeshP1.z;

				double x2 = m_cutMeshP2.x;
				double y2 = m_cutMeshP2.y;
				double z2 = m_cutMeshP2.z;

				if (isCSGBoolAlg())
				{
					cutingUsingCSGCube(x1, y1, x2, y2);
				}
				if (isBilinearInterpolationAlg())
				{
					cutingUsingBilinearInterpolation(x1, y1, x2, y2);
				}
				else if (is3DUnprojectAlg())
				{
					cutingUsing3DUnproject(x1, y1, x2, y2);
				}

				updateGL();
			}
		}
		else if (ACTION_CSG_MESH == m_viewAction)
		{
			cutingMesh(m_nPosXStart, m_nPosYStart, posX, posY);
			updateGL();

			updateCutingData(m_nPosXStart, m_nPosYStart, posX, posY);
		}		
	}
	else if (event->button() & Qt::RightButton)
	{
		setViewAction(ACTION_SELECT);
	}	
}

void ViewWidget::mouseMoveEvent(QMouseEvent *event)
{
	int posX = event->x(); int posY = event->y();
	if (event->buttons() & Qt::LeftButton)
	{
		if (ACTION_MOVE == m_viewAction)
		{
			// having cutting mesh
			if (m_idCutMesh > 0 && m_nCutMeshSelect > -1)
			{
				//double x1, y1;				
				//ScreenPtToViewPtExt(posX, posY, m_cutMeshP1.z, &x1, &y1);

				//Vector pt1 = m_cutMeshP1; Vector pt2 = m_cutMeshP2;
				//if (0 == m_nCutMeshSelect)
				//{
				//	pt1.x = x1;
				//	pt1.y = y1;
				//}
				//else if (1 == m_nCutMeshSelect)
				//{
				//	pt2.x = x1;
				//	pt2.y = y1;
				//}
				//changeCutingMesh(pt1, pt2);

				double xMoved = 0, yMoved = 0;

				double x1, y1, x2, y2;
				ScreenPtToViewPtExt(m_nPosXPre, m_nPosYPre, m_cutMeshP1.z, &x1, &y1);
				ScreenPtToViewPtExt(posX, posY, m_cutMeshP1.z, &x2, &y2);

				xMoved = (x2 - x1);
				yMoved = (y2 - y1);

				Vector pt1 = m_cutMeshP1; Vector pt2 = m_cutMeshP2;
				if (0 == m_nCutMeshSelect)
				{
					pt1.x += xMoved;
					pt1.y += yMoved;
					//pt1.z += (z2 - z1);
				}
				else if (1 == m_nCutMeshSelect)
				{
					pt2.x += xMoved;
					pt2.y += yMoved;
					//pt2.z += (z2 - z1);
				}
				changeCutingMesh(pt1, pt2);
			}
			else
			{
				manager->executeTranslateOperation(posX, posY);
			}

			updateGL();
		}
		else if (ACTION_ROTATE == m_viewAction)
		{
			//manager->executeRotateOperation(posX, posY);
            excuteRotate(posX, posY);

			updateGL();
		}
		else if (ACTION_CSG_MESH == m_viewAction)
		{
			double idx, idy, dx, dy, dz;

			idx = m_nPosXStart;
			idy = m_nPosYStart;
			ScreenPtToViewPt(idx, idy, &dx, &dy, &dz);
			double x1 = dx;
			double y1 = dy;
			double z1 = dz;

			idx = posX;
			idy = posY;
			ScreenPtToViewPt(idx, idy, &dx, &dy, &dz);
			double x2 = dx;
			double y2 = dy;
			double z2 = dz;

			if (m_idCutMeshLine > 0)
			{
				glDeleteLists(m_idCutMeshLine, 1);
				m_idCutMeshLine = 0;
			}
			m_idCutMeshLine = solidLine(x1, y1, dx, dy);

			updateGL();
		}	
	}
	else if (event->buttons() & Qt::MiddleButton)
	{
		//manager->executeRotateOperation(posX, posY);
		updateGL();
	}
	else
	{
		if (ACTION_MOVE == m_viewAction)
		{
			// having cutting mesh
			if (m_idCutMesh > 0)
			{
				double idx, idy, dx, dy, dz;

				idx = posX;
				idy = posY - 22;
				ScreenPtToViewPt(idx, idy, &dx, &dy, &dz);

				Vector viewPt;
				viewPt.x = dx;
				viewPt.y = dy;
				viewPt.z = dz;

				QString path = QApplication::applicationDirPath();
				path += "/image/";
				QCursor cursor;				

				if (isPointNearPoint(viewPt, m_cutMeshP1, 150))
				{	
					QPixmap pixmap(path + "hsizeHand.png");
					cursor = QCursor(pixmap, -1, -1);
					setCursor(cursor);

					m_nCutMeshSelect = 0;					
				}
				else if (isPointNearPoint(viewPt, m_cutMeshP2, 150))
				{
					QPixmap pixmap(path + "hsizeHand.png");
					cursor = QCursor(pixmap, -1, -1);
					setCursor(cursor);

					m_nCutMeshSelect = 1;				
				}
				else
				{				
					QPixmap pixmap(path + "moveHand.png");
					cursor = QCursor(pixmap, -1, -1);
					setCursor(cursor);

					m_nCutMeshSelect = -1;
				}
			}
		}
	}

	m_nPosXPre = posX;
	m_nPosYPre = posY;
}

void ViewWidget::excuteRotate(int x, int y)
{
    Mouse->setX(x);
    Mouse->setY(y);

    if ((fabs(OldMouse->X()) >= 0.01) && (fabs(OldMouse->X()) >= 0.01))
    {
        Vector3D rotateAxis; rotateAxis.setX(0); rotateAxis.setY(0); rotateAxis.setZ(1.0);
        Vector3D MouseTrace = rotateAxis*(Mouse->X() - OldMouse->X())*0.02;
        rotateAxis.normalize();

        float angle = (Mouse->X() - OldMouse->X()) > 0 ? -MouseTrace.length() : MouseTrace.length();       

        m_csgMesh->rotate(rotateAxis, angle);

        m_csgWorld->rotate(1, Vector(0, 0, 1), Vector(0, 0, -1), angle);
    }

    //更新鼠标
    *OldMouse = *Mouse;
}

void ViewWidget::wheelEvent(QWheelEvent *e)
{
	if (e->delta() >= 0)
	{
		manager->executeScaleOperation(-0.1);
	}
	else
	{
		manager->executeScaleOperation(0.1);
	}
	updateGL();
}

void ViewWidget::cutingMesh(double dPosX1, double dPosY1, double dPosX2, double dPosY2)
{
	double idx, idy, dx, dy, dz;

	idx = dPosX1;
	idy = dPosY1;
	ScreenPtToViewPt(idx, idy, &dx, &dy, &dz);
	double x1 = dx;
	double y1 = dy;
	double z1 = dz;

	idx = dPosX2;
	idy = dPosY2;
	ScreenPtToViewPt(idx, idy, &dx, &dy, &dz);
	double x2 = dx;
	double y2 = dy;
	double z2 = dz;

	if (isCSGBoolAlg())
	{
		cutingUsingCSGCube(x1, y1, x2, y2);

		m_bShowMeshTriangulation = false;
		m_bShowMeshSurfaceCute = true;
	}
	if (isBilinearInterpolationAlg())
	{
		cutingUsingBilinearInterpolation(x1, y1, x2, y2);
		m_bShowMeshTriangulation = true;
		m_bShowMeshSurfaceCute = false;
	}
	else if (is3DUnprojectAlg())
	{
		cutingUsing3DUnproject(x1, y1, x2, y2);
		m_bShowMeshTriangulation = true;
		m_bShowMeshSurfaceCute = false;
	}

	fullScreen();	

	if (m_idCutMesh > 0)
	{
		glDeleteLists(m_idCutMesh, 1);
		m_idCutMesh = 0;
	}
	m_idCutMesh = solidPlane(x1, y1, z1, x2, y2, z2);

	m_nCutMeshSelect = -1;

	if (m_idCutMeshLine > 0)
	{
		glDeleteLists(m_idCutMeshLine, 1);
		m_idCutMeshLine = 0;
	}		
}

//cv::Mat ViewWidget::QImage2cvMat(QImage image)
//{
//	cv::Mat mat;
//	switch (image.format())
//	{
//	case QImage::Format_ARGB32:
//	case QImage::Format_RGB32:
//	case QImage::Format_ARGB32_Premultiplied:
//		mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
//		break;
//	case QImage::Format_RGB888:
//		mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
//		cv::cvtColor(mat, mat, CV_BGR2RGB);
//		break;
//	case QImage::Format_Indexed8:
//		mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
//		break;
//	}
//	return mat;
//}

void ViewWidget::changeCutingMesh(Vector pt1, Vector pt2)
{
	double x1 = pt1.x;
	double y1 = pt1.y;
	double z1 = pt1.z;

	double x2 = pt2.x;
	double y2 = pt2.y;
	double z2 = pt2.z;

	if (m_idCutMesh > 0)
	{
		glDeleteLists(m_idCutMesh, 1);
		m_idCutMesh = 0;
	}
	m_idCutMesh = solidPlane(x1, y1, z1, x2, y2, z2);
}

bool ViewWidget::writeBMP(const char filename[], unsigned char* data, unsigned int w, unsigned int h)
{
	std::ofstream out_file;

	/** 检查data */
	if (!data)
	{
		qDebug() << "data corrupted! ";
		out_file.close();
		return false;
	}

	/** 创建位图文件信息和位图文件头结构 */
	BITMAPFILEHEADER header;
	BITMAPINFOHEADER bitmapInfoHeader;

	//unsigned char textureColors = 0;/**< 用于将图像颜色从BGR变换到RGB */

	/** 打开文件,并检查错误 */
	out_file.open(filename, std::ios::out | std::ios::binary);
	if (!out_file)
	{
		qDebug() << "Unable to open file ";
		return false;
	}

	/** 填充BITMAPFILEHEADER */
	header.bfType = 0x4d42;
	header.bfSize = w*h * 3 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	/** 写入位图文件头信息 */
	out_file.write((char*)&header, sizeof(BITMAPFILEHEADER));

	/** 填充BITMAPINFOHEADER */
	bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfoHeader.biWidth = w;
	bitmapInfoHeader.biHeight = h;
	bitmapInfoHeader.biPlanes = 1;
	bitmapInfoHeader.biBitCount = 24;
	bitmapInfoHeader.biCompression = BI_RGB; // BI_RLE4 BI_RLE8
	bitmapInfoHeader.biSizeImage = w * h * 3; // 当压缩类型为BI_RGB是也可以设置为0
	bitmapInfoHeader.biXPelsPerMeter = 0;
	bitmapInfoHeader.biYPelsPerMeter = 0;
	bitmapInfoHeader.biClrUsed = 0;
	bitmapInfoHeader.biClrImportant = 0;
	/** 写入位图文件信息 */
	out_file.write((char*)&bitmapInfoHeader, sizeof(BITMAPINFOHEADER));

	/** 将指针移到数据开始位置 */
	out_file.seekp(header.bfOffBits, std::ios::beg);

	/** 写入图像数据 */
	out_file.write((char*)data, bitmapInfoHeader.biSizeImage);

	out_file.close();
	return true;
}

void ViewWidget::updateCutingData(double x1, double y1, double x2, double y2)
{
	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (pVision)
	{
		GLint iViewport[4]; // 视图大小
		glGetIntegerv(GL_VIEWPORT, iViewport);

		int x = 0;
		int y = 0;
		int width = iViewport[2];
		int height = iViewport[3] * m_height / m_width;

		int nRowWidth = width * 3;
		int PixelDataLength = nRowWidth * height;

		GLbyte* pBuffer = (GLbyte*)(new uchar[PixelDataLength]);
		// Read Image from buffer
		//glReadBuffer(GL_FRONT);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_SKIP_ROWS, 0);
		glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

		GLenum lastBuffer;
		glGetIntegerv(GL_READ_BUFFER, (GLint*)&lastBuffer);
		glReadBuffer(GL_FRONT);
		glReadPixels(x, y, width, height, GL_RGB/*GL_BGR_EXT*/, GL_UNSIGNED_BYTE, pBuffer);
		glReadBuffer(lastBuffer);

		//QString path = QApplication::applicationDirPath();
		//path += "/output/";
		//QString fileName = path + "test.bmp";
		//writeBMP(fileName.toStdString().c_str(), (unsigned char*)pBuffer, width, height);

		QImage image((uchar*)pBuffer, width, height, QImage::Format_RGB888);
		// flip the image.
		QMatrix matrix;
		matrix.scale(1, -1);
		image = image.transformed(matrix);

		pVision->setProf3DImg(image);

		double dScaleW = (double)width / m_width;
		double dScaleH = (double)height / m_height;	
		pVision->setProfRange(x1 * dScaleW, y1 * dScaleH, x2 * dScaleW, y2 * dScaleH);

		pVision->prepareNewProf();

		delete[] pBuffer;
	}

	/*double idx, idy, dx, dy, dz;

	idx = x1;
	idy = y1;
	ScreenPtToViewPt(idx, idy, &dx, &dy, &dz);
	double ox1 = dx + (m_dMeshSizeMaxX - m_dMeshSizeMinX) / 2;
	double oy1 = dy + (m_dMeshSizeMaxY - m_dMeshSizeMinY) / 2;;
	double oz1 = dz;

	idx = x2;
	idy = y2;
	ScreenPtToViewPt(idx, idy, &dx, &dy, &dz);
	double ox2 = dx + (m_dMeshSizeMaxX - m_dMeshSizeMinX) / 2;;
	double oy2 = dy + (m_dMeshSizeMaxY - m_dMeshSizeMinY) / 2;;
	double oz2 = dz;

	pVision->setProfRange(ox1, oy1, ox2, oy2);*/
}

void ViewWidget::solidCylinder(GLdouble radius, GLdouble height, GLint slices, GLint stacks) {

	GLUquadricObj* qobj = gluNewQuadric();

	gluCylinder(qobj, radius, radius, height, slices, stacks);
	glScalef(-1.0f, 1.0f, -1.0f);
	gluDisk(qobj, 0.0, radius, slices, stacks);
	glScalef(-1.0f, 1.0f, -1.0f);
	glTranslatef(0.0f, 0.0f, static_cast<GLfloat>(height));
	gluDisk(qobj, 0.0, radius, slices, stacks);

	gluDeleteQuadric(qobj);
}

GLuint ViewWidget::solidLine(double posX1, double posY1, double posX2, double posY2)
{
	GLuint glID = glGenLists(1);
	glNewList(glID, GL_COMPILE);
	glPushMatrix();

	int nHeight = m_dMeshSizeMaxZ;
	double dSize = (m_dMeshSizeMaxX - m_dMeshSizeMinX)*0.02;

	glTranslatef(0.0f, 0.0f, nHeight);
	glColor3f(1.0f, 1.0f, 1.0f);
	double x1 = posX1 - dSize;
	double y1 = posY1 - dSize;
	double x2 = posX1 + dSize;
	double y2 = posY1 + dSize;
	glRectf(x1, y1, x2, y2);
	glTranslatef(0.0f, 0.0f, -nHeight);

	glLineWidth(2.0f);
	glLineStipple(2, 0x0f0f);//线条样式
	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_LINE_STIPPLE);

	glBegin(GL_LINES);
	glEnable(GL_LINE_SMOOTH);
	glVertex3f(posX1, posY1, nHeight);
	glVertex3f(posX2, posY2, nHeight);
	glEnd();
	glDisable(GL_LINE_STIPPLE);

	glLineWidth(1.0f);

	glTranslatef(0.0f, 0.0f, nHeight);
	x1 = posX2 - dSize;
	y1 = posY2 - dSize;
	x2 = posX2 + dSize;
	y2 = posY2 + dSize;
	glRectf(x1, y1, x2, y2);
	glTranslatef(0.0f, 0.0f, -nHeight);

	glPopMatrix();
	glEndList();	

	return glID;
}

GLuint ViewWidget::solidPlane(double posX1, double posY1, double posZ1, double posX2, double posY2, double posZ2)
{
	double dExtlen = qAbs(posX2 - posX1)*0.1;

	double posX[2] = { posX1, posX2 };
	double posY[2] = { posY1, posY2 };
	double posExtX[2] = { 0 };
	double posExtY[2] = { 0 };

	//getLineLimitPos(posX, posY, dExtlen, posExtX, posExtY);
	posExtX[0] = posX1; posExtX[1] = posX2;
	posExtY[0] = posY1; posExtY[1] = posY2;
	
	//double angle = qAtan2((double)(Y2 - Y1), (double)(X2 - X1));
	//double cX = (X1 + X2) / 2;
	//double cY = (Y1 + Y2) / 2;

	GLuint glID = glGenLists(1);
	glNewList(glID, GL_COMPILE);
	glPushMatrix();	

	glLineWidth(1.0f);
	glLineStipple(2, 0xffff);//线条样式
	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);
	glEnable(GL_LINE_SMOOTH);

	glVertex3f(m_dMeshSizeMinX, m_dMeshSizeMinY, m_dMeshSizeMaxZ);
	glVertex3f(m_dMeshSizeMinX, m_dMeshSizeMaxY, m_dMeshSizeMaxZ);
	glVertex3f(m_dMeshSizeMinX, m_dMeshSizeMaxY, m_dMeshSizeMaxZ);
	glVertex3f(m_dMeshSizeMaxX, m_dMeshSizeMaxY, m_dMeshSizeMaxZ);
	glVertex3f(m_dMeshSizeMaxX, m_dMeshSizeMaxY, m_dMeshSizeMaxZ);
	glVertex3f(m_dMeshSizeMaxX, m_dMeshSizeMinY, m_dMeshSizeMaxZ);
	glVertex3f(m_dMeshSizeMaxX, m_dMeshSizeMinY, m_dMeshSizeMaxZ);
	glVertex3f(m_dMeshSizeMinX, m_dMeshSizeMinY, m_dMeshSizeMaxZ);

	glVertex3f(m_dMeshSizeMinX, m_dMeshSizeMinY, m_dMeshSizeMinZ);
	glVertex3f(m_dMeshSizeMinX, m_dMeshSizeMinY, m_dMeshSizeMaxZ);
	glVertex3f(m_dMeshSizeMinX, m_dMeshSizeMaxY, m_dMeshSizeMinZ);
	glVertex3f(m_dMeshSizeMinX, m_dMeshSizeMaxY, m_dMeshSizeMaxZ);
	glVertex3f(m_dMeshSizeMaxX, m_dMeshSizeMaxY, m_dMeshSizeMinZ);
	glVertex3f(m_dMeshSizeMaxX, m_dMeshSizeMaxY, m_dMeshSizeMaxZ);
	glVertex3f(m_dMeshSizeMaxX, m_dMeshSizeMinY, m_dMeshSizeMinZ);
	glVertex3f(m_dMeshSizeMaxX, m_dMeshSizeMinY, m_dMeshSizeMaxZ);
	
	glEnd();
	glDisable(GL_LINE_STIPPLE);
	glLineWidth(1.0f);	

	double height = qAbs(m_dMeshSizeMaxZ - m_dMeshSizeMinZ);
	double dMaxZ = m_dMeshSizeMaxZ + height*0.00;
	double dMinZ = m_dMeshSizeMinZ - height*0.00;
	double dMaxZExt = height * 0.08;

	GLUquadricObj *objCylinder = gluNewQuadric();

	glPushMatrix();
	glColor4ub(116, 153, 200, 255);
	glTranslatef(posExtX[0], posExtY[0], dMaxZ + dMaxZExt*1.2);
	glRotatef(180, 0.0, 1.0, 0.0);
	gluCylinder(objCylinder, dMaxZExt*0.4, 0.0, dMaxZExt, 10, 5);
	glPopMatrix();

	m_cutMeshP1.x = posExtX[0];
	m_cutMeshP1.y = posExtY[0];
	m_cutMeshP1.z = dMaxZ;

	/*	glColor3f(0.0f, 0.0f, 1.0f);

		double x1 = posX1 - 0.35f;
		double y1 = posY1 - 0.35f;
		double x2 = posX1 + 0.35f;
		double y2 = posY1 + 0.35f;
		glRectf(x1, y1, x2, y2);*/	

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);

	glBegin(GL_QUADS);
	glColor4ub(198, 251, 218, 200);
	glNormal3f(0, -1, 0);
	glVertex3f(posExtX[0], posExtY[0], dMinZ);
	glVertex3f(posExtX[1], posExtY[1], dMinZ);
	glVertex3f(posExtX[1], posExtY[1], dMaxZ);
	glVertex3f(posExtX[0], posExtY[0], dMaxZ);
	glEnd();

	glDisable(GL_BLEND);

	glPushMatrix();
	glColor4ub(116, 153, 200, 255);
	glTranslatef(posExtX[1], posExtY[1], dMaxZ + dMaxZExt*1.2);
	glRotatef(180, 0.0, 1.0, 0.0);
	gluCylinder(objCylinder, dMaxZExt*0.4, 0.0, dMaxZExt, 10, 5);
	glPopMatrix();

	m_cutMeshP2.x = posExtX[1];
	m_cutMeshP2.y = posExtY[1];
	m_cutMeshP2.z = dMaxZ;

	glLineWidth(1.0f);
	glLineStipple(2, 0x5555);//线条样式
	glColor3f(0.7f, 0.0f, 0.7f);
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);
	glEnable(GL_LINE_SMOOTH);

	glVertex3f(m_cutMeshP1.x, m_cutMeshP1.y, m_cutMeshP1.z);
	glVertex3f(m_cutMeshP2.x, m_cutMeshP2.y, m_cutMeshP2.z);

	glEnd();
	glDisable(GL_LINE_STIPPLE);
	glLineWidth(1.0f);

	//qDebug() << "Solid Pt1:" << m_cutMeshP1.x << " " << m_cutMeshP1.y << " " << m_cutMeshP1.z << endl;
	//qDebug() << "Solid Pt2:" << m_cutMeshP2.x << " " << m_cutMeshP2.y << " " << m_cutMeshP2.z << endl;

	//x1 = posX2 - 0.35f;
	//y1 = posY2 - 0.35f;
	//x2 = posX2 + 0.35f;
	//y2 = posY2 + 0.35f;
	//glRectf(x1, y1, x2, y2);	

	glPopMatrix();
	glEndList();

	return glID;
}

void ViewWidget::cutingUsingCSGCube(double posX1, double posY1, double posX2, double posY2)
{	
	m_csgWorld->GetSolid(1)->Reset();

	double x = 0;
	double y = 0;

	double X1 = posX1;
	double Y1 = posY1;
	double X2 = posX2;
	double Y2 = posY2;

	if (qAbs(X1 - X2) < 5 && qAbs(Y1 - Y2) < 5)
	{
		return;
	}

	double A = Y2 - Y1;
	double B = X1 - X2;
	double C = X2*Y1 - X1*Y2;

	double d = (A*x + B*y + C) / qSqrt(qPow(A,2) + qPow(B,2));

	Vector vector;
	vector.Set(0, d, 0);
	m_csgWorld->GetSolid(1)->Translate(vector);

	double angle = qAtan2((double)(posY2 - posY1), (double)(posX2 - posX1));

	Vector a, b;
	a.Set(0, 0, 0);
	b.Set(0, 0, 5);
	m_csgWorld->GetSolid(1)->Rotate(a, b, angle);

	if (m_pBooleanModeller)
	{
		delete m_pBooleanModeller;
		m_pBooleanModeller = NULL;
	}

	if (m_pDifferenceModel)
	{
		delete m_pDifferenceModel;
		m_pDifferenceModel = NULL;
	}

	if (!m_pBooleanModeller)
		m_pBooleanModeller = new BooleanModeller(m_csgWorld->GetSolid(0), m_csgWorld->GetSolid(1));

	if (!m_pDifferenceModel)
	{
		m_pDifferenceModel = m_pBooleanModeller->getDifference(false);
	}

	double slop = -(C / B);
	showCrossSectionAsCSG(angle, slop);
}

void ViewWidget::showCrossSectionAsCSG(double angle, double slop)
{
	m_pGraphicsEditor->clearSplines();

	std::vector<aaAaa::aaSpline> splines;
	aaAaa::aaSpline spline;
	if (m_pBooleanModeller)
	{
		VectorSet* vectors = m_pBooleanModeller->getFaceSplits();

		for (int i = 0; i < vectors->GetSize(); i++)
		{
			double x = vectors->GetVector(i).x;
			double y = vectors->GetVector(i).y;
			//double valuePos = qSqrt(qPow(x,2) + qPow(y,2));

			double len1 = slop * qCos(M_PI / 2 - angle);
			double len2 = (x / qCos(angle));

			spline.addKnots(aaAaa::aaPoint(len1 + len2, vectors->GetVector(i).z));
		}
	}

	spline.name = "Cross-Section";
	splines.push_back(spline);

	m_pGraphicsEditor->setSplines(splines);

	int nWidth = 0, nHeight = 0;
	m_pGraphicsEditor->getSize(nWidth, nHeight);

	m_pGraphicsEditor->setPos(m_nLeft - nWidth, m_nTop);
	m_pGraphicsEditor->show();
}

void ViewWidget::cutingUsingBilinearInterpolation(double posX1, double posY1, double posX2, double posY2)
{
	// assume the value is same to index 

	double dExtlen = 0;
	double posX[2] = { posX1, posX2 };
	double posY[2] = { posY1, posY2 };
	double posExtX[2] = { 0 };
	double posExtY[2] = { 0 };

	//getLineLimitPos(posX, posY, dExtlen, posExtX, posExtY);
	posExtX[0] = posX1; posExtX[1] = posX2;
	posExtY[0] = posY1; posExtY[1] = posY2;

	int nSizeX = 0, nSizeY = 0;
	m_csgMesh->getSize(nSizeX, nSizeY);

	int nSampLing = m_csgMesh->getSampling();
	int nLimitX = nSizeX * nSampLing;
	int nLimitY = nSizeY * nSampLing;

	double x1 = posExtX[0];
	double x2 = posExtX[1];
	double y1 = posExtY[0];
	double y2 = posExtY[1];

	//qDebug() << "Bilinear Pt1:" << x1 << " " << y1 << endl;
	//qDebug() << "Bilinear Pt2:" << x2 << " " << y2 << endl;

	if (x1 < m_dMeshSizeMinX) x1 = m_dMeshSizeMinX; if (x1 > m_dMeshSizeMaxX) x1 = m_dMeshSizeMaxX;
	if (y1 < m_dMeshSizeMinY) y1 = m_dMeshSizeMinY; if (y1 > m_dMeshSizeMaxY) y1 = m_dMeshSizeMaxY;
	if (x2 < m_dMeshSizeMinX) x2 = m_dMeshSizeMinX; if (x2 > m_dMeshSizeMaxX) x2 = m_dMeshSizeMaxX;
	if (y2 < m_dMeshSizeMinY) y2 = m_dMeshSizeMinY; if (y2 > m_dMeshSizeMaxY) y2 = m_dMeshSizeMaxY;

	double dDist = qSqrt(qPow(x1 - x2, 2) + qPow(y1 - y2, 2));
	dDist = qCeil(dDist);

	int nNum = qCeil(dDist); // x = 1,2,...m / y = 1,2,...n	

	double* xValues = new double[nNum];
	double* yValues = new double[nNum];
	double* fxs = new double[nNum];
	double* fys = new double[nNum];

	for (int i = 0; i < nNum; i++)
	{
		xValues[i] = x1 + (x2 - x1)* i / dDist;
		yValues[i] = y1 + (y2 - y1)* i / dDist;

		xValues[i] += (m_dMeshSizeMaxX - m_dMeshSizeMinX) / 2 - 0;//80
		yValues[i] += (m_dMeshSizeMaxY - m_dMeshSizeMinY) / 2 + 0;//80

		xValues[i] = xValues[i] / nSampLing + 1;
		yValues[i] = yValues[i] / nSampLing + 1;
		 
		fxs[i] = xValues[i] - qFloor(xValues[i]);
		fys[i] = yValues[i] - qFloor(yValues[i]);
		xValues[i] = qFloor(xValues[i]);
		yValues[i] = qFloor(yValues[i]);
	}

	std::vector<aaAaa::aaSpline> splines;
	aaAaa::aaSpline spline;
	m_pGraphicsEditor->clearSplines();

	int axisUnit = manager->getAxisUnit();	

	double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();

	QVector<cv::Point2d> proData;
	for (int i = 0; i < nNum - 1; i++)
	{	
		int xValue = yValues[i];// X,Y 值互换
		int yValue = xValues[i];

		int index1 = (xValue - 1)*(nSizeX)+yValue - 1;
		int index2 = (xValue - 1)*(nSizeX)+yValue - 1 + 1;
		int index3 = (xValue - 1 + 1)*(nSizeX)+yValue - 1;
		int index4 = (xValue - 1 + 1)*(nSizeX)+yValue - 1 + 1;

		double z1 = m_csgWorld->GetSolid(0)->getVertice(index1).z;
		double z2 = m_csgWorld->GetSolid(0)->getVertice(index2).z;
		double z3 = m_csgWorld->GetSolid(0)->getVertice(index3).z;
		double z4 = m_csgWorld->GetSolid(0)->getVertice(index4).z;

		double zValue = z1*(1 - fxs[i])*(1 - fys[i]) + z2*(1 - fxs[i])*fys[i]
			+ z3*fxs[i]*(1 - fys[i]) + z4*fxs[i]*fys[i];
		
		zValue = zValue * dResolutionX / 1000;
		double dValueX = i * dResolutionX / 1000;

		spline.addKnots(aaAaa::aaPoint(i*1.0 / axisUnit, zValue /*/ axisUnit*/));
		proData.push_back(cv::Point2d(dValueX/*i*1.0 / axisUnit*/, zValue));
	}

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (pVision)
	{
		pVision->setProfData(proData);
	}

	delete[] xValues;
	delete[] yValues;
	delete[] fxs;
	delete[] fys;

	spline.name = "Cross-Section";
	splines.push_back(spline);

	m_pGraphicsEditor->setScale(100.0 / ((m_dMeshSizeMaxX - m_dMeshSizeMinX) / axisUnit / 2), 100.0 / ((m_dMeshSizeMaxZ - m_dMeshSizeMinZ) * dResolutionX / 1000 / 2));
	m_pGraphicsEditor->setViewPos((m_dMeshSizeMaxX - m_dMeshSizeMinX) / axisUnit / 2, (m_dMeshSizeMaxZ + m_dMeshSizeMinZ) * dResolutionX / 1000 / 2);
	m_pGraphicsEditor->setSplines(splines);

	int nWidth = 0, nHeight = 0;
	m_pGraphicsEditor->getSize(nWidth, nHeight);

	m_pGraphicsEditor->setPos(m_nLeft - nWidth, m_nTop);
	m_pGraphicsEditor->show();
}

void ViewWidget::cutingUsing3DUnproject(double posX1, double posY1, double posX2, double posY2)
{
	double X1 = posX1;
	double Y1 = posY1;
	double X2 = posX2;
	double Y2 = posY2;

	if (qAbs(X1 - X2) < 5 && qAbs(Y1 - Y2) < 5)
	{
		return;
	}

	double A = Y2 - Y1;
	double B = X1 - X2;
	double C = X2*Y1 - X1*Y2;
	double angle = qAtan2((double)(posY2 - posY1), (double)(posX2 - posX1));
}



void ViewWidget::setViewAction(ViewAction action)
{
	switch (action)
	{
	case ACTION_MOVE:
		{
			QString path = QApplication::applicationDirPath();
			path += "/image/";
			QCursor cursor;
			QPixmap pixmap(path + "moveHand.png");
			cursor = QCursor(pixmap, -1, -1);
			setCursor(cursor);
		}
		break;		
	case ACTION_ROTATE:
		{
			QString path = QApplication::applicationDirPath();
			path += "/image/";
			QCursor cursor;
			QPixmap pixmap(path + "rotateHand.png");
			cursor = QCursor(pixmap, -1, -1);
			setCursor(cursor);
		}
		break;
	case ACTION_SELECT:
		setCursor(Qt::ArrowCursor);
		break;
	case ACTION_CSG_MESH:
		setCursor(Qt::CrossCursor);
		break;
	case ACTION_NULL:
		setCursor(Qt::ArrowCursor);
		break;
	default:
		setCursor(Qt::ArrowCursor);
		break;
	}	
	m_viewAction = action;
}

void ViewWidget::hideView()
{
	m_pGraphicsEditor->hide();
}

bool ViewWidget::isCSGBoolAlg()
{
	int nCSGIndex = System->getParam("3D_view_param_csg_alg").toInt();
	return 0 == nCSGIndex;
}

bool ViewWidget::isBilinearInterpolationAlg()
{
	int nCSGIndex = System->getParam("3D_view_param_csg_alg").toInt();
	return 1 == nCSGIndex;
}

bool ViewWidget::is3DUnprojectAlg()
{
	int nCSGIndex = System->getParam("3D_view_param_csg_alg").toInt();
	return 2 == nCSGIndex;
}

void ViewWidget::loadFile(QString& fileName)
{
	QString fileNameY, fileNameZ;
	int nIndex = fileName.lastIndexOf('.');
	QString fileExt = fileName.right(fileName.length() - nIndex - 1);
	if (nIndex > 0 && nIndex < fileName.length())
	{
		fileNameY = fileName.left(nIndex - 1) + "y" + "." + fileExt;
		fileNameZ = fileName.left(nIndex - 1) + "z" + "." + fileExt;
	}

	m_csgMesh->loadData(fileName, fileNameY, fileNameZ, fileExt);
	
	initData();
}

void ViewWidget::loadFile(bool invert, int nSizeX, int nSizeY, QVector<Vector>& values)
{
	manager->setXYCoordinate(invert);
	m_csgMesh->loadData(invert, nSizeX, nSizeY, values);	

	initData();
}

void ViewWidget::initData()
{
	CGameFont::SetDeviceContext(wglGetCurrentDC());
	resizeGL(this->geometry().width(), this->geometry().height());

	m_pGraphicsEditor->hide();
	m_bShowMeshTriangulation = false;
	m_bShowMeshSurfaceCute = false;

	if (m_idCutMeshLine > 0)
	{
		glDeleteLists(m_idCutMeshLine, 1);
		m_idCutMeshLine = 0;
	}

	if (m_idCutMesh > 0)
	{
		glDeleteLists(m_idCutMesh, 1);
		m_idCutMesh = 0;
	}

	m_csgWorld->ClearSolids();
	if (m_pBooleanModeller)
	{
		delete m_pBooleanModeller;
		m_pBooleanModeller = NULL;
	}
	if (m_pDifferenceModel)
	{
		delete m_pDifferenceModel;
		m_pDifferenceModel = NULL;
	}

	m_csgMesh->getLimit(m_dMeshSizeMaxX, m_dMeshSizeMinX, m_dMeshSizeMaxY, m_dMeshSizeMinY, m_dMeshSizeMaxZ, m_dMeshSizeMinZ);
	
	VectorSet* vertices = m_csgMesh->getVerticesMeshData();  IntSet* indices = m_csgMesh->getIndicesMeshData();
	if (vertices != NULL && indices != NULL)
	{
		int nSizeX = 0, nSizeY = 0;
		m_csgMesh->getSize(nSizeX, nSizeY);
		m_csgWorld->LoadObj(vertices, indices, nSizeX, nSizeY, m_dMeshSizeMaxZ, m_dMeshSizeMinZ);
		m_csgWorld->LoadCube();

		if (isCSGBoolAlg())
		{
			if (!m_pBooleanModeller)
			{
				m_pBooleanModeller = new BooleanModeller(m_csgWorld->GetSolid(0), m_csgWorld->GetSolid(1));
			}

			if (!m_pDifferenceModel)
			{
				m_pDifferenceModel = m_pBooleanModeller->getDifference(false);
			}
		}
	}

	int xyLen = 0, xyOffset = 0;
	if (qAbs(m_dMeshSizeMaxX - m_dMeshSizeMinX) > qAbs(m_dMeshSizeMaxY - m_dMeshSizeMinY))
	{
		xyLen = (m_dMeshSizeMaxX - m_dMeshSizeMinX) * VIEW_POINT_XY_SCALE;
		xyOffset = (m_dMeshSizeMaxX + m_dMeshSizeMinX) / 2;
	}
	else
	{
		xyLen = (m_dMeshSizeMaxY - m_dMeshSizeMinY) * VIEW_POINT_XY_SCALE;
		xyOffset = (m_dMeshSizeMaxY + m_dMeshSizeMinY) / 2;
	}

	int zLen = 0, zOffset = 0;
	zLen = (m_dMeshSizeMaxZ - m_dMeshSizeMinZ)* VIEW_POINT_Z_SCALE;
	zOffset = (m_dMeshSizeMaxZ + m_dMeshSizeMinZ) / 2;

	manager->setOrthoSize(xyLen * 0.8 /*35*/);
	manager->setXYSize(xyLen, xyOffset);
	manager->setZSize(zLen, zOffset);

	manager->init();

	m_bShowCloudPoint = true;	
}

void ViewWidget::zoomIn()
{
	manager->executeScaleOperation(-0.2);
	updateGL();
}

void ViewWidget::zoomOut()
{
	manager->executeScaleOperation(0.2);
	updateGL();
}

void ViewWidget::moveView()
{
	setViewAction(ACTION_MOVE);
	updateGL();
}

void ViewWidget::rotateView()
{
	setViewAction(ACTION_ROTATE);
	updateGL();
}

void ViewWidget::fullScreen()
{
	if (manager)
	{
		manager->fullView(ACTION_PERSPECT_FULL);
	}
	setViewAction(ACTION_SELECT);
	updateGL();
}

void ViewWidget::frontView()
{
	if (manager)
	{
		manager->fullView(ACTION_PERSPECT_FRONT);
	}
	setViewAction(ACTION_SELECT);
	updateGL();
}

void ViewWidget::topView()
{
	if (manager)
	{
		manager->fullView(ACTION_PERSPECT_TOP);
	}
	setViewAction(ACTION_SELECT);
	updateGL();
}

void ViewWidget::sideView()
{
	if (manager)
	{
		manager->fullView(ACTION_PERSPECT_SIDE);
	}
	setViewAction(ACTION_SELECT);
	updateGL();
}

void ViewWidget::changePointCloudView()
{
	m_bShowCloudPoint = !m_bShowCloudPoint;
	updateGL();
}

void ViewWidget::changeMeshTriView(bool bPrmpt)
{
	if (!m_csgMesh->isConvertToMesh())
	{
		if (!bPrmpt || QMessageBox::Ok == QMessageBox::warning(NULL, QStringLiteral("提示"),
			QStringLiteral("生成3D曲面？"), QMessageBox::Ok, QMessageBox::Cancel))
		{
			m_bShowCloudPoint = false;

			// Convert to Mesh Triangles
			//m_csgMesh->reSamplingUsingMLS();
			//m_csgMesh->convetToMeshTriangulation();
			QDateTime timeRead = QDateTime::currentDateTime();
			m_csgMesh->convetToMeshTriangulationExt();
			qDebug() << QString("convet to mesh data time : %1").arg(timeRead.msecsTo(QDateTime::currentDateTime()));

			m_csgWorld->ClearSolids();
			if (m_pBooleanModeller)
			{
				delete m_pBooleanModeller;
				m_pBooleanModeller = NULL;
			}
			if (m_pDifferenceModel)
			{
				delete m_pDifferenceModel;
				m_pDifferenceModel = NULL;
			}

			VectorSet* vertices = m_csgMesh->getVerticesMeshData();  IntSet* indices = m_csgMesh->getIndicesMeshData();
			if (vertices != NULL && indices != NULL)
			{
				int nSizeX = 0, nSizeY = 0;
				m_csgMesh->getSize(nSizeX, nSizeY);
				qDebug() << QString("get mesh data time : %1").arg(timeRead.msecsTo(QDateTime::currentDateTime()));
				m_csgWorld->LoadObj(vertices, indices, nSizeX, nSizeY, m_dMeshSizeMaxZ, m_dMeshSizeMinZ);
				qDebug() << QString("load mesh obj time : %1").arg(timeRead.msecsTo(QDateTime::currentDateTime()));
				m_csgWorld->LoadCube();

				//if (!m_pBooleanModeller)
				//	m_pBooleanModeller = new BooleanModeller(m_csgWorld->GetSolid(0), m_csgWorld->GetSolid(1));

				//if (!m_pDifferenceModel)
				//{
				//	m_pDifferenceModel = m_pBooleanModeller->getDifference(false);
				//}
			}
		}
		else
		{
			return;
		}		
	}

	m_bShowMeshTriangulation = !m_bShowMeshTriangulation;	
	updateGL();
}

void ViewWidget::meshTexture()
{
	if (!m_csgWorld->isBindTexture())
	{
		if (QMessageBox::Ok == QMessageBox::warning(NULL, QStringLiteral("提示"),
			QStringLiteral("导入表面纹理？"), QMessageBox::Ok, QMessageBox::Cancel))
		{
			QString path = QApplication::applicationDirPath();
			path += "/3D/texture/";

			QString fileName = QFileDialog::getOpenFileName(this,
				QString("Select Data File to load"),
				path,
				"*.png *.jpg *.jpeg *.bmp");

			if (!fileName.isEmpty())
			{
				QImage tex, buf;

				if (!buf.load(fileName))
				{
					qDebug() << "Cannot open the texture image...";
					QImage dummy(128, 128, QImage::Format_RGB32);
					dummy.fill(Qt::green);
					buf = dummy;
				}

				tex = convertToGLFormat(buf);

				m_csgWorld->bindTexture(tex, false);
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		if (QMessageBox::Ok == QMessageBox::warning(NULL, QStringLiteral("提示"),
			QStringLiteral("清除表面纹理？"), QMessageBox::Ok, QMessageBox::Cancel))
		{
			m_csgWorld->clearTexture();
		}
	}
}

void ViewWidget::meshTexture(QImage& textrue, bool invert)
{
	if (textrue.isNull()) return;

	if (m_csgWorld->isBindTexture())
	{
		m_csgWorld->clearTexture();
	}
	
	m_csgWorld->bindTexture(convertToGLFormat(textrue), invert);
}

void ViewWidget::csgCutMesh()
{
	if (!m_csgMesh->isConvertToMesh())
	{
		if (QMessageBox::Ok == QMessageBox::warning(NULL, QStringLiteral("提示"),
			QStringLiteral("没有曲面数据，请先设置曲面!"), QMessageBox::Ok))
		{			
		}

		return;
	}

	if (manager->getViewPsp() != ACTION_PERSPECT_TOP)	topView();

	manager->executeScaleOperation(0.2);

	//fullScreen();

	m_bShowCloudPoint = false;
	m_bShowMeshTriangulation = true;
	m_bShowMeshSurfaceCute = false;	

	if (m_idCutMeshLine > 0)
	{
		glDeleteLists(m_idCutMeshLine, 1);
		m_idCutMeshLine = 0;
	}

	if (m_idCutMesh > 0)
	{
		glDeleteLists(m_idCutMesh, 1);
		m_idCutMesh = 0;
	}

	setViewAction(ACTION_CSG_MESH);
	updateGL();
}

void ViewWidget::setBasePos(int nLeft, int nTop)
{
	m_nLeft = nLeft;
	m_nTop = nTop;
}

bool ViewWidget::ScreenPtToViewPt(double i_ScreenPtX, double i_ScreenPtY, double* o_ViewPointX, double* o_ViewPointY, double* o_ViewPointZ)
{
	int x = i_ScreenPtX;    /* 屏幕坐标 */
	int y = i_ScreenPtY;

	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLfloat winx, winy, winz;
	GLdouble posx, posy, posz;

	glPushMatrix();

	//glScalef(0.1, 0.1, 0.1);  
	glGetIntegerv(GL_VIEWPORT, viewport);   /* 获取三个矩阵 */
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

	glPopMatrix();

	winx = x;
	winy = m_height - y;	

	glReadPixels((int)winx, (int)winy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winz);   /* 获取深度 */
	gluUnProject(winx, winy, winz, mvmatrix, projmatrix, viewport, &posx, &posy, &posz); /* 获取三维坐标 */

	*o_ViewPointX = posx;
	*o_ViewPointY = posy;
	*o_ViewPointZ = posz;

	int nMaxZ = 0, nMinZ = 0;
	manager->getAxisZLimit(nMaxZ, nMinZ);

	int nMaxXY = 0, nMinXY = 0;
	manager->getAxisXYLimit(nMaxXY, nMinXY);

	if (posx >= nMaxXY || posx <= nMinXY
		|| posy >= nMaxXY || posy <= nMinXY
		|| posz >= nMaxZ || posz <= nMinZ)
	{	
		return false;		
	}

	return true;
}

bool ViewWidget::ScreenPtToViewPtExt(double i_ScreenPtX, double i_ScreenPtY, double o_ViewPointZ, double* o_ViewPointX, double* o_ViewPointY)
{
	int x = i_ScreenPtX;    /* 屏幕坐标 */
	int y = i_ScreenPtY;

	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLfloat winx, winy, winz;
	GLdouble posx, posy, posz;

	glPushMatrix();

	//glScalef(0.1, 0.1, 0.1);  
	glGetIntegerv(GL_VIEWPORT, viewport);   /* 获取三个矩阵 */
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

	glPopMatrix();

	winx = x;
	winy = m_height - y;
	winz = o_ViewPointZ;

	//glReadPixels((int)winx, (int)winy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winz);   /* 获取深度 */
	gluUnProject(winx, winy, winz, mvmatrix, projmatrix, viewport, &posx, &posy, &posz); /* 获取三维坐标 */

	*o_ViewPointX = posx;
	*o_ViewPointY = posy;
	//*o_ViewPointZ = posz;

	//int nMaxZ = 0, nMinZ = 0;
	//manager->getAxisZLimit(nMaxZ, nMinZ);

	int nMaxXY = 0, nMinXY = 0;
	manager->getAxisXYLimit(nMaxXY, nMinXY);

	if (posx >= nMaxXY || posx <= nMinXY
		|| posy >= nMaxXY || posy <= nMinXY
		/*|| posz >= nMaxZ || posz <= nMinZ*/)
	{
		return false;
	}

	return true;
}

void ViewWidget::getLineLimitPos(double* posX, double* posY, double dExtLen, double* posXExt, double* posYExt)
{
	double X1 = posX[0];
	double Y1 = posY[0];
	double X2 = posX[1];
	double Y2 = posY[1];	

	if (qAbs(X1 - X2) < 1 && qAbs(Y1 - Y2) < 1)
	{
		return;
	}

	double A = Y2 - Y1;
	double B = X1 - X2;
	double C = X2*Y1 - X1*Y2;

	QVector<double> xCrossValues;
	QVector<double> yCrossValues;
	if (qAbs(B) < 0.0000001)
	{
		xCrossValues.push_back(-C / A);
		yCrossValues.push_back(m_dMeshSizeMinY - dExtLen);

		xCrossValues.push_back(-C / A);
		yCrossValues.push_back(m_dMeshSizeMaxY + dExtLen);
	}
	else if (qAbs(A) < 0.0000001)
	{
		xCrossValues.push_back(m_dMeshSizeMinX - dExtLen);
		yCrossValues.push_back(-C / B);

		xCrossValues.push_back(m_dMeshSizeMaxX + dExtLen);
		yCrossValues.push_back(-C / B);
	}
	else
	{
		double xVal1 = -(B*m_dMeshSizeMaxY + C) / A;
		double xVal2 = -(B*m_dMeshSizeMinY + C) / A;
		double yVal1 = -(A*m_dMeshSizeMaxX + C) / B;
		double yVal2 = -(A*m_dMeshSizeMinX + C) / B;

		if (xVal1 <= m_dMeshSizeMaxX && xVal1 >= m_dMeshSizeMinX)
		{
			xVal1 = -(B*(m_dMeshSizeMaxY + dExtLen) + C) / A;
			xCrossValues.push_back(xVal1);
			yCrossValues.push_back(m_dMeshSizeMaxY + dExtLen);
		}
		if (xVal2 <= m_dMeshSizeMaxX && xVal2 >= m_dMeshSizeMinX)
		{
			xVal2 = -(B*(m_dMeshSizeMinY - dExtLen) + C) / A;
			xCrossValues.push_back(xVal2);
			yCrossValues.push_back(m_dMeshSizeMinY - dExtLen);
		}
		if (yVal1 <= m_dMeshSizeMaxY && yVal1 >= m_dMeshSizeMinY)
		{
			yVal1 = -(A*(m_dMeshSizeMaxX + dExtLen) + C) / B;
			xCrossValues.push_back(m_dMeshSizeMaxX + dExtLen);
			yCrossValues.push_back(yVal1);
		}
		if (yVal2 <= m_dMeshSizeMaxY && yVal2 >= m_dMeshSizeMinY)
		{
			yVal2 = -(A*(m_dMeshSizeMinX - dExtLen) + C) / B;
			xCrossValues.push_back(m_dMeshSizeMinX - dExtLen);
			yCrossValues.push_back(yVal2);
		}
	}

	if (xCrossValues.size() == 2 && yCrossValues.size() == 2)
	{
		posXExt[0] = xCrossValues[0];
		posXExt[1] = xCrossValues[1];

		posYExt[0] = yCrossValues[0];
		posYExt[1] = yCrossValues[1];
	}
}

bool ViewWidget::isPointNearPoint(Vector pt1, Vector pt2, double dDist)
{
	double ptDist = qSqrt(qPow(pt1.x - pt2.x, 2) + qPow(pt1.y - pt2.y, 2) + qPow(pt1.z - pt2.z, 2));
	

	if (ptDist <= dDist)
	{
		return true;
	}

	return false;
}

//bool ViewWidget::isPointInMesh(double posX, double posY, double posZ)
//{
//	bool bInMesh = false;
//
//	for (int i = 0; i < m_csgWorld->GetSolid(0)->vertices.GetSize(); i++)
//	{
//		int x = m_csgWorld->GetSolid(0)->vertices[i].x;
//		int y = m_csgWorld->GetSolid(0)->vertices[i].y;
//		int z = m_csgWorld->GetSolid(0)->vertices[i].z;
//
//		if (x == posX && y == posY && z == posZ)
//		{
//			bInMesh = true;
//			break;
//		}
//	}
//
//	return bInMesh;
//}