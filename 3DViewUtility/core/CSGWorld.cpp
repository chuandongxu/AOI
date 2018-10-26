
// Author: Greg Santucci, 2009
// Email: thecodewitch@gmail.com
// Web: http://createuniverses.blogspot.com/

#include "CSGWorld.h"

#include "BooleanModeller.h"
#include "Solid.h"
#include "Color.h"
#include "IntSet.h"
#include "FaceSet.h"
#include "VertexSet.h"
#include "VectorSet.h"
#include "ColorSet.h"

#include "Color.h"

#include <stdlib.h>
#include <time.h>
#include <qmath.h>

CSGWorld * g_pCSGWorld = 0;

CSGWorld::CSGWorld()
{
	srand(::time(0));

	g_pCSGWorld = this;	

	m_nPaletteIndex = 0;

	int nStepNum = 10;
	int nStep = 255 / nStepNum;
	int nSupperStepNum = 30;
	int nSupperStep = 255 / nSupperStepNum;
	m_Palette.push_back(new Color(255, 0, 0));
	for (int i = 0; i < nStepNum; i++)
	{
		m_Palette.push_back(new Color(255, i*nStep, 0));
	}
	m_Palette.push_back(new Color(255, 255, 0));
	for (int i = 0; i < nStepNum; i++)
	{
		m_Palette.push_back(new Color(255 - i*nStep, 255, 0));
	}
	m_Palette.push_back(new Color(0, 255, 0));
	for (int i = 0; i < nStepNum; i++)
	{
		m_Palette.push_back(new Color(0, 255, i*nStep));
	}
	m_Palette.push_back(new Color(0, 255, 255));
	for (int i = 0; i < nSupperStepNum; i++)
	{
		m_Palette.push_back(new Color(0, 255 - i*nSupperStep, 255));
	}
	m_Palette.push_back(new Color(0, 0, 255));
	for (int i = 0; i < nSupperStepNum; i++)
	{
		m_Palette.push_back(new Color(0, 0, 255 - i*nSupperStep * 2 / 3));
	}
	
	//m_Palette.push_back(new Color(217, 236, 255));
	//m_Palette.push_back(new Color(198, 226, 255));
	//m_Palette.push_back(new Color(183, 219, 255));
	//m_Palette.push_back(new Color(164, 209, 255));
	//m_Palette.push_back(new Color(149, 202, 255));
	//m_Palette.push_back(new Color(132, 192, 255));
	//m_Palette.push_back(new Color(113, 184, 255));
	//m_Palette.push_back(new Color(96, 175, 255));
	//m_Palette.push_back(new Color(79, 167, 255));
	//m_Palette.push_back(new Color(62, 158, 255));
	//m_Palette.push_back(new Color(47, 151, 255));
	//m_Palette.push_back(new Color(28, 141, 255));
	//m_Palette.push_back(new Color(11, 133, 255));
	//m_Palette.push_back(new Color(0, 124, 249));
	//m_Palette.push_back(new Color(0, 116, 233));
	//m_Palette.push_back(new Color(0, 106, 213));
	//m_Palette.push_back(new Color(0, 99, 198));
	//m_Palette.push_back(new Color(0, 90, 181));
	//m_Palette.push_back(new Color(0, 83, 166));
	//m_Palette.push_back(new Color(0, 73, 147));
	//m_Palette.push_back(new Color(0, 68, 136));
	//m_Palette.push_back(new Color(0, 57, 115));
	//m_Palette.push_back(new Color(0, 49, 98));
	//m_Palette.push_back(new Color(0, 41, 83));


	/*m_Palette.push_back(new Color(255, 0, 0));
	m_Palette.push_back(new Color(255, 75, 0));
	m_Palette.push_back(new Color(255, 165, 0));
	m_Palette.push_back(new Color(255, 200, 0));
	m_Palette.push_back(new Color(255, 255, 0));
	m_Palette.push_back(new Color(125, 255, 0));
	m_Palette.push_back(new Color(0, 255, 0));
	m_Palette.push_back(new Color(0, 255, 255));
	m_Palette.push_back(new Color(0, 127, 255));
	m_Palette.push_back(new Color(0, 65, 255));
	m_Palette.push_back(new Color(0, 0, 255));
	m_Palette.push_back(new Color(75, 0, 255));
	m_Palette.push_back(new Color(139, 0, 255));*/


	/*m_Palette.push_back(new Color(255,  0,    0  ));
	m_Palette.push_back(new Color(0,    255,  0  ));
	m_Palette.push_back(new Color(0,    0,    255));

	m_Palette.push_back(new Color(255,  255,  0  ));
	m_Palette.push_back(new Color(0,    255,  255));
	m_Palette.push_back(new Color(255,  0,    255));

	m_Palette.push_back(new Color(255,  127,  127));
	m_Palette.push_back(new Color(127,  255,  127));
	m_Palette.push_back(new Color(127,  127,  255));

	m_Palette.push_back(new Color(255,  255,  127));
	m_Palette.push_back(new Color(127,  255,  255));
	m_Palette.push_back(new Color(255,  127,  255));*/
}

