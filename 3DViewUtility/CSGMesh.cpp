#include "CSGMesh.h"

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QApplication>

#include <QDebug>
#include <QDateTime>

#include "../Common/SystemData.h"

#include"MathAPIKernel/Vector3D.h"
#include"MathAPIKernel/Position3D.h"
#include"MathAPIKernel/Matrix4D.h"

CSGMesh::CSGMesh(QObject *parent)
	: QObject(parent)
{
	m_bMeshTriWriteToFile = false;
	m_bConvetToMesh = false;

	m_bChangeToCenter = false;

	m_verticesMesh = new VectorSet();
	m_indicesMesh = new IntSet();

	m_nSizeX = 0;
	m_nSizeY = 0;
	m_nSampling = 1;

	m_dSizeMaxX = 0;
	m_dSizeMinX = 0;
	m_dSizeMaxY = 0;
	m_dSizeMinY = 0;
	m_dSizeMaxZ = 0;
	m_dSizeMinZ = 0;

	m_bXYCoordinateInvert = false; 
}

CSGMesh::~CSGMesh()
{
	delete m_verticesMesh;
	delete m_indicesMesh;   
}

void CSGMesh::loadData(const QString& sFileNameX, const QString& sFileNameY, const QString& sFileNameZ, const QString& szFileExt)
{
	QVector<double> fileXValues, fileYValues, fileZValues;
	int nSizeX = 0, nSizeY = 0;
	readData(sFileNameX, szFileExt, fileXValues, nSizeX, nSizeY);
	readData(sFileNameY, szFileExt, fileYValues, nSizeX, nSizeY);
	readData(sFileNameZ, szFileExt, fileZValues, nSizeX, nSizeY);

	if (fileXValues.size() == fileYValues.size() && fileYValues.size() == fileZValues.size())
	{
		//m_vertices.clear();
		m_verticesMesh->clear();
		m_indicesMesh->clear();	

		// Auto Resize Data Number
		int nRate = 1;
		int nSampFreqIndex = System->getParam("3D_view_param_samp_freq").toInt();
		//if nSizeXRem > 0 && nSizeYRem > 0, image display will not correct.
		while (nSampFreqIndex-- > 0)
		{
			nRate *= 2;
			while (nRate < 100 && (nSizeX%nRate == 0 || nSizeY%nRate == 0))
			{
				nRate += 1;
			}
		}

		if (nSizeX%nRate == 0 || nSizeY%nRate == 0)
		{
			nRate = 1;
		}
		qDebug() << "Auto Sampling Freq: " << nRate;

		//int nSize = nSizeX;
		//while (nSize > 500/*500*/)
		//{		
		//	nRate *= 3;
		//	nSize = nSize / (nRate);
		//}

		int nSizeXDiv = nSizeX / nRate;
		int nSizeXRem = nSizeX%nRate;
		int nSizeYDiv = nSizeY / nRate;
		int nSizeYRem = nSizeY%nRate;			

		double xMax = -1000000, xMin = 1000000, yMax = -1000000, yMin = 1000000, zMax = -1000000, zMin = 1000000;
		for (int i = 0, j = 0; i < fileXValues.size(); i++, j++)
		{
			if (j >= nSizeX)
			{
				i += nSizeX * (nRate - 1);
				if (i >= fileXValues.size()) break;
				j = 0;
			}
			if (j%nRate != 0) continue;			

			double dValueX = fileXValues.at(i);
			double dValueY = fileYValues.at(i);
			double dValueZ = fileZValues.at(i);

			if (dValueX > xMax)
			{
				xMax = dValueX;
			}
			if (dValueX < xMin)
			{
				xMin = dValueX;
			}
			if (dValueY > yMax)
			{
				yMax = dValueY;
			}

			if (dValueY < yMin)
			{
				yMin = dValueY;
			}

			if (dValueZ > zMax)
			{
				zMax = dValueZ;
			}

			if (dValueZ < zMin)
			{
				zMin = dValueZ;
			}

			Vector vValue;
			vValue.Set(dValueX, dValueY, dValueZ);

			if (m_bChangeToCenter)
			{
				vValue.x -= nSizeX / 2;
				vValue.y -= nSizeY / 2;
			}

			//m_vertices.AddVector(vValue);
			m_verticesMesh->AddVector(vValue);
		}
		nSizeX = nSizeXDiv + (nRate > 1 ? 1 : 0);
		nSizeY = nSizeYDiv + (nRate > 1 ? 1 : 0);
		m_nSampling = nRate;

		//qDebug() << "num: " << m_verticesMesh->GetSize();
		m_dSizeMaxX = xMax;
		m_dSizeMinX = xMin;
		m_dSizeMaxY = yMax;
		m_dSizeMinY = yMin;
		m_dSizeMaxZ = zMax;
		m_dSizeMinZ = zMin;

		m_nSizeX = nSizeX;
		m_nSizeY = nSizeY;

		m_bConvetToMesh = false;
	}
	else
	{
		m_nSizeX = 0;
		m_nSizeY = 0;
	}
}

