#include "RoamingScenceManager.h"

#include"MathAPIKernel/Vector3D.h"
#include"MathAPIKernel/Position3D.h"
#include"MathAPIKernel/Matrix4D.h"

#define  MAX_CHAR 128

#include <windows.h>

#include"GL/glut.h"

//#include "GLFont.h"

#include <qdebug.h>

//#define ROTATE_AS_Z

RoamingScenceManager::RoamingScenceManager(QGLWidget* pWidget)
	: m_viewWidget(pWidget)
{
    OldMouse=new Position3D();
    Mouse=new Position3D();

    NewEye=new Vector3D(1,0,0);
    NewUp=new Vector3D(0,0,1);
    NewView=new Vector3D(0,0,0);

    AuxY =new Vector3D(0,1,0);
    AuxZ=new Vector3D();
	*AuxZ = (Vector3D)*NewEye - (Vector3D)*NewView;
    AuxX=new Vector3D();
    *AuxX=Vector3D::crossProduct(*AuxY,*AuxZ);
    AuxX->normalize();

    TempTranslateVec=new Vector3D(0,0,0);
    TempscaleFactor=1;

	m_OrthoSize = 35;
	AXES_LEN = 20;
	AXES_LEN_OFFSET = 0;
	AXES_LEN_Z = 8;
	AXES_LEN_Z_OFFSET = 0;
	AXES_GRADUATION = 10;
	AXES_UNIT = 1;
	//m_newFont = new CGameFont();	

	m_bXYCoordinateInvert = false;
}

RoamingScenceManager::~RoamingScenceManager()
{
    delete OldMouse;
    delete Mouse;
    delete NewEye;
    delete NewUp;
    delete NewView;
    delete AuxX;
    delete AuxY;
    delete AuxZ;
    delete TempTranslateVec;
	//delete m_newFont;
}

void RoamingScenceManager::getAxisZLimit(int& nMaxZ, int& nMinZ)
{
	nMaxZ = AXES_LEN_Z / 2 + AXES_LEN_Z_OFFSET;
	nMinZ = -AXES_LEN_Z / 2 + AXES_LEN_Z_OFFSET;
}

void RoamingScenceManager::getAxisXYLimit(int& nMaxValue, int& nMinValue)
{
	nMaxValue = AXES_LEN / 2 + AXES_LEN_OFFSET;
	nMinValue = -AXES_LEN / 2 + AXES_LEN_OFFSET;
}

int RoamingScenceManager::getAxisUnit()
{
	return AXES_UNIT;
}

double RoamingScenceManager::getScaleFactor()
{
	return TempscaleFactor;
}

void RoamingScenceManager::setXYCoordinate(bool bInvert)
{
	m_bXYCoordinateInvert = bInvert;
}

void RoamingScenceManager::setOrthoSize(double orthoSize)
{
	m_OrthoSize = orthoSize;
}

void RoamingScenceManager::setXYSize(int xyAxisLen, int xyOffset)
{
	AXES_LEN = xyAxisLen;
	AXES_LEN_OFFSET = xyOffset;
}

void RoamingScenceManager::setZSize(int zAxisLen, int zOffset)
{
	AXES_LEN_Z = zAxisLen;
	AXES_LEN_Z_OFFSET = zOffset;
}

void RoamingScenceManager::init()
{
   init_CoordinaryDisplayList();
   //GLDrawSpaceAxes();
  
   glShadeModel(GL_SMOOTH);
   glClearColor(0.156   ,   0.156  ,    0.168 ,     0.0);
   glClearDepth(1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-m_OrthoSize / 2, m_OrthoSize / 2, -m_OrthoSize / 2, m_OrthoSize / 2, -m_OrthoSize *1.5, m_OrthoSize *1.5);    

   fullView(ACTION_PERSPECT_TOP);
}

void RoamingScenceManager::render()
{
    glClearColor(0.7   ,   0.7  ,    0.7 ,     0.0);
	//glClearColor(0.239, 0.353, 0.518, 0.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    *NewView=*NewEye*-1;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(TempTranslateVec->X(),TempTranslateVec->Y(),TempTranslateVec->Z());
    glScalef(TempscaleFactor,TempscaleFactor,TempscaleFactor);
    gluLookAt(NewEye->X(),NewEye->Y(),NewEye->Z(),
              NewView->X(),NewView->Y(),NewView->Z(),
              NewUp->X(),NewUp->Y(),NewUp->Z());
    glColor3f(1.0,0.0,0.0);
    //glutWireCube(250);
    glCallList(ID_COORDINATY);	

	// Render Coordinate Text
	render_CoordinaryDisplayText();
}

