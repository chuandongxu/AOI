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
	Vector bernstein(float u, Vector *p);			//���㱴�������̵�ֵ  
	GLuint genBezier();                             //���ɱ������������ʾ�б� 
	void initBezier();                              //��ʼ������������ 

private:	 
	bool m_ShowCPoints;                             //�Ƿ���ʾ���Ƶ�  
	int m_Divs;                                     //ϸ����  

	struct BEZIER_PATCH                             //����������ṹ��  
	{
		Vector anchors[4][4];						 //���Ƶ�����  
		GLuint dlBPatch;                            //������ʾ�б��ַ  
		GLuint texture;                             //������Ƶ�����  
	} m_Mybezier;                                   //����Ҫ���Ƶı�����������
};
