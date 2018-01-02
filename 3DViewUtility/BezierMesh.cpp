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
	glEnable(GL_TEXTURE_2D);                            //��������ӳ��  
	m_Mybezier.dlBPatch = genBezier();

	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);               //��ɫ����  
	//glShadeModel(GL_SMOOTH);                            //������Ӱƽ��  
	//glClearDepth(1.0);                                  //������Ȼ���  
	//glEnable(GL_DEPTH_TEST);                            //������Ȳ���  
	//glDepthFunc(GL_LEQUAL);                             //������Ȳ��Ե�����  
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  //����ϵͳ��͸�ӽ�������  
}

void BezierMesh::render()
{
	glCallList(m_Mybezier.dlBPatch);                    //������ʾ�б����Ʊ��������� 

	if (m_ShowCPoints)                                  //�Ƿ���ƿ��Ƶ�  
	{
		glDisable(GL_TEXTURE_2D);                       //����������ͼ  
		glColor3f(1.0f, 0.0f, 0.0f);                    //������ɫΪ��ɫ  
		for (int i = 0; i < 4; i++)                         //����ˮƽ��  
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
		for (int i = 0; i < 4; i++)                         //���ƴ�ֱ��  
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
		glColor3f(1.0f, 1.0f, 1.0f);                    //�ָ�OpenGL����  
		glEnable(GL_TEXTURE_2D);
	}
}

void BezierMesh::initBezier()                           //��ʼ������������  
{
	//���ñ���������Ŀ��Ƶ�  
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

	m_Mybezier.dlBPatch = 0;                            //Ĭ�ϵ���ʾ�б�Ϊ0  
}

Vector BezierMesh::bernstein(float u, Vector *p)    //���㱴�������̵�ֵ  
{
	Vector a = p[0] * pow(u, 3);
	Vector b = p[1] * (3 * pow(u, 2)*(1 - u));
	Vector c = p[2] * (3 * u*pow(1 - u, 2));
	Vector d = p[3] * pow(1 - u, 3);

	Vector r = a + b + c + d;
	return r;
}

GLuint BezierMesh::genBezier()                          //���ɱ������������ʾ�б�  
{
	GLuint drawlist = glGenLists(1);                    //����1����ʾ�б�Ŀռ�  
	Vector temp[4];
	//����ÿһ�����ߵ�ϸ������������Ӧ���ڴ�  
	Vector *last = (Vector*)malloc(sizeof(Vector)*(m_Divs + 1));

	if (m_Mybezier.dlBPatch != 0)                       //�����ʾ�б���ڣ���ɾ��  
	{
		glDeleteLists(m_Mybezier.dlBPatch, 1);
	}

	temp[0] = m_Mybezier.anchors[0][3];                 //���u������ĸ����Ƶ�  
	temp[1] = m_Mybezier.anchors[1][3];
	temp[2] = m_Mybezier.anchors[2][3];
	temp[3] = m_Mybezier.anchors[3][3];

	for (int v = 0; v <= m_Divs; v++)                       //����ϸ���������������ָ��Ĳ���  
	{
		float py = ((float)v) / ((float)m_Divs);
		last[v] = bernstein(py, temp);                  //ʹ��bernstein������÷ָ������  
	}

	glNewList(drawlist, GL_COMPILE);                    //����һ���µ���ʾ�б�  
	glBindTexture(GL_TEXTURE_2D, m_Mybezier.texture);   //������  
	for (int u = 1; u <= m_Divs; u++)
	{
		float px = ((float)u) / ((float)m_Divs);          //����v�����ϵ�ϸ�ֵ�Ĳ���  
		float pxold = ((float)u - 1.0f) / ((float)m_Divs);  //��һ��v�����ϸ�ֵ�Ĳ���  

		temp[0] = bernstein(px, m_Mybezier.anchors[0]); //����ÿ��ϸ�ֵ�v�����ϱ���������Ŀ��Ƶ�  
		temp[1] = bernstein(px, m_Mybezier.anchors[1]);
		temp[2] = bernstein(px, m_Mybezier.anchors[2]);
		temp[3] = bernstein(px, m_Mybezier.anchors[3]);

		glBegin(GL_TRIANGLE_STRIP);                     //��ʼ���������δ�  
		for (int v = 0; v <= m_Divs; v++)
		{
			float py = ((float)v) / ((float)m_Divs);  //����u����˳�����  
			glTexCoord2f(pxold, py);                //�����������겢����һ������  
			glVertex3d(last[v].x, last[v].y, last[v].z);

			last[v] = bernstein(py, temp);          //������һ������  
			glTexCoord2f(px, py);                   //�����������겢�����µĶ���  
			glVertex3d(last[v].x, last[v].y, last[v].z);
		}
		glEnd();                                        //���������δ��Ļ���  
	}
	glEndList();                                        //��ʾ�б���ƽ���  

	free(last);                                         //�ͷŷ�����ڴ�  
	return drawlist;                                    //���ش�������ʾ�б�  
}