void RoamingScenceManager::executeRotateOperation(int x, int y)
{
    Mouse->setX(x);
    Mouse->setY(y);

    Vector3D MouseTrace=*AuxY*(OldMouse->Y()-Mouse->Y())*0.2 + *AuxX*(Mouse->X()-OldMouse->X())*0.2;
    Vector3D RotateAsix=Vector3D::crossProduct(MouseTrace,*AuxZ);
    RotateAsix.normalize();

    float angle = MouseTrace.length();

    #ifdef ROTATE_AS_Z    
    RotateAsix = *AuxZ;
    RotateAsix.normalize();

    MouseTrace = *AuxX*(Mouse->X() - OldMouse->X())*0.2;
    angle = (Mouse->X() - OldMouse->X()) > 0 ? -MouseTrace.length() : MouseTrace.length();   
    #endif
    
    Matrix4D rotatMatrix=Matrix4D::getRotateMatrix(angle,RotateAsix);

    *NewEye=rotatMatrix*(*NewEye);
    *NewUp=rotatMatrix*(*NewUp);

    NewUp->normalize();
    *AuxY=*NewUp;
	*AuxZ = (Vector3D)*NewEye - (Vector3D)*NewView;
    *AuxX=Vector3D::crossProduct(*AuxY,*AuxZ);
    AuxX->normalize();
    //更新鼠标
    *OldMouse=*Mouse;

	m_viewPerspective = ACTION_PERSPECT_OPTIONAL;
}

void RoamingScenceManager::executeScaleOperation(float factor)
{
      if(TempscaleFactor<=0)
      {
          TempscaleFactor=0.1;
      }else
      {
          TempscaleFactor+=factor;
      }

	 // *TempTranslateVec = (*TempTranslateVec) * (TempscaleFactor);

	  m_viewPerspective = ACTION_PERSPECT_OPTIONAL;
}

void RoamingScenceManager::executeTranslateOperation(int x, int y)
{
    Mouse->setX(x);
    Mouse->setY(y);
    Vector3D vec(-OldMouse->X()+Mouse->X(),-Mouse->Y()+OldMouse->Y(),0);
	*TempTranslateVec = *TempTranslateVec + vec*0.02*AXES_LEN/20;
    *OldMouse=*Mouse;

	m_viewPerspective = ACTION_PERSPECT_OPTIONAL;
}

void RoamingScenceManager::getInitPos(int x, int y)
{
    Mouse->setX(x);
    Mouse->setY(y);
    *OldMouse=*Mouse;
}

void RoamingScenceManager::fullView(ViewPerspective viewPsp)
{
	NewEye->setX(0);
	NewEye->setY(0);
	NewEye->setZ(0);

	NewUp->setX(0);
	NewUp->setY(0);
	NewUp->setZ(1);

	NewView->setX(0);
	NewView->setY(0);
	NewView->setZ(0);

	AuxY->setX(0);
	AuxY->setY(1);
	AuxY->setZ(0);

	switch (viewPsp)
	{
	case ACTION_PERSPECT_FRONT:
		NewEye->setX(1);
		break;
	case ACTION_PERSPECT_SIDE:
		NewEye->setY(1);
		break;
	case ACTION_PERSPECT_TOP:
		NewEye->setZ(1);
		NewView->setZ(-1);
		NewUp->setX(-1);
		break;
	case ACTION_PERSPECT_FULL:
		{
			fullView(ACTION_PERSPECT_FRONT);

			NewEye->setX(1);

			Vector3D MouseTrace = *AuxY*(-15) + *AuxX*(50);
			Vector3D RotateAsix = Vector3D::crossProduct(MouseTrace, *AuxZ);
			RotateAsix.normalize();

			float angle = MouseTrace.length();
			Matrix4D rotatMatrix = Matrix4D::getRotateMatrix(angle, RotateAsix);

			*NewEye = rotatMatrix*(*NewEye);
			*NewUp = rotatMatrix*(*NewUp);

			NewUp->normalize();
			*AuxY = *NewUp;
		}
		break;
	default:
		break;
	}
	
	*AuxZ = (Vector3D)*NewEye - (Vector3D)*NewView;	
	*AuxX = Vector3D::crossProduct(*AuxY, *AuxZ);
	AuxX->normalize();

	switch (viewPsp)
	{
	case ACTION_PERSPECT_FRONT:
		TempTranslateVec->setX(-AXES_LEN_OFFSET);
		TempTranslateVec->setY(-AXES_LEN_OFFSET);
		TempTranslateVec->setZ(0);
		break;
	case ACTION_PERSPECT_SIDE:
		TempTranslateVec->setX(AXES_LEN_OFFSET);
		TempTranslateVec->setY(-AXES_LEN_OFFSET);
		TempTranslateVec->setZ(0);
		break;
	case ACTION_PERSPECT_TOP:
		TempTranslateVec->setX(-AXES_LEN_OFFSET);
		TempTranslateVec->setY(AXES_LEN_OFFSET);
		TempTranslateVec->setZ(0);
		break;
	case ACTION_PERSPECT_FULL:
		TempTranslateVec->setX(-AXES_LEN_OFFSET);
		TempTranslateVec->setY(0);
		TempTranslateVec->setZ(0);		
		break;
	default:
		TempTranslateVec->setX(0);
		TempTranslateVec->setY(0);
		TempTranslateVec->setZ(0);
		break;
	}

	TempscaleFactor = 1.0;
	m_viewPerspective = viewPsp;
}