void CSGMesh::readData(const QString& fileName, const QString& szFileExt, QVector<double>& fileValues, int& nSizeX, int& nSizeY)
{
	if (fileName.isEmpty() /*|| !fileName.contains(".csv")*/)
		return;

	//QFileInfo firmwareFileInfo;
	//firmwareFileInfo.setFile(fileName);
	//m_firmwarePath = firmwareFileInfo.absolutePath();

	QFile iniFile(fileName);
	if (!iniFile.open(QIODevice::ReadWrite | QIODevice::Text))
	{
		return;
	}	

	if (0 == szFileExt.compare("csv", Qt::CaseInsensitive))
	{
		QTextStream in(&iniFile);
		QString lineAll;

		//QDateTime timeRead = QDateTime::currentDateTime();

		lineAll = in.readAll();
		//qDebug() << QString("load data time : %1").arg(timeRead.msecsTo(QDateTime::currentDateTime()));
		QVector<QStringRef> lineAllValues = lineAll.splitRef(',');
		//qDebug() << QString("split data time : %1").arg(timeRead.msecsTo(QDateTime::currentDateTime()));
		for (int i = 0; i < lineAllValues.size(); i++)
		{
			double dValue = lineAllValues[i].toDouble();
			fileValues.push_back(dValue);
		}
		//splitCSVLine(line, fileValues);
		//qDebug() << QString("convert data time : %1").arg(timeRead.msecsTo(QDateTime::currentDateTime()));

		in.seek(0);
		QString line = in.readLine();
		QVector<QStringRef> lineValues = line.splitRef(',');
		nSizeX = lineValues.size() - 1;
		nSizeY = lineAllValues.size() / nSizeX;

		//qDebug() << QString("calculate data time : %1").arg(timeRead.msecsTo(QDateTime::currentDateTime()));

		////写二进制文件-----------------------
		//QString fileNameExt;
		//int nIndex = fileName.indexOf('.');
		//if (nIndex > 0 && nIndex < fileName.length())
		//{
		//	fileNameExt = fileName.left(nIndex) + ".raw";
		//}
		//QFile file(fileNameExt);
		//if (!file.open(QIODevice::WriteOnly))
		//{
		//	qDebug() << "Can't open file for writing";
		//	return;
		//}
		//QDataStream out(&file);
		//out.setVersion(QDataStream::Qt_5_4);
		//out << nSizeX << nSizeY;
		//for (int i = 0; i < fileValues.size(); i++)
		//{
		//	out << fileValues[i];
		//}
		////out << int(12) << QByteArray("123");
		//file.close();
		////-------------------------		
	}
	else if (0 == szFileExt.compare("raw", Qt::CaseInsensitive))
	{
		//QDateTime timeRead = QDateTime::currentDateTime();

		QDataStream in(&iniFile);
		in.setVersion(QDataStream::Qt_5_4);		

		in >> nSizeX >> nSizeY;

		//qDebug() << QString("load data time : %1").arg(timeRead.msecsTo(QDateTime::currentDateTime()));

		double dValue;
		while (!in.atEnd())
		{
			in >> dValue;
			fileValues.push_back(dValue);
		}

		//qDebug() << QString("convert data time : %1").arg(timeRead.msecsTo(QDateTime::currentDateTime()));

		//nSizeX = 1443;
		//nSizeY = 1466;

		//qDebug() << QString("calculate data time : %1").arg(timeRead.msecsTo(QDateTime::currentDateTime()));
	}	

	iniFile.close();

	//int nRowCount = 0;
	//while (!in.atEnd())
	//{
	//	line = in.readLine();
	//	
	//	QVector<QStringRef> lineValues = line.splitRef(',');
	//	for (int i = 0; i < lineValues.size(); i++)
	//	{
	//		double dValue = lineValues[i].toDouble();
	//		fileValues.push_back(dValue);
	//	}		

	//	if (nSizeX <= 0)	nSizeX = lineValues.size();

	//	nRowCount++;

	//	//if (nRowCount > 10) break;
	//}	
}