CSGWorld::~CSGWorld()
{
	for(int i = 0; i < m_Solids.size(); i++)
	{
		Solid * pSolid = m_Solids[i];
		delete pSolid;
	}

	for(int i = 0; i < m_Palette.size(); i++)
	{
		Color * pColor = m_Palette[i];
		delete pColor;
	}
}

void CSGWorld::ClearSolids()
{
	for (int i = 0; i < m_Solids.size(); i++)
	{
		Solid * pSolid = m_Solids[i];
		delete pSolid;
	}

	m_Solids.clear();
	m_nPaletteIndex = 0;
}

void CSGWorld::LoadObj(VectorSet * vertices, IntSet * indices, int nSizeX, int nSizeY, double dVerMaxZ, double dVerMinZ)
{
	//Color colBase = *m_Palette[m_Palette.size() - 1];

	//double zMin = dVerMinZ, zMax = dVerMaxZ, height = 0;	
	//height = zMax - zMin;

	//int nMaxLevel = m_Palette.size() - 1;
	//ColorSet colors;
	//for (int i = 0; i < vertices->GetSize(); i++)
	//{
	//	Vector vector = vertices->GetVector(i);
	//	int nLevel = 0;
	//	if (qAbs(height) > 0.0000001)
	//	{

	//		nLevel = (int)(((vector.z - zMin) / height) * nMaxLevel);

	//	}
	//	colBase = *m_Palette[nMaxLevel - nLevel];
	//	colors.AddColor(colBase);
	//}

	//Solid * pSolid = new Solid(vertices, indices, &colors);
	Solid * pSolid = new Solid(vertices, indices, m_Palette, nSizeX, nSizeY, dVerMaxZ, dVerMinZ);

	m_Solids.push_back(pSolid);
	m_RenderSwitchboard.push_back(true);	
}

//void CSGWorld::LoadObj(VectorSet * vertices, IntSet * indices)
//{
//	Color colBase = *m_Palette[m_nPaletteIndex];
//	int dRed = 0, dGreen = 0, dBlue = 0;
//
//	ColorSet colors;
//	for (int i = 0; i < vertices->GetSize(); i++)
//	{
//		Color col = colBase;
//
//		if (dRed < 0) col.red -= rand() % (-1 * dRed); else if (dRed > 0) col.red += rand() % dRed;
//		if (dGreen < 0) col.green -= rand() % (-1 * dGreen); else if (dGreen > 0) col.green += rand() % dGreen;
//		if (dBlue < 0) col.blue -= rand() % (-1 * dBlue); else if (dBlue > 0) col.blue += rand() % dBlue;
//
//		colors.AddColor(col);
//	}
//
//	Solid * pSolid = new Solid(vertices, indices, &colors);
//
//	m_Solids.push_back(pSolid);
//	m_RenderSwitchboard.push_back(true);
//	m_nPaletteIndex = (m_nPaletteIndex + 1) % m_Palette.size();
//}

void CSGWorld::LoadCube()
{
	Solid * pSolid = new Solid("3D/solid_data/box.txt", *m_Palette[m_nPaletteIndex], 0,0,0);

	m_Solids.push_back(pSolid);
	m_RenderSwitchboard.push_back(true);
	m_nPaletteIndex = (m_nPaletteIndex + 1) % m_Palette.size();
}

void CSGWorld::LoadSphere()
{
	Solid * pSolid = new Solid("3D/solid_data/sphere.txt", *m_Palette[m_nPaletteIndex], 0,0,0);

	m_Solids.push_back(pSolid);
	m_RenderSwitchboard.push_back(true);
	m_nPaletteIndex = (m_nPaletteIndex + 1) % m_Palette.size();
}