void RoamingScenceManager::init_CoordinaryDisplayList()
{
	/*****网格绘制*****/
	/*****使用颜色混合来消除一些锯齿， 主要针对点和线
	以及不相互重叠的多边形的反锯齿。*****/

	if (ID_COORDINATY > 0)
	{
		glDeleteLists(ID_COORDINATY, 1);
	}

	ID_COORDINATY = glGenLists(1);
	glNewList(ID_COORDINATY, GL_COMPILE);

	/*红色轴是X轴，绿色是Y轴，蓝色是Z轴*/
	int length=1;
	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(1.0,0.0,0.0);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(length,0.0,0.0);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0,1.0,0.0);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(0.0,length,0.0);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0,0.0,1.0);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(0.0,0.0,length);
	glEnd();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);                   //设置反走样
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);       //设置反走样
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	//绘制立体坐标系
	GLUquadricObj *objCylinder = gluNewQuadric();
	glRotatef(0, 0.0, 1.0, 0.0);

	double dAxisLen = AXES_LEN / 2 - AXES_LEN_OFFSET;
	double dAxisZLen = AXES_LEN_Z / 2 - AXES_LEN_Z_OFFSET;

	//确定坐标系原点
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(AXES_LEN_OFFSET, AXES_LEN_OFFSET, AXES_LEN_Z_OFFSET);
	glPopMatrix();

	//画网格线 Y
	glPushMatrix();
	{
		Position3D cpoint1 = Position3D(-dAxisLen, -dAxisLen, -dAxisZLen);
		Position3D cpoint2 = Position3D(-dAxisLen + AXES_LEN, -dAxisLen, -dAxisZLen + AXES_LEN_Z);
		glColor3f(0.5f, 0.5f, 0.5f);
		GLGrid(cpoint1, cpoint2, 10);
	}
	glPopMatrix();

	//画网格线 Z
	glPushMatrix();
	{
		glRotatef(-90, 1.0, 0.0, 0.0);
		glTranslatef(0, dAxisZLen, 0);
		Position3D cpoint3 = Position3D(-dAxisLen, 0, -dAxisLen);
		Position3D cpoint4 = Position3D(-dAxisLen + AXES_LEN, 0, -dAxisLen + AXES_LEN);
		glColor3f(0.5f, 0.5f, 0.5f);
		GLGrid(cpoint3, cpoint4, 10);		
	}
	glPopMatrix();

	//画网格线 X
	glPushMatrix();
	{
		glRotatef(90, 0.0, 0.0, 1.0);
		glTranslatef(0, dAxisLen, -0);
		Position3D cpoint5 = Position3D(-dAxisLen, 0, -dAxisZLen);
		Position3D cpoint6 = Position3D(-dAxisLen + AXES_LEN, 0, -dAxisZLen + AXES_LEN_Z);
		glColor3f(0.5f, 0.5f, 0.5f);
		GLGrid(cpoint5, cpoint6, 10);
	}
	glPopMatrix();

	//画坐标轴 Z	
	glPushMatrix();
	glColor3f(0.4f, 0.4f, 0.4f);
	glTranslatef(-dAxisLen, -dAxisLen, -dAxisZLen);
	gluCylinder(objCylinder, 0.05, 0.05, AXES_LEN_Z, 10, 5);           
	glTranslatef(0, 0, AXES_LEN_Z);
	gluCylinder(objCylinder, 0.2, 0.0, 0.5, 10, 5);                 
	glPopMatrix();

	//glPushMatrix();
	//glTranslatef(-dAxisLen, -dAxisLen, -dAxisZLen);
	//glTranslatef(0, 0.2, AXES_LEN_Z);
	//glRotatef(90, 0.0, 1.0, 0.0);
	//drawString(QString("Z"), -dAxisLen, -dAxisLen, -dAxisZLen + AXES_LEN_Z, axisFont);  // Print GL Text ToThe Screen
	//glPopMatrix();

	//画坐标轴 X
	glPushMatrix();
	glColor3f(0.4f, 0.4f, 0.4f);
	glTranslatef(-dAxisLen, -dAxisLen, -dAxisZLen);
	glRotatef(90, 0.0, 1.0, 0.0);
	gluCylinder(objCylinder, 0.05, 0.05, AXES_LEN, 10, 5);           
	glTranslatef(0, 0, AXES_LEN);
	gluCylinder(objCylinder, 0.2, 0.0, 0.5, 10, 5);                 
	glPopMatrix();

	//glPushMatrix();
	//glTranslatef(-dAxisLen, -dAxisLen, -dAxisZLen);
	//glRotatef(90, 0.0, 1.0, 0.0);
	//glTranslatef(0, 0.2, AXES_LEN);
	//glRotatef(90, 0.0, 1.0, 0.0);
	//drawString(QString("X"), 0, 0, 0, axisFont);                                               // Print GL Text ToThe Screen
	//glPopMatrix();

	//画坐标轴 Y
	glPushMatrix();
	glColor3f(0.4f, 0.4f, 0.4f);
	glTranslatef(-dAxisLen, -dAxisLen, -dAxisZLen);
	glRotatef(-90, 1.0, 0.0, 0.0);
	gluCylinder(objCylinder, 0.05, 0.05, AXES_LEN, 10, 5);           
	glTranslatef(0, 0, AXES_LEN);
	gluCylinder(objCylinder, 0.2, 0.0, 0.5, 10, 5);                 
	glPopMatrix();

	//glPushMatrix();
	//glTranslatef(-dAxisLen, -dAxisLen, -dAxisZLen);
	//glRotatef(-90, 1.0, 0.0, 0.0);
	//glTranslatef(0.0, 0.6, AXES_LEN);
	//glRotatef(90, 0.0, 1.0, 0.0);
	//glRotatef(90, 0.0, 0.0, 1.0);
	//drawString(QString("Y"), 0, 0, 0, axisFont);                                                // Print GL Text ToThe Screen
	//glPopMatrix();

	/*****取消反锯齿*****/
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_POINT_SMOOTH);
	glDisable(GL_POLYGON_SMOOTH);

	// Axis Z graduation
	glLineWidth(1.0f);
	glLineStipple(1, 0xffff);//线条样式
	glEnable(GL_LINE_STIPPLE);

	glBegin(GL_LINES);
	glEnable(GL_LINE_SMOOTH);

	//绘制平行于Y的直线
	double dGraduationLen = AXES_LEN * 0.015;
	int num = AXES_GRADUATION;
	const double _xLen = AXES_LEN / num;
	const double _yLen = AXES_LEN / num;
	const double _zLen = (double)AXES_LEN_Z / num;

	for (int zi = 0; zi <= num; zi++)
	{
		double z = -dAxisZLen + _zLen * zi;
		double x = -dAxisLen;
		double y = -dAxisLen;

		glVertex3f(x, y, z);
		glVertex3f(x, y - dGraduationLen, z);		
	}

	for (int xi = 0; xi <= num; xi++)
	{
		double z = -dAxisZLen;
		double x = -dAxisLen + _xLen * xi;
		double y = -dAxisLen;

		glVertex3f(x, y, z);
		glVertex3f(x, y - dGraduationLen, z);
	}

	for (int yi = 0; yi <= num; yi++)
	{
		double z = -dAxisZLen;
		double x = -dAxisLen;
		double y = -dAxisLen + _yLen * yi;

		glVertex3f(x, y, z);
		glVertex3f(x - dGraduationLen, y, z);
	}

	glDisable(GL_LINE_SMOOTH);
	glEnd();
	glDisable(GL_LINE_STIPPLE);

	glLineWidth(1);
	glEnd();
	glEndList();

	int nLevel = 1;
	int axisRange = AXES_LEN;
	while (axisRange >= 10)
	{
		axisRange /= 10;
		nLevel *= 10;
	}

	AXES_UNIT = nLevel/10;
}

