
// Author: Greg Santucci, 2008 and 2009
// This is part of a port of the CSG project
// originally written in java by Danilo Balby
// Email: thecodewitch@gmail.com
// Web: http://createuniverses.blogspot.com/

#include "Solid.h"

#include "Vector.h"

#include "VectorSet.h"
#include "ColorSet.h"
#include "IntSet.h"

#include <fstream>

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <tchar.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <QDebug>

Solid::Solid(const std::string & sFileName, const Color & colBase, int dRed, int dGreen, int dBlue)
{
	m_bBindTexture = false;
	m_bTextureEnable = false;
	m_nSizeX = 0;
	m_nSizeY = 0;
	loadCoordinateFile(sFileName, colBase, dRed, dGreen, dBlue);
}

Solid::Solid(VectorSet * vertices, IntSet * indices, ColorSet * colors)
{
	m_bBindTexture = false;
	m_bTextureEnable = true;
	m_nSizeX = 0;
	m_nSizeY = 0;
	setData(*vertices, *indices, *colors);		
}

Solid::Solid(VectorSet * vertices, IntSet * indices, Palette& palette, int nSizeX, int nSizeY, double dMaxZ, double dMinZ)
{
	m_bBindTexture = false;
	m_bTextureEnable = true;
	m_nSizeX = nSizeX;
	m_nSizeY = nSizeY;

	setData(*vertices, *indices, palette, dMaxZ, dMinZ);
}

Solid::~Solid()
{
}

//---------------------------------------GETS-----------------------------------//

VectorSet * Solid::getVertices()
{
	// This thing makes a fresh copy and hands the requestor the copy.

	VectorSet * newVertices = new VectorSet();

	for(int i = 0; i < vertices.length(); i++)
	{
		newVertices->AddVector(vertices[i]);
	}

	return newVertices;
}

IntSet * Solid::getIndices()
{
	IntSet * newIndices = new IntSet();

	for(int i = 0; i < indices.length(); i++)
	{
		newIndices->AddInt(indices[i]);
	}

	return newIndices;
}

ColorSet * Solid::getColors()
{
	ColorSet * newColors = new ColorSet();

	for(int i = 0; i < colors.length(); i++)
	{
		newColors->AddColor(colors[i]);
	}

	return newColors;
}

Vector Solid::getVertice(int nIndex)
{
	if (nIndex < 0 || nIndex >= vertices.GetSize()) return Vector();

	return vertices[nIndex];
}