void CSGWorld::LoadCone()
{
	Solid * pSolid = new Solid("3D/solid_data/cone.txt", *m_Palette[m_nPaletteIndex], 0,0,0);

	m_Solids.push_back(pSolid);
	m_RenderSwitchboard.push_back(true);
	m_nPaletteIndex = (m_nPaletteIndex + 1) % m_Palette.size();
}

void CSGWorld::LoadCylinder()
{
	Solid * pSolid = new Solid("3D/solid_data/cylinder.txt", *m_Palette[m_nPaletteIndex], 0,0,0);

	m_Solids.push_back(pSolid);
	m_RenderSwitchboard.push_back(true);
	m_nPaletteIndex = (m_nPaletteIndex + 1) % m_Palette.size();
}

int CSGWorld::GetNumSolids()
{
	return m_Solids.size();
}

Solid * CSGWorld::GetSolid(int i)
{
	if(i < 0) return 0;
	if(i >= m_Solids.size()) return 0;

	return m_Solids[i];
}

void CSGWorld::popSolid()
{	
	if (m_Solids.size() <= 0) return;

	delete m_Solids[m_Solids.size() - 1];

	m_Solids.pop_back();
	m_RenderSwitchboard.pop_back();
	m_nPaletteIndex = (m_nPaletteIndex - 1) % m_Palette.size();
}

void CSGWorld::Difference(int i, int j)
{
	Solid * pSolid1 = GetSolid(i);
	Solid * pSolid2 = GetSolid(j);

	if(pSolid1 == 0) return;
	if(pSolid2 == 0) return;

	BooleanModeller * pModeller = new BooleanModeller(pSolid1, pSolid2);

	Solid * pResult = pModeller->getDifference(true);

	delete pModeller;

	printf("Num vertices  %10d\n", pResult->vertices.GetSize());
	printf("Num indices   %10d\n", pResult->indices.GetSize());
	printf("Num colors    %10d\n", pResult->colors.GetSize());

	m_Solids.push_back(pResult);
	m_RenderSwitchboard.push_back(true);
	// Don't change palette index
	m_RenderSwitchboard[i] = false;
	m_RenderSwitchboard[j] = false;
}

void CSGWorld::Union(int i, int j)
{
	Solid * pSolid1 = GetSolid(i);
	Solid * pSolid2 = GetSolid(j);

	if(pSolid1 == 0) return;
	if(pSolid2 == 0) return;

	BooleanModeller * pModeller = new BooleanModeller(pSolid1, pSolid2);

	Solid * pResult = pModeller->getUnion();

	delete pModeller;

	printf("Num vertices  %10d\n", pResult->vertices.GetSize());
	printf("Num indices   %10d\n", pResult->indices.GetSize());
	printf("Num colors    %10d\n", pResult->colors.GetSize());

	m_Solids.push_back(pResult);
	m_RenderSwitchboard.push_back(true);
	// Don't change palette index
	m_RenderSwitchboard[i] = false;
	m_RenderSwitchboard[j] = false;
}

void CSGWorld::Intersection(int i, int j)
{
	Solid * pSolid1 = GetSolid(i);
	Solid * pSolid2 = GetSolid(j);

	if(pSolid1 == 0) return;
	if(pSolid2 == 0) return;

	BooleanModeller * pModeller = new BooleanModeller(pSolid1, pSolid2);

	Solid * pResult = pModeller->getIntersection();

	delete pModeller;

	printf("Num vertices  %10d\n", pResult->vertices.GetSize());
	printf("Num indices   %10d\n", pResult->indices.GetSize());
	printf("Num colors    %10d\n", pResult->colors.GetSize());

	m_Solids.push_back(pResult);
	m_RenderSwitchboard.push_back(true);
	// Don't change palette index
	m_RenderSwitchboard[i] = false;
	m_RenderSwitchboard[j] = false;
}

void CSGWorld::init()
{

}

void CSGWorld::Render(int nNum)
{
	int nLoopNum = nNum <= m_Solids.size() ? nNum : m_Solids.size();
	for (int i = 0; i < nLoopNum; i++)
	{
		Solid * pSolid = m_Solids[i];

		if(m_RenderSwitchboard[i])
		{
			pSolid->Render();
		}		
	}
}

void CSGWorld::rotate(int nNum, Vector& p1, Vector& p2, float angle)
{
    int nLoopNum = nNum <= m_Solids.size() ? nNum : m_Solids.size();
    for (int i = 0; i < nLoopNum; i++)
    {
        Solid * pSolid = m_Solids[i];

        pSolid->Rotate(p1, p2, angle);
    }
}