void RoamingScenceManager::render_CoordinaryDisplayText()
{
	double dAxisLen = AXES_LEN / 2 - AXES_LEN_OFFSET;
	double dAxisZLen = AXES_LEN_Z / 2 - AXES_LEN_Z_OFFSET;
	QFont axisFont("Arial", 8);
	double dExtLen = (double)AXES_LEN / 20 * 1;

	// Axis Name
	glColor3f(0.2f, 0.2f, 0.2f);
	glPushMatrix();
	drawString(QString("Z"), -dAxisLen, -dAxisLen, -dAxisZLen + AXES_LEN_Z + dExtLen, axisFont);  // Print GL Text ToThe Screen
	drawString(QString(m_bXYCoordinateInvert ? "Y" : "X"), -dAxisLen + AXES_LEN + dExtLen, -dAxisLen, -dAxisZLen, axisFont); // Print GL Text ToThe Screen
	drawString(QString(m_bXYCoordinateInvert ? "X" : "Y"), -dAxisLen, -dAxisLen + AXES_LEN + dExtLen, -dAxisZLen, axisFont); // Print GL Text ToThe Screen
	glPopMatrix();	

	// Axis Z graduation
	QFont axisGradFont("Arial", 5);

	int num = AXES_GRADUATION;
	const double _xLen = AXES_LEN / num;
	const double _yLen = AXES_LEN / num;
	const double _zLen = (double)AXES_LEN_Z / num;

	for (int zi = 0; zi <= num; zi++)
	{
		double z = -dAxisZLen + _zLen * zi;
		double x = -dAxisLen;
		double y = -dAxisLen;

		//glVertex3f(x, y, z);
		//glVertex3f(x, y - dGraduationLen, z);

        drawString(QString::number((z + AXES_LEN_Z / 2 + AXES_LEN_Z_OFFSET) / AXES_UNIT, 'g', 2), x, y - (double)AXES_LEN / 20 * 1, z, axisGradFont);
	}

	for (int xi = 0; xi <= num; xi++)
	{
		double z = -dAxisZLen;
		double x = -dAxisLen + _xLen * xi;
		double y = -dAxisLen;

		//glVertex3f(x, y, z);
		//glVertex3f(x, y - dGraduationLen, z);

		drawString(QString::number(x / AXES_UNIT, 'g', 2), x, y - (double)AXES_LEN / 20 * 1, z, axisGradFont);
	}

	for (int yi = 0; yi <= num; yi++)
	{
		double z = -dAxisZLen;
		double x = -dAxisLen;
		double y = -dAxisLen + _yLen * yi;

		//glVertex3f(x, y, z);
		//glVertex3f(x - dGraduationLen, y, z);

		drawString(QString::number(y / AXES_UNIT, 'g', 2), x - (double)AXES_LEN / 20 * 0.5, y, z, axisGradFont);
	}

	QFont unitFont("Arial", 8);
	int width = m_viewWidget->width();
	int height = m_viewWidget->height();
	
	drawStringExt(QString("Unit: %1 mm").arg(AXES_UNIT), width - 100, height - 10, unitFont);
}