bool Solid::isEmpty()
{
	if(indices.length() <= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//---------------------------------------SETS-----------------------------------//

/**
 * Sets the solid data. An exception may occur in the case of abnormal arrays 
 * (indices making references to inexistent vertices, there are less colors 
 * than vertices...)
 * 
 * @param vertices array of points defining the solid vertices
 * @param indices array of indices for a array of vertices
 * @param colors array of colors defining the vertices colors 
 */
void Solid::setData(VectorSet & vertices, IntSet & indices, ColorSet & colors)
{
	// Clear them...
	//this->vertices = new VectorSet();
	//this->colors = new ColorSet();
	//this->indices = new IntSet();

	//this->vertices.Clear();
	//this->colors.Clear();
	//this->indices.Clear();

	if (indices.length() > 0)
	{
		for (int i = 0; i < vertices.length(); i++)
		{
			// Should add cloning to vectorset, and other 2 sets

			this->vertices.AddVector(vertices[i]);
			this->verticesOrig.AddVector(vertices[i]);
			this->colors.AddColor(colors[i]);
			//this->indices.AddInt(indices[i]);
		}

		for (int i = 0; i < indices.length(); i++)
		{
			this->indices.AddInt(indices[i]);
		}
	}
}

void Solid::setData(VectorSet& vertices, IntSet& indices, Palette& palette, double dMaxZ, double dMinZ)
{	
	double height = dMaxZ - dMinZ;
	Color colBase = *palette[palette.size() - 1];
	int nMaxLevel = palette.size() - 1;	

	if (indices.length() > 0)
	{
		for (int i = 0; i < vertices.length(); i++)
		{
			// Should add cloning to vectorset, and other 2 sets
			this->vertices.AddVector(vertices[i]);
			//this->verticesOrig.AddVector((*vertices)[i]);

			int nLevel = 0;
			if (abs(height) > 0.0000001)
			{
				double dZValue = (vertices[i].z - dMinZ) / height;
				double dZNormal = dZValue*dZValue / 3 + dZValue * 2 / 3;
				nLevel = (int)(dZNormal * nMaxLevel);
			}
			colBase = *palette[nMaxLevel - nLevel];

			this->colors.AddColor(colBase);
			//this->indices.AddInt(indices[i]);		
		}

		for (int i = 0; i < indices.length(); i++)
		{
			this->indices.AddInt(indices[i]);			
		}
	}	
}


/**
 * Sets the solid color (all the vertices with the same color)
 * 
 * @param color solid color
 */
void Solid::setColor(const Color & color)
{
	for(int i=0; i < vertices.length(); i++)
	{
		colors.SetColor(i, color);
	}
}

//-------------------------GEOMETRICAL_TRANSFORMATIONS-------------------------//

void Solid::Translate(const Vector & t)
{
	for(int i = 0; i < vertices.GetSize(); i++)
	{
		Vector v = vertices.GetVector(i);

		v = v + t;

		vertices.SetVector(i, v);
	}
}

void Solid::Rotate(const Vector & a, const Vector & b, float angle)
{
	for(int i = 0; i < vertices.GetSize(); i++)
	{
		Vector v = vertices.GetVector(i);

		v = VectorRotate(v, a, b, angle);

		vertices.SetVector(i, v);
	}
}

void Solid::Scale(const Vector & s)
{
	for(int i = 0; i < vertices.GetSize(); i++)
	{
		Vector v = vertices.GetVector(i);

		v = VectorScale(v, s);

		vertices.SetVector(i,v);
	}
}

void Solid::Reset()
{
	if (vertices.GetSize() == verticesOrig.GetSize())
	{
		for (int i = 0; i < vertices.GetSize(); i++)
		{
			Vector v = verticesOrig.GetVector(i);
			vertices.SetVector(i, v);
		}
	}	
}

//-----------------------------------PRIVATES--------------------------------//

void Solid::Render()
{
	glEnable (GL_POLYGON_OFFSET_FILL);
	glPolygonOffset (1., 1.);

	glLineWidth(1);

	glBegin(GL_TRIANGLES);	

	int nNumTriangles = indices.GetSize() / 3;

	for(int i = 0; i < nNumTriangles; i++)
	{
		int nIndex1 = indices.GetInt(i * 3 + 0);
		int nIndex2 = indices.GetInt(i * 3 + 1);
		int nIndex3 = indices.GetInt(i * 3 + 2);

		Vector vP1 = vertices.GetVector(nIndex1);
		Vector vP2 = vertices.GetVector(nIndex2);
		Vector vP3 = vertices.GetVector(nIndex3);		

		Color col1 = colors.GetColor(nIndex1);
		Color col2 = colors.GetColor(nIndex2);
		Color col3 = colors.GetColor(nIndex3);
		
		if (m_bBindTexture) glColor4ub(255, 255, 255, 255);

		if (m_bBindTexture) glTexCoord2f(m_texCoords[nIndex1].x, m_texCoords[nIndex1].y);
		else glColor4ub(col1.red, col1.green, col1.blue, 255);
		glVertex3d(vP1.x, vP1.y, vP1.z);
		
		if (m_bBindTexture) glTexCoord2f(m_texCoords[nIndex2].x, m_texCoords[nIndex2].y);
		else glColor4ub(col2.red, col2.green, col2.blue, 255);
		glVertex3d(vP2.x, vP2.y, vP2.z);
		
		if (m_bBindTexture) glTexCoord2f(m_texCoords[nIndex3].x, m_texCoords[nIndex3].y);
		else glColor4ub(col3.red, col3.green, col3.blue, 255);
		glVertex3d(vP3.x, vP3.y, vP3.z);		
	}

	glEnd();	

	//glLineWidth(1);
	//glBegin(GL_LINES);
	//for(int i = 0; i < nNumTriangles; i++)
	//{
	//	int nIndex1 = indices.GetInt(i * 3 + 0);
	//	int nIndex2 = indices.GetInt(i * 3 + 1);
	//	int nIndex3 = indices.GetInt(i * 3 + 2);

	//	Vector vP1 = vertices.GetVector(nIndex1);
	//	Vector vP2 = vertices.GetVector(nIndex2);
	//	Vector vP3 = vertices.GetVector(nIndex3);

	//	//float fScale = 0.5f;
	//	//vP1 *= fScale;
	//	//vP2 *= fScale;
	//	//vP3 *= fScale;

	//	//Vector vTranslation(50,20,50);
	//	//vP1 += vTranslation;
	//	//vP2 += vTranslation;
	//	//vP3 += vTranslation;

	//	Color col1 = colors.GetColor(nIndex1);
	//	Color col2 = colors.GetColor(nIndex2);
	//	Color col3 = colors.GetColor(nIndex3);

	//	glColor4ub(255, 255, 255, 255);
	//	//glColor4ub(200,200,200, 255);
	//	//glColor4ub(127,127,127, 255);
	//	//glColor4ub(0,0,0, 255);

	//	glVertex3d(vP1.x, vP1.y, vP1.z);
	//	glVertex3d(vP2.x, vP2.y, vP2.z);

	//	glVertex3d(vP2.x, vP2.y, vP2.z);
	//	glVertex3d(vP3.x, vP3.y, vP3.z);

	//	glVertex3d(vP3.x, vP3.y, vP3.z);
	//	glVertex3d(vP1.x, vP1.y, vP1.z);
	//}
	//glEnd();

	glLineWidth(1);
	glColor4ub(255,255,255, 255);
}

/**
 * Gets the solid mean
 * 
 * @return point representing the mean
 */
Vector Solid::getMean()
{
	Vector mean;
	for(int i=0;i<vertices.length();i++)
	{
		mean.x += vertices[i].x;
		mean.y += vertices[i].y;
		mean.z += vertices[i].z;
	}
	mean.x /= vertices.length();
	mean.y /= vertices.length();
	mean.z /= vertices.length();
	
	return mean;
}

void Solid::getSize(double& dSizeMaxX, double& dSizeMinX, double& dSizeMaxY, double& dSizeMinY, double& dSizeMaxZ, double& dSizeMinZ)
{
	double xMax = -1000000, xMin = 1000000, yMax = -1000000, yMin = 1000000, zMax = -1000000, zMin = 1000000;
	for (int i = 0; i < vertices.length(); i++)
	{
		if (vertices[i].x > xMax)
		{
			xMax = vertices[i].x;
		}
		if (vertices[i].x < xMin)
		{
			xMin = vertices[i].x;
		}
		if (vertices[i].y > yMax)
		{
			yMax = vertices[i].y;
		}

		if (vertices[i].y < yMin)
		{
			yMin = vertices[i].y;
		}

		if (vertices[i].z > zMax)
		{
			zMax = vertices[i].z;
		}

		if (vertices[i].z < zMin)
		{
			zMin = vertices[i].z;
		}
	}

	dSizeMaxX = xMax;
	dSizeMinX = xMin;
	dSizeMaxY = yMax;
	dSizeMinY = yMin;
	dSizeMaxZ = zMax;
	dSizeMinZ = zMin;
}

/**
 * Loads a coordinates file, setting vertices and indices 
 * 
 * @param solidFile file used to create the solid
 * @param color solid color
 */
void Solid::loadCoordinateFile(const std::string & sFileName, const Color & colBase, int dRed, int dGreen, int dBlue)
{
	std::ifstream modelFile;

	modelFile.open(sFileName.c_str());

	int nNumVertices = 0;
	modelFile >> nNumVertices;

	for(int i = 0; i < nNumVertices; i++)
	{
		// Read in a vector.
		int nVertexID = 0;

		Vector vPosition;

		modelFile >> nVertexID;

		modelFile >> vPosition.x;
		modelFile >> vPosition.y;
		modelFile >> vPosition.z;

		//vPosition = VectorScale(vPosition, Vector(0.5f, 0.5f, 0.5f));
		vPosition = VectorScale(vPosition, Vector(1.0f, 1.0f, 1.0f));

		vertices.AddVector(vPosition);
		verticesOrig.AddVector(vPosition);
	}

	// Now load up the indices.

	int nNumTriangles = 0;
	modelFile >> nNumTriangles;

	for(int i = 0; i < nNumTriangles; i++)
	{
		int nTriangleID = 0;

		int nTriIndex1 = 0;
		int nTriIndex2 = 0;
		int nTriIndex3 = 0;

		modelFile >> nTriangleID;

		modelFile >> nTriIndex1;
		modelFile >> nTriIndex2;
		modelFile >> nTriIndex3;

		indices.AddInt(nTriIndex1);
		indices.AddInt(nTriIndex2);
		indices.AddInt(nTriIndex3);
	}

	modelFile.close();

	for(int i = 0; i < nNumVertices; i++)
	{
		Color col = colBase;

		if(dRed < 0) col.red -= rand() % (-1*dRed); else if(dRed > 0) col.red += rand() % dRed;
		if(dGreen < 0) col.green -= rand() % (-1*dGreen); else if(dGreen > 0) col.green += rand() % dGreen;
		if(dBlue < 0) col.blue -= rand() % (-1*dBlue); else if(dBlue > 0) col.blue += rand() % dBlue;

		colors.AddColor(col);
	}

	//defineGeometry();
}

bool Solid::isEnableTexture()
{
	return m_bTextureEnable;
}

bool Solid::isBindTexture()
{
	return m_bBindTexture;
}

void Solid::bindTexture(QImage& img, bool invert)
{
	if (m_bTextureEnable)
	{
		glGenTextures(1, &m_idTexture);
		glBindTexture(GL_TEXTURE_2D, m_idTexture);

		qDebug() << "texture image: " << img.width() << " " << img.height();

		glTexImage2D(GL_TEXTURE_2D, 0, 3, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR/*GL_NEAREST*/);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		//glTexCoord2f(0.0, 1.0);
		if (m_nSizeX > 0 && m_nSizeY > 0)
		{
			int nNumX = invert ? m_nSizeY : m_nSizeX;
			int nNumY = invert ? m_nSizeX : m_nSizeY;

			for (int i = 0; i < nNumX; i++)
			{
				float px = ((float)i) / ((float)(nNumX - 1));          //计算v方向上的细分点的参数  
				
				for (int j = 0; j < nNumY; j++)
				{
					float py = ((float)j) / ((float)(nNumY - 1));          //计算v方向上的细分点的参数  

					m_texCoords.AddVector(Vector(px, 1.0 - py, 0));
				}
			}
		}

		m_bBindTexture = true;
	}
}

void Solid::clearTexture()
{
	if (m_bTextureEnable)
	{
		m_texCoords.clear();
		glDeleteTextures(1, &m_idTexture);
		m_idTexture = 0;
		m_bBindTexture = false;
	}
}