void CSGMesh::loadData(bool invert, int nSizeX, int nSizeY, QVector<Vector>& values)
{
	m_bXYCoordinateInvert = invert;
	if (values.size() > 0)
	{
		int nValueNum = values.size();

		//m_vertices.clear();
		m_verticesMesh->clear();
		m_indicesMesh->clear();

		// Auto Resize Data Number
		int nRate = 1;
		int nSampFreqIndex = System->getParam("3D_view_param_samp_freq").toInt();
		//if nSizeXRem > 0 && nSizeYRem > 0, image display will not correct.
		while (nSampFreqIndex -- > 0)
		{
			nRate *= 2;
			while (nRate < 100 && (nSizeX%nRate == 0 || nSizeY%nRate == 0))
			{
				nRate += 1;				
			}
		}	

		if (nSizeX%nRate == 0 || nSizeY%nRate == 0)
		{
			nRate = 1;
		}
		qDebug() << "Auto Sampling Freq: " << nRate;

		//int nSize = nSizeX;		
		//while (nSize > 500/*500*/)
		//{
		//	nRate *= 3;
		//	nSize = nSize / (nRate);
		//}

		int nSizeXDiv = nSizeX / nRate;
		int nSizeXRem = nSizeX%nRate;
		int nSizeYDiv = nSizeY / nRate;
		int nSizeYRem = nSizeY%nRate;

		double xMax = -1000000, xMin = 1000000, yMax = -1000000, yMin = 1000000, zMax = -1000000, zMin = 1000000;
		for (int i = 0, j = 0; i < nValueNum; i++, j++)
		{
			if (j >= nSizeX)
			{
				i += nSizeX * (nRate - 1);
				if (i >= nValueNum) break;
				j = 0;
			}
			if (j%nRate != 0) continue;

			double dValueX = values.at(i).x;
			double dValueY = values.at(i).y;
			double dValueZ = values.at(i).z;

			if (dValueX > xMax)
			{
				xMax = dValueX;
			}
			if (dValueX < xMin)
			{
				xMin = dValueX;
			}
			if (dValueY > yMax)
			{
				yMax = dValueY;
			}

			if (dValueY < yMin)
			{
				yMin = dValueY;
			}

			if (dValueZ > zMax)
			{
				zMax = dValueZ;
			}

			if (dValueZ < zMin)
			{
				zMin = dValueZ;
			}

			Vector vValue;
			vValue.Set(dValueX, dValueY, dValueZ);

			if (m_bChangeToCenter)
			{
				vValue.x -= nSizeX / 2;
				vValue.y -= nSizeY / 2;
			}

			//m_vertices.AddVector(vValue);
			m_verticesMesh->AddVector(vValue);
		}
		nSizeX = nSizeXDiv + (nRate > 1 ? 1 : 0);
		nSizeY = nSizeYDiv + (nRate > 1 ? 1 : 0);
		m_nSampling = nRate;

		//qDebug() << "num: " << m_verticesMesh->GetSize();
		m_dSizeMaxX = xMax;
		m_dSizeMinX = xMin;
		m_dSizeMaxY = yMax;
		m_dSizeMinY = yMin;
		m_dSizeMaxZ = zMax;
		m_dSizeMinZ = zMin;

		m_nSizeX = nSizeX;
		m_nSizeY = nSizeY;

		m_bConvetToMesh = false;
	}
	else
	{
		m_nSizeX = 0;
		m_nSizeY = 0;
	}
}

void CSGMesh::splitCSVLine(const QString &lineStr, QVector<double>& fileValues)
{	
	QString str;

	int length = lineStr.length();
	int quoteCount = 0;
	int repeatQuoteCount = 0;

	for (int i = 0; i < length; ++i)
	{
		if (lineStr[i] != '\"')
		{
			repeatQuoteCount = 0;
			if (lineStr[i] != ',')
			{
				str.append(lineStr[i]);
			}
			else
			{
				if (quoteCount % 2)
				{
					str.append(',');
				}
				else
				{
					fileValues.push_back(str.toDouble());
					quoteCount = 0;
					str.clear();
				}
			}
		}
		else
		{
			++quoteCount;
			++repeatQuoteCount;
			if (repeatQuoteCount == 4)
			{
				str.append('\"');
				repeatQuoteCount = 0;
				quoteCount -= 4;
			}
		}
	}
	fileValues.push_back(str.toDouble());	
}