void RoamingScenceManager::GLGrid(Position3D& pt1, Position3D& pt2, int num)
{
	const float _xLen = (pt2.X() - pt1.X()) / num;
	const float _yLen = (pt2.Y() - pt1.Y()) / num;
	const float _zLen = (pt2.Z() - pt1.Z()) / num;

	glLineWidth(1.0f);
	glLineStipple(1, 0x5555);//线条样式
	glEnable(GL_LINE_STIPPLE);

	glBegin(GL_LINES);
	glEnable(GL_LINE_SMOOTH);
	//glColor3f(0.0f,0.0f, 1.0f); //白色线条

	int xi = 0;
	int yi = 0;
	int zi = 0;

	//绘制平行于X的直线
	for (zi = 0; zi <= num; zi++)
	{
		float z = _zLen * zi + pt1.Z();
		for (yi = 0; yi <= num; yi++)
		{
			float y = _yLen * yi + pt1.Y();

			glVertex3f(pt1.X(), y, z);
			glVertex3f(pt2.X(), y, z);
		}
	}

	//绘制平行于Y的直线
	for (zi = 0; zi <= num; zi++)
	{

		float z = _zLen * zi + pt1.Z();
		for (xi = 0; xi <= num; xi++)
		{
			float x = _xLen * xi + pt1.X();
			glVertex3f(x, pt1.Y(), z);
			glVertex3f(x, pt2.Y(), z);
		}
	}

	//绘制平行于Z的直线
	for (yi = 0; yi <= num; yi++)
	{
		float y = _yLen * yi + pt1.Y();

		for (xi = 0; xi <= num; xi++)
		{

			float x = _xLen * xi + pt1.X();
			glVertex3f(x, y, pt1.Z());
			glVertex3f(x, y, pt2.Z());
		}
	}

	glEnd();
	glDisable(GL_LINE_STIPPLE);
}

