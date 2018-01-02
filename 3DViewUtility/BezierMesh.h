#pragma once

#include <QObject>

#include <QtOpenGL/QGLWidget>
#include <QtGui/QMouseEvent>

#include <vector>

#include <Core/Vector.h>


class BezierMesh : public QObject
{
	Q_OBJECT

public:
	BezierMesh(QObject *parent = NULL);
	~BezierMesh();

public:
	void init(GLuint texture);
	void render();

private:
	Vector bernstein(float u, Vector *p);			//计算贝塞尔方程的值  
	GLuint genBezier();                             //生成贝塞尔曲面的显示列表 
	void initBezier();                              //初始化贝塞尔曲面 

private:	 
	bool m_ShowCPoints;                             //是否显示控制点  
	int m_Divs;                                     //细分数  

	struct BEZIER_PATCH                             //贝塞尔曲面结构体  
	{
		Vector anchors[4][4];						 //控制点坐标  
		GLuint dlBPatch;                            //储存显示列表地址  
		GLuint texture;                             //储存绘制的纹理  
	} m_Mybezier;                                   //储存要绘制的贝塞尔曲面数
};