void CSGMesh::convetToMeshTriangulationExt()
{
	if (m_verticesMesh->GetSize() <= 2 || m_nSizeX <= 1 || m_nSizeY <= 1)
	{
		m_verticesMesh->clear();
		m_indicesMesh->clear();
		m_bConvetToMesh = false;
		return;
	}

	//m_verticesMesh->clear();	
	//for (int i = 0; i < m_vertices.GetSize(); i++)
	//{	
	//	m_verticesMesh->AddVector(m_vertices[i]);
	//}
	
	m_indicesMesh->clear();
	int nNumTriangles = (m_nSizeX - 1) * (m_nSizeY - 1);
	for (int i = 0; i < nNumTriangles; i++)
	{
		int colIndex = i % (m_nSizeX - 1);
		int rowIndex = i / (m_nSizeX - 1);	

		int nTriIndex1 = m_nSizeX*rowIndex + colIndex;
		int nTriIndex2 = m_nSizeX*rowIndex + colIndex + 1;
		int nTriIndex3 = m_nSizeX*(rowIndex + 1) + colIndex;

		//if ((m_verticesMesh->GetVector(nTriIndex1).z != 0)
		//	&& (m_verticesMesh->GetVector(nTriIndex2).z != 0)
		//	&& (m_verticesMesh->GetVector(nTriIndex3).z != 0))
		{
			m_indicesMesh->AddInt(nTriIndex1);
			m_indicesMesh->AddInt(nTriIndex2);
			m_indicesMesh->AddInt(nTriIndex3);
		}		

		nTriIndex1 = m_nSizeX*rowIndex + colIndex + 1;
		nTriIndex2 = m_nSizeX*(rowIndex + 1) + colIndex;
		nTriIndex3 = m_nSizeX*(rowIndex + 1) + colIndex + 1;

		//if ((m_verticesMesh->GetVector(nTriIndex1).z != 0)
		//	&& (m_verticesMesh->GetVector(nTriIndex2).z != 0)
		//	&& (m_verticesMesh->GetVector(nTriIndex3).z != 0))
		{
			m_indicesMesh->AddInt(nTriIndex1);
			m_indicesMesh->AddInt(nTriIndex2);
			m_indicesMesh->AddInt(nTriIndex3);
		}			
	}

	m_bConvetToMesh = true;
}

VectorSet* CSGMesh::getVerticesMeshData()
{
	if (m_bConvetToMesh)
	{		
		return m_verticesMesh;	
	}
	else
	{
		return NULL;
	}
}

IntSet* CSGMesh::getIndicesMeshData()
{
	if (m_bConvetToMesh)
	{
		return m_indicesMesh;
	}
	else
	{
		return NULL;
	}
}

bool CSGMesh::isConvertToMesh()
{
	return m_bConvetToMesh;
}

void CSGMesh::init()
{
}

void CSGMesh::Render()
{
	glColor3f(1.0f, 1.0f, 0.0f);

	glPointSize(2.0f);

	glBegin(GL_POINTS);	

	for (int i = 0; i < m_verticesMesh->GetSize(); i++)
	{
		Vector vP1 = m_verticesMesh->GetVector(i);
		glVertex3d(vP1.x, vP1.y, vP1.z);
	}

	glEnd();

	glPointSize(1.0f);
}

void CSGMesh::rotate(Vector3D& rotateAxis, float angle)
{
    Matrix4D rotateMatrix = Matrix4D::getRotateMatrix(angle, rotateAxis);
    transform(rotateMatrix);
}

void CSGMesh::transform(Matrix4D& m)
{
    for (int i = 0; i < m_verticesMesh->GetSize(); i++)
    {
        Vector vP1 = m_verticesMesh->GetVector(i);

        Vector3D pt;
        pt.setX(vP1.x);
        pt.setY(vP1.y);
        pt.setZ(vP1.z);

        pt = m * pt;

        vP1.x = pt.X();
        vP1.y = pt.Y();
        vP1.z = pt.Z();
    
        m_verticesMesh->SetVector(i, vP1);
    }
}

void CSGMesh::getLimit(double& dSizeMaxX, double& dSizeMinX, double& dSizeMaxY, double& dSizeMinY, double& dSizeMaxZ, double& dSizeMinZ)
{
	dSizeMaxX = m_dSizeMaxX;
	dSizeMinX = m_dSizeMinX;
	dSizeMaxY = m_dSizeMaxY;
	dSizeMinY = m_dSizeMinY;
	dSizeMaxZ = m_dSizeMaxZ;
	dSizeMinZ = m_dSizeMinZ;
}

void CSGMesh::getSize(int& nSizeX, int& nSizeY)
{
	nSizeX = m_nSizeX;
	nSizeY = m_nSizeY;
}

int  CSGMesh::getSampling()
{
	return m_nSampling;
}