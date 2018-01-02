
// Author: Greg Santucci, 2008 and 2009
// This is part of a port of the CSG project
// originally written in java by Danilo Balby
// Email: thecodewitch@gmail.com
// Web: http://createuniverses.blogspot.com/

#include "VectorSet.h"

#include "Vector.h"

VectorSet::VectorSet()
{
	//m_nMaxSize = 10000;
	//m_nSize = 0;
	//m_pVectors = new Vector[m_nMaxSize];

	//length = m_nSize;
}

VectorSet::VectorSet(int nMaxSize)
{
	//m_nMaxSize = nMaxSize;
	//m_nSize = 0;
	//m_pVectors = new Vector[m_nMaxSize];

	//length = m_nSize;
}

VectorSet::~VectorSet()
{
	//delete [] m_pVectors;
}

//int VectorSet::GetMaxSize()
//{
//	return m_nMaxSize;
//}

int VectorSet::GetSize()
{
	//return m_nSize;
	return m_pVectors.size();
}

int VectorSet::length()
{
	//return m_nSize;
	return m_pVectors.size();
}

Vector VectorSet::GetVector(int i)
{
	if(i < 0) return Vector();
	if(i >= m_pVectors.size()) return Vector();

	return m_pVectors[i];
}

void VectorSet::SetVector(int i, const Vector & vVector)
{
	if(i < 0) return;
	if(i >= m_pVectors.size()) return;

	m_pVectors[i] = vVector;
}

void VectorSet::AddVector(const Vector & vVector)
{
	m_pVectors.push_back(vVector);

	//if(m_nSize >= m_nMaxSize) return;

	//m_pVectors[m_nSize] = vVector;
	//m_nSize++;

	//length = m_nSize;
}

void VectorSet::clear()
{
	m_pVectors.clear();
}

void VectorSet::RemoveVector(int i)
{
	m_pVectors.erase(m_pVectors.begin()+i);

	//// Shunt everything down over it
	//if(m_nSize <= 0)
	//{
	//	return;
	//}

	//for(int j = i; j < m_nSize-1; j++)
	//{
	//	m_pVectors[j] = m_pVectors[j+1];
	//}

	//m_nSize--;
	//length = m_nSize;
}

Vector & VectorSet::operator[](int index)
{
	Vector & pVector = m_pVectors[index];

	// If its null, we're in trouble...

	return pVector;
}
