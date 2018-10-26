#pragma once

#include <QObject>

#include <QtOpenGL/QGLWidget>
#include <QtGui/QMouseEvent>

#include <vector>
#include "./core/Vector.h"

enum ViewAction
{	
	ACTION_MOVE,
	ACTION_ROTATE,
	ACTION_SELECT,
	ACTION_CSG_MESH,
	ACTION_NULL
};

class Vector3D;
class Position3D;
class Matrix4D;

class CSGWorld;
class BooleanModeller;
class Solid;
class VectorSet;

class CSGMesh;
class RoamingScenceManager;
class IGraphicEditor;
class ViewWidget : public QGLWidget
{
	Q_OBJECT

public:
	ViewWidget(QWidget *parent = 0);
	~ViewWidget();

private:
	int m_width, m_height;
	int m_nLeft, m_nTop;

private:
	bool isCSGBoolAlg();
	bool isBilinearInterpolationAlg();
	bool is3DUnprojectAlg();

public:
	void loadFile(QString& fileName);
	void loadFile(bool invert, int nSizeX, int nSizeY, QVector<Vector>& values);
	void initData();

	void zoomIn();
	void zoomOut();
	void moveView();
	void rotateView();
	void fullScreen();
	void frontView();
	void topView();
	void sideView();

	void changePointCloudView();
	void changeMeshTriView(bool bPrmpt = true);
	void changeDataToCenter(bool bChangeToCenter);
	
	void csgCutMesh();
	void setBasePos(int nLeft, int nTop);

	void meshTexture();
	void meshTexture(QImage& textrue, bool invert);

	void setViewAction(ViewAction action);

	void hideView();
private:
	ViewAction m_viewAction;

protected:
	//void paintEvent(QPaintEvent *event);
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *);

private:
    void excuteRotate(int x, int y);

private:
	void cutingMesh(double dPosX1, double dPosY1, double dPosX2, double dPosY2);
	void changeCutingMesh(Vector pt1, Vector pt2);

private:
	bool writeBMP(const char filename[], unsigned char* data, unsigned int w, unsigned int h);
	void updateCutingData(double x1, double y1, double x2, double y2);

private:
	void solidCylinder(GLdouble radius, GLdouble height, GLint slices, GLint stacks);
	GLuint solidLine(double posX1, double posY1, double posX2, double posY2);
	GLuint solidPlane(double posX1, double posY1, double posZ1, double posX2, double posY2, double posZ2);

	void cutingUsingCSGCube(double posX1, double posY1, double posX2, double posY2);
	void showCrossSectionAsCSG(double angle, double slop);

	void cutingUsingBilinearInterpolation(double posX1, double posY1, double posX2, double posY2);
	void cutingUsing3DUnproject(double posX1, double posY1, double posX2, double posY2);	

	bool ScreenPtToViewPt(double i_ScreenPtX, double i_ScreenPtY, double* o_ViewPointX, double* o_ViewPointY, double* o_ViewPointZ);
	bool ScreenPtToViewPtExt(double i_ScreenPtX, double i_ScreenPtY, double o_ViewPointZ, double* o_ViewPointX, double* o_ViewPointY);

	bool isPointNearPoint(Vector pt1, Vector pt2, double dDist = 1);

	void getLineLimitPos(double* posX, double* posY, double dExtLen, double* posXExt, double* posYExt);
	//bool isPointInMesh(double posX, double posY, double posZ);
private:
	RoamingScenceManager*manager;
	CSGWorld* m_csgWorld;
	BooleanModeller*	m_pBooleanModeller;	
	Solid*				m_pDifferenceModel;	

	CSGMesh* m_csgMesh;

	bool m_bShowCloudPoint;
	bool m_bShowMeshTriangulation;
	bool m_bShowMeshSurfaceCute;

private:
	GLuint m_idCutMeshLine;
	GLuint m_idCutMesh;
	Vector m_cutMeshP1;
	Vector m_cutMeshP2;
	int    m_nCutMeshSelect;

private:
	int m_nPosXStart, m_nPosYStart;
	int m_nPosXPre, m_nPosYPre;
	double m_dMeshSizeMaxX;
	double m_dMeshSizeMinX;
	double m_dMeshSizeMaxY;
	double m_dMeshSizeMinY;
	double m_dMeshSizeMaxZ;
	double m_dMeshSizeMinZ;	

private:
	IGraphicEditor *m_pGraphicsEditor;

    Position3D *OldMouse;
    Position3D *Mouse;
};
