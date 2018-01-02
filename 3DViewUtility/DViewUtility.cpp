#include "DViewUtility.h"
#include "ViewEditor.h"
#include "./core/Vector.h"

DViewUtility::DViewUtility()
{
	m_pInstance = new ViewEditor();		
}

DViewUtility::~DViewUtility()
{
	if (m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}	
}

void DViewUtility::show()
{
	m_pInstance->show();
	m_pInstance->setShow(true);
}

void DViewUtility::hide()
{
	m_pInstance->close();	
	m_pInstance->setShow(false);
}

bool DViewUtility::isShown()
{
	return m_pInstance->isShown();
}

void DViewUtility::changeToMesh()
{
	m_pInstance->changeToMesh();
}

void DViewUtility::loadFile(bool invert, int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues)
{
	QVector<Vector> values;
	if (xValues.size() == yValues.size() && yValues.size() == zValues.size())
	{
		for (int i = 0; i < xValues.size(); i++)
		{
			Vector valueTmp;
			valueTmp.Set(xValues[i], yValues[i], zValues[i]);
			values.push_back(valueTmp);
		}
	}
	m_pInstance->loadFile(invert, nSizeX, nSizeY, values);
}

QGLWidget* DViewUtility::getQGLWidget()
{
	return m_pInstance->getQGLWidget();
}

void DViewUtility::previousROIDisplay()
{
	m_pInstance->previousROIDisplay();
}

void DViewUtility::prepareROIDisplay(QImage& texture, bool invert)
{
	m_pInstance->prepareROIDisplay(texture, invert);
}
