
// Author: Greg Santucci, 2008 and 2009
// This is part of a port of the CSG project
// originally written in java by Danilo Balby
// Email: thecodewitch@gmail.com
// Web: http://createuniverses.blogspot.com/

#include <QObject>
#include <QImage>

#ifndef SOLID_H
#define SOLID_H

#include "Color.h"

#include "IntSet.h"
#include "VectorSet.h"
#include "ColorSet.h"

#include <string>

typedef unsigned int GLuint;

class Solid
{
public:

	IntSet indices;
	VectorSet vertices;
	VectorSet verticesOrig;
	ColorSet colors;

	typedef std::vector<Color *> Palette;
	
	Solid(const std::string & sFileName, const Color & colBase, int dRed, int dGreen, int dBlue);
	Solid(VectorSet * vertices, IntSet * indices, ColorSet * colors);
	Solid(VectorSet * vertices, IntSet * indices, Palette& palette, int nSizeX, int nSizeY, double dMaxZ, double dMinZ);
	virtual ~Solid();
	
	VectorSet * getVertices();
	IntSet * getIndices();
	ColorSet * getColors();

	Vector getVertice(int nIndex);

	bool isEmpty();

	void setData(VectorSet & vertices, IntSet & indices, ColorSet & colors);
	void setData(VectorSet & vertices, IntSet & indices, Palette& palette, double dMaxZ, double dMinZ);
	
	void setColor(const Color & color);

	void Translate(const Vector & t);
	void Rotate(const Vector & a, const Vector & b, float angle);
	void Scale(const Vector & s);
	void Reset();

	void Render();

	void loadCoordinateFile(const std::string & sFileName, const Color & colBase, int dRed, int dGreen, int dBlue);

	Vector getMean();

	void getSize(double& dSizeMaxX, double& dSizeMinX, double& dSizeMaxY, double& dSizeMinY, double& dSizeMaxZ, double& dSizeMinZ);


public:
	bool isEnableTexture();
	bool isBindTexture();
	void bindTexture(QImage& img, bool invert);
	void clearTexture();

private:
	VectorSet m_texCoords;
	bool m_bBindTexture;
	bool m_bTextureEnable;
	GLuint m_idTexture;
	int m_nSizeX;
	int m_nSizeY;
};

#endif // SOLID_H