void RoamingScenceManager::GLDrawSpaceAxes(void)
{
	GLUquadricObj *objCylinder = gluNewQuadric();

	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	//glutSolidSphere(0.25, 6, 6);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluCylinder(objCylinder, 0.1, 0.1, AXES_LEN, 10, 5);         //Z
	glTranslatef(0, 0, AXES_LEN);
	gluCylinder(objCylinder, 0.3, 0.0, 0.6, 10, 5);                 //Z
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, AXES_LEN);
	glRotatef(90, 0.0, 1.0, 0.0);
	//drawString("Z");                                               // Print GL Text ToThe Screen
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.0f, 1.0f, 0.0f);
	glRotatef(-90, 1.0, 0.0, 0.0);
	gluCylinder(objCylinder, 0.1, 0.1, AXES_LEN, 10, 5);         //Y
	glTranslatef(0, 0, AXES_LEN);
	gluCylinder(objCylinder, 0.3, 0.0, 0.6, 10, 5);                 //Y
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.5, AXES_LEN, 0);
	//drawString("Y");                                               // Print GL Text ToThe Screen
	glPopMatrix();

	glPushMatrix();
	glColor3f(1.0f, 0.0f, 0.0f);
	glRotatef(90, 0.0, 1.0, 0.0);
	gluCylinder(objCylinder, 0.1, 0.1, AXES_LEN, 10, 5);         //X
	glTranslatef(0, 0, AXES_LEN);
	gluCylinder(objCylinder, 0.3, 0.0, 0.6, 10, 5);                 //X
	glPopMatrix();

	glPushMatrix();
	glTranslatef(AXES_LEN, 0.5, 0);
	//drawString("X");                                               // Print GL Text ToThe Screen
	glPopMatrix();
}

void RoamingScenceManager::drawString(QString& strText, double posX, double posY, double posZ, QFont& font)
{	
	int width = m_viewWidget->width();
	int height = m_viewWidget->height();

	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];

	glPushMatrix();
	//glScalef(0.1, 0.1, 0.1);  
	glGetIntegerv(GL_VIEWPORT, viewport);   /* 获取三个矩阵 */
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
	glPopMatrix();

	GLdouble winx, winy, winz;
	GLdouble posx, posy, posz;

	posx = posX;
	posy = posY;
	posz = posZ;

	gluProject(posx, posy, posz, mvmatrix, projmatrix, viewport, &winx, &winy, &winz); /* 获取三维坐标 */

	winy = height - winy;

	m_viewWidget->renderText(winx, winy, strText, font);
	//if (!m_newFont->isCreateFont())
	//{
	//	m_newFont->CreateFont("Arial", 30, FW_BOLD);
	//}
	//
	//m_newFont->DrawText("Test", 0, 0, 1.0, 1.0, 1.0);	
}

void RoamingScenceManager::drawStringExt(QString& strText, int posX, int posY, QFont& font)
{
	int width = m_viewWidget->width();
	int height = m_viewWidget->height();	

	m_viewWidget->renderText(posX, posY, strText, font);
}