bool CSGWorld::isBindTexture()
{
	bool bBindTexture = true;

	for (int i = 0; i < m_Solids.size(); i++)
	{
		if (m_Solids[i]->isEnableTexture())
		{
			if (!m_Solids[i]->isBindTexture())
			{
				bBindTexture = false;
				break;
			}
		}
	}

	return bBindTexture;
}

void CSGWorld::bindTexture(QImage& img, bool invert)
{
	for (int i = 0; i < m_Solids.size(); i++)
	{
		if (m_Solids[i]->isEnableTexture())
		{
			m_Solids[i]->bindTexture(img, invert);
		}
	}
}

void CSGWorld::clearTexture()
{
	for (int i = 0; i < m_Solids.size(); i++)
	{
		if (m_Solids[i]->isEnableTexture())
		{
			m_Solids[i]->clearTexture();
		}		
	}
}

//void CSGWorld::Build_Ex1()
//{
//	pSolid1 = new Solid("box.txt", Color(0,0,255), 0,0,0);// 0, 150, -120);
//	pSolid2 = new Solid("cone.txt", Color(255,0,0), 0,0,0);// -120, 150, 0);
//
//	pSolid1->Translate(Vector(0.2, -0.2, 0));
//	pSolid2->Scale(Vector(0.4f, 0.3f, 2.8f));
//
//	pModeller = new BooleanModeller(pSolid2, pSolid1);
//
//	pUnionResult = pModeller->getUnion();
//	pIntersectionResult = pModeller->getIntersection();
//	pDifferenceResult = pModeller->getDifference();
//
//	printf("              %10s %10s %10s\n", "Union", "Intersection", "Difference");
//	printf("Num vertices  %10d %10d %10d\n", pUnionResult->vertices.GetSize(), pIntersectionResult->vertices.GetSize(), pDifferenceResult->vertices.GetSize());
//	printf("Num indices   %10d %10d %10d\n", pUnionResult->indices.GetSize(), pIntersectionResult->indices.GetSize(), pDifferenceResult->indices.GetSize());
//	printf("Num colors    %10d %10d %10d\n", pUnionResult->colors.GetSize(), pIntersectionResult->colors.GetSize(), pDifferenceResult->colors.GetSize());
//}
//
//void CSGWorld::Build_Ex2()
//{
//	pSolid1 = new Solid("box.txt", Color(0,0,255), 0,0,0);// 0, 150, -120);
//	pSolid2 = new Solid("cone.txt", Color(255,0,0), 0,0,0);// -120, 150, 0);
//
//	pSolid1->Translate(Vector(0.2, -0.2, 0));
//	pSolid2->Scale(Vector(0.4f, 1.3f, 1.8f));
//
//	pMidModeller = new BooleanModeller(pSolid2, pSolid1);
//
//	Solid * pMidResult = pMidModeller->getDifference();
//
//	pSphere = new Solid("sphere.txt", Color(0,255,0), 0,0,0);//150, -120, 0);
//
//	pSphere->Scale(Vector(1.0f, 1.0f, 0.6f));
//	pSphere->Translate(Vector(0.5, 0, 0));
//
//	pModeller = new BooleanModeller(pMidResult, pSphere);
//
//	pUnionResult = pModeller->getUnion();
//	pIntersectionResult = pModeller->getIntersection();
//	pDifferenceResult = pModeller->getDifference();
//
//	printf("              %10s %10s %10s\n", "Union", "Intersection", "Difference");
//	printf("Num vertices  %10d %10d %10d\n", pUnionResult->vertices.GetSize(), pIntersectionResult->vertices.GetSize(), pDifferenceResult->vertices.GetSize());
//	printf("Num indices   %10d %10d %10d\n", pUnionResult->indices.GetSize(), pIntersectionResult->indices.GetSize(), pDifferenceResult->indices.GetSize());
//	printf("Num colors    %10d %10d %10d\n", pUnionResult->colors.GetSize(), pIntersectionResult->colors.GetSize(), pDifferenceResult->colors.GetSize());
//}
//
//void CSGWorld::Render()
//{
//	//pUnionResult->Render();
//	//pDifferenceResult->Render();
//	pIntersectionResult->Render();
//
//	extern QtAdaptor * g_pImagineQtAdaptor;
//	g_pImagineQtAdaptor->RenderText(0,0,0,"hello");
//}
//
