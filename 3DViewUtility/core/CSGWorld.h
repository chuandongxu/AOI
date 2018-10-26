
// Author: Greg Santucci, 2009
// Email: thecodewitch@gmail.com
// Web: http://createuniverses.blogspot.com/

#ifndef CSGWORLD_H
#define CSGWORLD_H

#include <QObject>
#include <QImage>
#include <vector>

class BooleanModeller;
class Solid;
class Color;
class VectorSet;
class Vector;
class IntSet;

class CSGWorld
{
public:
	CSGWorld();
	virtual ~CSGWorld();

	void ClearSolids();

	void LoadObj(VectorSet * vertices, IntSet * indices, int nSizeX, int nSizeY, double dVerMaxZ, double dVerMinZ);

	void LoadCube();
	void LoadSphere();
	void LoadCone();
	void LoadCylinder();

	int GetNumSolids();
	Solid * GetSolid(int i);
	void popSolid();

	void Difference(int i, int j);
	void Union(int i, int j);
	void Intersection(int i, int j);

	void init();
	void Render(int nNum = 0);
    void rotate(int nNum, Vector& p1, Vector& p2, float angle);

	typedef std::vector<Solid *> SolidSet;
	typedef std::vector<bool> BoolSet;
	typedef std::vector<Color *> Palette;

	SolidSet m_Solids;
	BoolSet m_RenderSwitchboard;

	int m_nPaletteIndex;
	Palette m_Palette;

public:
	bool isBindTexture();
	void bindTexture(QImage& img, bool invert);
	void clearTexture();
};

#endif // CSGWORLD_H
