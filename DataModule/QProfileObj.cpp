#include "QProfileObj.h"
#include <QMath.h>

#include "../lib/VisionLibrary/include/VisionAPI.h"
using namespace AOI;

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

QProfileObj::QProfileObj(int nID, QString name, QObject *parent)
	: m_nID(nID), m_objName(name), QObject(parent)
{
	m_posX1 = 0;
	m_posY1 = 0;
	m_posX2 = 0;
	m_posY2 = 0;
}

QProfileObj::~QProfileObj()
{
	clearMeasObjs();
}

QProfileObj::QProfileObj(const QProfileObj &other)//复制构造函数 
{
	this->m_objName = other.m_objName;

	this->m_posX1 = other.m_posX1;
	this->m_posY1 = other.m_posY1;
	this->m_posX2 = other.m_posX2;
	this->m_posY2 = other.m_posY2;

	this->m_img3D = other.m_img3D;
	this->m_imgGray = other.m_imgGray;
	this->m_profDatas = other.m_profDatas;

	deepCopy(other);
}

QProfileObj&  QProfileObj::operator = (const QProfileObj &other)//赋值符 
{
	if (this == &other) //如果对象和other是用一个对象，直接返回本身  
	{
		return *this;
	}

	this->m_objName = other.m_objName;

	this->m_posX1 = other.m_posX1;
	this->m_posY1 = other.m_posY1;
	this->m_posX2 = other.m_posX2;
	this->m_posY2 = other.m_posY2;

	this->m_img3D = other.m_img3D;
	this->m_imgGray = other.m_imgGray;
	this->m_profDatas = other.m_profDatas;

	deepCopy(other);

	return *this;
}

void QProfileObj::deepCopy(const QProfileObj &other)
{
	clearMeasObjs();

	for (int i = 0; i < other.m_profMeasObjs.size(); i++)
	{
		QProfMeasureObj* pObj = other.m_profMeasObjs[i];

		QProfMeasureObj* pObjNew = NULL;
		switch (pObj->getType())
		{
		case MEASURE_TYPE_LNTOLNANGLE:
			pObjNew = new QProfMeasLnToLnAngle();
			break;
		case MEASURE_TYPE_LNTOLN_DIST:
			pObjNew = new QProfMeasLnToLnDist();
			break;
		case MEASURE_TYPE_PTTOLN_DIST:
			pObjNew = new QProfMeasPtToLnDist();
			break;
		case MEASURE_TYPE_PTTOPT_DIST:
			pObjNew = new QProfMeasPtToPtDist();
			break;
		case MEASURE_TYPE_PT_HEIGHT:
			pObjNew = new QProfMeasPtHeight();
			break;
		case MEASURE_TYPE_CRTOCR_DIST:
			pObjNew = new QProfMeasCirToCir();
			break;
		case MEASURE_TYPE_CRTOLN_DIST:
			pObjNew = new QProfMeasCirToLn();
			break;
		case MEASURE_TYPE_CRTOPT_DIST:
			pObjNew = new QProfMeasCirToPt();
			break;
		case MEASURE_TYPE_CR_RADIUS:
			pObjNew = new QProfMeasCirRadius();
			break;
		case MEASURE_TYPE_AREA:
			pObjNew = new QProfMeasProfArea();
			break;
		case MEASURE_TYPE_PRF_LENGTH:
			pObjNew = new QProfMeasProfLength();
			break;
		case MEASURE_TYPE_NULL:
		default:
			break;
		}

		*pObjNew = *pObj;

		this->addMeasObj(pObjNew);
	}
}

void QProfileObj::setCutingPosn(double posX1, double posY1, double posX2, double posY2)
{
	m_posX1 = posX1;
	m_posY1 = posY1;

	m_posX2 = posX2;
	m_posY2 = posY2;
}

void QProfileObj::getCutingPosn(double& posX1, double& posY1, double& posX2, double& posY2)
{
	posX1 = m_posX1;
	posY1 = m_posY1;
	posX2 = m_posX2;
	posY2 = m_posY2;
}

void QProfileObj::addMeasObj(QProfMeasureObj* pObj) 
{
	m_profMeasObjs.push_back(pObj); 
}

QProfMeasureObj* QProfileObj::getMeasObj(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_profMeasObjs.size())
	{
		return m_profMeasObjs[nIndex];
	}
    return NULL;
}

void QProfileObj::removeMeasObj(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_profMeasObjs.size())
	{
		delete m_profMeasObjs[nIndex];
		m_profMeasObjs.removeAt(nIndex);
	}
}

int QProfileObj::getMeasObjNum()
{
	return m_profMeasObjs.size();
}

void QProfileObj::clearMeasObjs()
{
	for (int i = 0; i < m_profMeasObjs.size(); i++)
	{
		delete m_profMeasObjs[i];
	}
	m_profMeasObjs.clear();
}

void QProfileObj::setOffset(cv::Point2d ptOffset)
{
	m_posX1 += ptOffset.x;
	m_posX2 += ptOffset.x;

	m_posY1 += ptOffset.y;
	m_posY2 += ptOffset.y;
}
