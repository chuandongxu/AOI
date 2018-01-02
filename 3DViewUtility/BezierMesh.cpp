#include "BezierMesh.h"

BezierMesh::BezierMesh(QObject *parent)
	: QObject(parent)
{
	m_ShowCPoints = true;	
	m_Divs = 7;

	initBezier();
}

BezierMesh::~BezierMesh()
{
}

void BezierMesh::init(GLuint texture)
{
	m_Mybezier.texture = texture;
	//m_Mybezier.texture = bindTexture(QPixmap("D:/QtOpenGL/QtImage/NeHe.bmp"));
	glEnable(GL_TEXTURE_2D);                            //启用纹理映射  
	m_Mybezier.dlBPatch = genBezier();

	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);               //黑色背景  
	//glShadeModel(GL_SMOOTH);                            //启用阴影平滑  
	//glClearDepth(1.0);                                  //设置深度缓存  
	//glEnable(GL_DEPTH_TEST);                            //启用深度测试  
	//glDepthFunc(GL_LEQUAL);                             //所作深度测试的类型  
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  //告诉系统对透视进行修正  
}

void BezierMesh::render()
{
	glCallList(m_Mybezier.dlBPatch);                    //调用显示列表，绘制贝塞尔曲面 

	if (m_ShowCPoints)                                  //是否绘制控制点  
	{
		glDisable(GL_TEXTURE_2D);                       //禁用纹理贴图  
		glColor3f(1.0f, 0.0f, 0.0f);                    //设置颜色为红色  
		for (int i = 0; i < 4; i++)                         //绘制水平线  
		{
			glBegin(GL_LINE_STRIP);
			for (int j = 0; j < 4; j++)
			{
				glVertex3d(m_Mybezier.anchors[i][j].x,
					m_Mybezier.anchors[i][j].y,
					m_Mybezier.anchors[i][j].z);
			}
			glEnd();
		}
		for (int i = 0; i < 4; i++)                         //绘制垂直线  
		{
			glBegin(GL_LINE_STRIP);
			for (int j = 0; j < 4; j++)
			{
				glVertex3d(m_Mybezier.anchors[j][i].x,
					m_Mybezier.anchors[j][i].y,
					m_Mybezier.anchors[j][i].z);
			}
			glEnd();
		}
		glColor3f(1.0f, 1.0f, 1.0f);                    //恢复OpenGL属性  
		glEnable(GL_TEXTURE_2D);
	}
}

void BezierMesh::initBezier()                           //初始化贝塞尔曲面  
{
	//设置贝塞尔曲面的控制点  
	m_Mybezier.anchors[0][0] = Vector(-75, -75, -50);
	m_Mybezier.anchors[0][1] = Vector(-25, -75, 00);
	m_Mybezier.anchors[0][2] = Vector(25, -75, 00);
	m_Mybezier.anchors[0][3] = Vector(75, -75, -50);
	m_Mybezier.anchors[1][0] = Vector(-75, -25, -75);
	m_Mybezier.anchors[1][1] = Vector(-25, -25, 50);
	m_Mybezier.anchors[1][2] = Vector(25, -25, 50);
	m_Mybezier.anchors[1][3] = Vector(75, -25, -75);
	m_Mybezier.anchors[2][0] = Vector(-75, 25, 0.00);
	m_Mybezier.anchors[2][1] = Vector(-25, 25, -50);
	m_Mybezier.anchors[2][2] = Vector(25, 25, -50);
	m_Mybezier.anchors[2][3] = Vector(75, 25, 0.00);
	m_Mybezier.anchors[3][0] = Vector(-75, 75, -50);
	m_Mybezier.anchors[3][1] = Vector(-25, 75, -00);
	m_Mybezier.anchors[3][2] = Vector(25, 75, -00);
	m_Mybezier.anchors[3][3] = Vector(75, 75, -50);

	m_Mybezier.dlBPatch = 0;                            //默认的显示列表为0  
}

Vector BezierMesh::bernstein(float u, Vector *p)    //计算贝塞尔方程的值  
{
	Vector a = p[0] * pow(u, 3);
	Vector b = p[1] * (3 * pow(u, 2)*(1 - u));
	Vector c = p[2] * (3 * u*pow(1 - u, 2));
	Vector d = p[3] * pow(1 - u, 3);

	Vector r = a + b + c + d;
	return r;
}

GLuint BezierMesh::genBezier()                          //生成贝塞尔曲面的显示列表  
{
	GLuint drawlist = glGenLists(1);                    //分配1个显示列表的空间  
	Vector temp[4];
	//根据每一条曲线的细分数，分配相应的内存  
	Vector *last = (Vector*)malloc(sizeof(Vector)*(m_Divs + 1));

	if (m_Mybezier.dlBPatch != 0)                       //如果显示列表存在，则删除  
	{
		glDeleteLists(m_Mybezier.dlBPatch, 1);
	}

	temp[0] = m_Mybezier.anchors[0][3];                 //获得u方向的四个控制点  
	temp[1] = m_Mybezier.anchors[1][3];
	temp[2] = m_Mybezier.anchors[2][3];
	temp[3] = m_Mybezier.anchors[3][3];

	for (int v = 0; v <= m_Divs; v++)                       //根据细分数，创建各个分割点的参数  
	{
		float py = ((float)v) / ((float)m_Divs);
		last[v] = bernstein(py, temp);                  //使用bernstein函数求得分割点坐标  
	}

	glNewList(drawlist, GL_COMPILE);                    //绘制一个新的显示列表  
	glBindTexture(GL_TEXTURE_2D, m_Mybezier.texture);   //绑定纹理  
	for (int u = 1; u <= m_Divs; u++)
	{
		float px = ((float)u) / ((float)m_Divs);          //计算v方向上的细分点的参数  
		float pxold = ((float)u - 1.0f) / ((float)m_Divs);  //上一个v方向的细分点的参数  

		temp[0] = bernstein(px, m_Mybezier.anchors[0]); //计算每个细分点v方向上贝塞尔曲面的控制点  
		temp[1] = bernstein(px, m_Mybezier.anchors[1]);
		temp[2] = bernstein(px, m_Mybezier.anchors[2]);
		temp[3] = bernstein(px, m_Mybezier.anchors[3]);

		glBegin(GL_TRIANGLE_STRIP);                     //开始绘制三角形带  
		for (int v = 0; v <= m_Divs; v++)
		{
			float py = ((float)v) / ((float)m_Divs);  //沿着u方向顺序绘制  
			glTexCoord2f(pxold, py);                //设置纹理坐标并绘制一个顶点  
			glVertex3d(last[v].x, last[v].y, last[v].z);

			last[v] = bernstein(py, temp);          //计算下一个顶点  
			glTexCoord2f(px, py);                   //设置纹理坐标并绘制新的顶点  
			glVertex3d(last[v].x, last[v].y, last[v].z);
		}
		glEnd();                                        //结束三角形带的绘制  
	}
	glEndList();                                        //显示列表绘制结束  

	free(last);                                         //释放分配的内存  
	return drawlist;                                    //返回创建的显示列表  
}
