#pragma once

#include <QObject>

#include <QtOpenGL/QGLWidget>
#include <QtGui/QMouseEvent>
#include <vector>

#include <Core/Solid.h>

#include <QVector>

class Vector3D;
class Position3D;
class Matrix4D;

class CSGMesh : public QObject
{
	Q_OBJECT

public:
	CSGMesh(QObject *parent = NULL);
	~CSGMesh();

public:
	void loadData(const QString& sFileNameX, const QString& sFileNameY, const QString& sFileNameZ, const QString& szFileExt);
	void loadData(bool invert, int nSizeX, int nSizeY, QVector<Vector>& values);

	void convetToMeshTriangulationExt();	

	VectorSet* getVerticesMeshData();
	IntSet* getIndicesMeshData();
	bool isConvertToMesh();

    void rotate(Vector3D& rotateAxis, float angle);
    void transform(Matrix4D& m);

	void init();
	void Render();

	void getLimit(double& dSizeMaxX, double& dSizeMinX, double& dSizeMaxY, double& dSizeMinY, double& dSizeMaxZ, double& dSizeMinZ);

	void getSize(int& nSizeX, int& nSizeY);
	int  getSampling();

private:
	void readData(const QString& fileName, const QString& szFileExt, QVector<double>& fileValues, int& nSizeX, int& nSizeY);
	void splitCSVLine(const QString &lineStr, QVector<double>& fileValues);

private:
	//VectorSet m_vertices;
	int m_nSizeX;
	int m_nSizeY;
	int m_nSampling;
	double m_dSizeMaxX;
	double m_dSizeMinX;
	double m_dSizeMaxY;
	double m_dSizeMinY;
	double m_dSizeMaxZ;
	double m_dSizeMinZ;

	bool m_bConvetToMesh;
	VectorSet* m_verticesMesh;
	IntSet* m_indicesMesh;

	bool m_bMeshTriWriteToFile;

	bool m_bChangeToCenter;
	bool m_bXYCoordinateInvert;
};
