#include <QMath.h>

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

#include "QDetectObj.h"

QBoardObj::QBoardObj(int nID, QString name, QObject *parent)
	: m_objName(name), QObject(parent)
{
	m_nID = nID;

	m_bBoardAlignCreated = false;
}

QBoardObj::~QBoardObj()
{

}

QBoardObj::QBoardObj(const QBoardObj &other)//复制构造函数 
{
	this->m_nID = other.m_nID;
	this->m_objName = other.m_objName;

	this->m_bBoardAlignCreated = other.m_bBoardAlignCreated;
	this->m_boardAlignRects.assign(other.m_boardAlignRects.begin(), other.m_boardAlignRects.end());
	this->m_boardAlignRecords.assign(other.m_boardAlignRecords.begin(), other.m_boardAlignRecords.end());
}

QBoardObj&  QBoardObj::operator = (const QBoardObj &other)//赋值符 
{
	if (this == &other) //如果对象和other是用一个对象，直接返回本身  
	{
		return *this;
	}

	this->m_nID = other.m_nID;
	this->m_objName = other.m_objName;

	this->m_bBoardAlignCreated = other.m_bBoardAlignCreated;
	this->m_boardAlignRects.assign(other.m_boardAlignRects.begin(), other.m_boardAlignRects.end());
	this->m_boardAlignRecords.assign(other.m_boardAlignRecords.begin(), other.m_boardAlignRecords.end());

	return *this;
}

bool QBoardObj::isBoardCreated()
{
	return m_bBoardAlignCreated;
}

void QBoardObj::addBoardAlign(cv::RotatedRect& rect, int nRecordID)
{
	m_boardAlignRects.push_back(rect);
	m_boardAlignRecords.push_back(nRecordID);
	m_bBoardAlignCreated = true;
}

int QBoardObj::getBoardAlignNum()
{
	return m_boardAlignRects.size();
}

void QBoardObj::removeBoardAlign(int index)
{
	if (index < 0 || index >= m_boardAlignRects.size()) return;

	m_boardAlignRects.erase(m_boardAlignRects.begin() + index);
	m_boardAlignRecords.erase(m_boardAlignRecords.begin() + index);
}

cv::RotatedRect QBoardObj::getBoardAlign(int index)
{
	if (index < 0 || index >= m_boardAlignRects.size()) return RotatedRect();

	cv::RotatedRect rect = m_boardAlignRects[index];

	return rect;
}

int QBoardObj::getRecordID(int index)
{
	if (index < 0 || index >= m_boardAlignRecords.size()) return 0;

	return m_boardAlignRecords[index];
}

void QBoardObj::clearBoardAlignments()
{
	m_bBoardAlignCreated = false;
	m_boardAlignRects.clear();
	m_boardAlignRecords.clear();
}

QDetectObj::QDetectObj(int nID, QString name)
	: m_nID(nID), m_objName(name)
{
	//m_objPosX = 0;
	//m_objPosY = 0;

	m_bframeCreated = false;
	m_bLocCreated = false;
	m_bhgtDetectCreated = false;
	m_bhgtBaseCreated = false;

	m_locRecordID = 0;

	m_bTested = false;

	m_objBitmap = "";
	m_objType = "0";
}

QDetectObj::~QDetectObj()
{
}

QDetectObj::QDetectObj(const QDetectObj &other)//复制构造函数 
{
	this->m_objName = other.m_objName;

	//this->m_objPosX = other.m_objPosX;
	//this->m_objPosY = other.m_objPosY;

	this->m_bframeCreated = other.m_bframeCreated;
	this->m_bLocCreated = other.m_bLocCreated;
	this->m_frameRect = other.m_frameRect;
	this->m_locationRect = other.m_locationRect;
	this->m_locRecordID = other.m_locRecordID;

	this->m_bhgtDetectCreated = other.m_bhgtDetectCreated;
	this->m_bhgtBaseCreated = other.m_bhgtBaseCreated;
	this->m_heightDetectRect.assign(other.m_heightDetectRect.begin(), other.m_heightDetectRect.end());
	this->m_heightDetectBaseRect.assign(other.m_heightDetectBaseRect.begin(), other.m_heightDetectBaseRect.end());	
	this->m_detAndBaseRectRel = other.m_detAndBaseRectRel;

	this->m_heightDetectValues.assign(other.m_heightDetectValues.begin(), other.m_heightDetectValues.end());
}

QDetectObj&  QDetectObj::operator = (const QDetectObj &other)//赋值符 
{
	if (this == &other) //如果对象和other是用一个对象，直接返回本身  
	{
		return *this;
	}

	this->m_objName = other.m_objName;	

	//this->m_objPosX = other.m_objPosX;
	//this->m_objPosY = other.m_objPosY;

	this->m_bframeCreated = other.m_bframeCreated;
	this->m_bLocCreated = other.m_bLocCreated;
	this->m_frameRect = other.m_frameRect;
	this->m_locationRect = other.m_locationRect;
	this->m_locRecordID = other.m_locRecordID;

	this->m_bhgtDetectCreated = other.m_bhgtDetectCreated;
	this->m_bhgtBaseCreated = other.m_bhgtBaseCreated;
	this->m_heightDetectRect.assign(other.m_heightDetectRect.begin(), other.m_heightDetectRect.end());
	this->m_heightDetectBaseRect.assign(other.m_heightDetectBaseRect.begin(), other.m_heightDetectBaseRect.end());	
	this->m_detAndBaseRectRel = other.m_detAndBaseRectRel;

	this->m_heightDetectValues.assign(other.m_heightDetectValues.begin(), other.m_heightDetectValues.end());

	return *this;
}

void QDetectObj::setObjPos(double posX, double posY)
{
	m_frameRect.center.x = posX - m_locationRect.center.x;
	m_frameRect.center.y = posY - m_locationRect.center.y;
}

double QDetectObj::getX()
{
	return m_frameRect.center.x;
}

double QDetectObj::getY()
{
	return m_frameRect.center.y;
}

double QDetectObj::getAngle()
{
	return m_frameRect.angle;
}

double QDetectObj::getWidth()
{
	return m_frameRect.size.width;
}

double QDetectObj::getHeight()
{
	return m_frameRect.size.height;
}

bool QDetectObj::isFrameCreated()
{
	return m_bframeCreated;
}

void QDetectObj::setFrame(cv::RotatedRect& rect)
{ 
	if (m_bframeCreated)
	{
		changeCoordinateOfObjParts(rect);
	}
	m_frameRect = rect; 
	m_bframeCreated = true;
}

cv::RotatedRect QDetectObj::getFrame() const
{ 
	return m_frameRect;
}

void QDetectObj::changeCoordinateOfObjParts(cv::RotatedRect& rect)
{	
	double q1 = m_frameRect.angle * M_PI / 180.0;
	double q3 = rect.angle * M_PI / 180.0;
	double dAngle = q3 - q1;

	calcCoordinateOfObjPart(m_locationRect, dAngle);

	for (int i = 0; i < m_heightDetectRect.size(); i++)
	{
		calcCoordinateOfObjPart(m_heightDetectRect[i], dAngle);
	}

	for (int i = 0; i < m_heightDetectBaseRect.size(); i++)
	{
		calcCoordinateOfObjPart(m_heightDetectBaseRect[i], dAngle);
	}

	//double q1 = m_frameRect.angle * M_PI / 180.0;
	//double q3 = rect.angle * M_PI / 180.0;
	//double q2 = qAtan2(m_locationRect.center.y, m_locationRect.center.x);
	//double dDistLoc = qSqrt(qPow(m_locationRect.center.x, 2) + qPow(m_locationRect.center.y, 2));
	//double angle = q3 - (q1 - q2);
	//m_locationRect.center.x = dDistLoc*qCos(angle);
	//m_locationRect.center.y = dDistLoc*qSin(angle);
}

void QDetectObj::calcCoordinateOfObjPart(cv::RotatedRect& rect, double dAngle)
{	
	double dDistLoc = qSqrt(qPow(rect.center.x, 2) + qPow(rect.center.y, 2));
	double q2 = qAtan2(rect.center.y, rect.center.x);
	double angle = dAngle + q2;
	rect.center.x = dDistLoc*qCos(angle);
	rect.center.y = dDistLoc*qSin(angle);
}

bool QDetectObj::isLocCreated()
{ 
	return m_bLocCreated; 
}

void QDetectObj::setLoc(cv::RotatedRect& rect)
{ 
	rect.center -= m_frameRect.center;
	rect.angle -= m_frameRect.angle;

	m_locationRect = rect;
	m_bLocCreated = true;
}

cv::RotatedRect QDetectObj::getLoc() const
{ 
	cv::RotatedRect rect = m_locationRect;

	rect.center += m_frameRect.center;
	rect.angle += m_frameRect.angle;

	return rect;
}

void QDetectObj::clearLoc()
{
	m_locationRect.center.x = 0;
	m_locationRect.center.y = 0;
	m_locationRect.size.width = 0;
	m_locationRect.size.height = 0;
	m_bLocCreated = false;
	m_locRecordID = 0;
}

void QDetectObj::setRecordID(int nRecordID)
{
	m_locRecordID = nRecordID;
}

int QDetectObj::getRecordID()
{
	return m_locRecordID;
}

bool QDetectObj::isHgtDetectCreated()
{
	return m_bhgtDetectCreated;
}

void QDetectObj::addHeightDetect(cv::RotatedRect& rect)
{
	rect.center -= m_frameRect.center;
	rect.angle -= m_frameRect.angle;

	m_heightDetectRect.push_back(rect);
	m_heightDetectValues.push_back(0);
	m_bhgtDetectCreated = true;
}

int QDetectObj::getHeightDetectNum() const
{
	return m_heightDetectRect.size();
}

void QDetectObj::setHeightDetect(int index, cv::RotatedRect& rect)
{
	if (index < 0 || index >= m_heightDetectRect.size()) return;

	rect.center -= m_frameRect.center;
	rect.angle -= m_frameRect.angle;

	m_heightDetectRect[index] = rect;
}

void QDetectObj::removeHeightDetect(int index)
{
	if (index < 0 || index >= m_heightDetectRect.size()) return;
	m_heightDetectRect.erase(m_heightDetectRect.begin() + index);
	m_heightDetectValues.erase(m_heightDetectValues.begin() + index);
}

cv::RotatedRect QDetectObj::getHeightDetect(int index) const
{
	if (index < 0 || index >= m_heightDetectRect.size()) return RotatedRect();

	cv::RotatedRect rect = m_heightDetectRect[index];

	rect.center += m_frameRect.center;
	rect.angle += m_frameRect.angle;

	return rect;	
}

void QDetectObj::clearHeightDetect()
{
	m_heightDetectRect.clear();
	m_heightDetectValues.clear();
	m_bhgtDetectCreated = false;
	m_bTested = false;
}

void QDetectObj::setHeightValue(int index, double dValue)
{
	if (index < 0 || index >= m_heightDetectValues.size()) return;
	m_heightDetectValues[index] = dValue;

	m_bTested = true;
}

double QDetectObj::getHeightValue(int index)
{
	if (index < 0 || index >= m_heightDetectValues.size()) return 0;

	return m_heightDetectValues[index];
}

bool QDetectObj::isTested()
{
	return m_bTested;
}

void QDetectObj::getHeightDetectBase(int index, QVector<cv::Rect>& rectBases)
{
	QList<int> baseIndexs = getDBRelationBase(index);
	for (int i = 0; i < baseIndexs.size(); i++)
	{
		int nBaseIndex = baseIndexs[i];

		RotatedRect rtRect = m_heightDetectBaseRect.at(nBaseIndex);
		rtRect.center += m_frameRect.center;
		rtRect.angle += m_frameRect.angle;

		rectBases.push_back(rtRect.boundingRect());
	}
}

bool QDetectObj::isHgtBaseCreated()
{
	return m_bhgtBaseCreated;
}

void QDetectObj::addHeightBase(cv::RotatedRect& rect)
{
	rect.center -= m_frameRect.center;
	rect.angle -= m_frameRect.angle;

	m_heightDetectBaseRect.push_back(rect);
	m_bhgtBaseCreated = true;
}

int QDetectObj::getHeightBaseNum() const
{
	return m_heightDetectBaseRect.size();
}

void QDetectObj::setHeightBase(int index, cv::RotatedRect& rect)
{
	if (index < 0 || index >= m_heightDetectBaseRect.size()) return;

	rect.center -= m_frameRect.center;
	rect.angle -= m_frameRect.angle;

	m_heightDetectBaseRect[index] = rect;
}

void QDetectObj::removeHeightBase(int index)
{
	if (index < 0 || index >= m_heightDetectBaseRect.size()) return;
	m_heightDetectBaseRect.erase(m_heightDetectBaseRect.begin() + index);
}

cv::RotatedRect QDetectObj::getHeightBase(int index) const
{
	if (index < 0 || index >= m_heightDetectBaseRect.size()) return RotatedRect();

	cv::RotatedRect rect = m_heightDetectBaseRect[index];

	rect.center += m_frameRect.center;
	rect.angle += m_frameRect.angle;

	return rect;
}

void QDetectObj::clearHeightBase()
{
	m_heightDetectBaseRect.clear();
	m_bhgtBaseCreated = false;
}

int QDetectObj::getDBRelationNum()
{
	return m_detAndBaseRectRel.count();
}

QList<int> QDetectObj::getDBRelationBase(int detectIndex)
{
	return m_detAndBaseRectRel.values(detectIndex);
}

QList<int> QDetectObj::getDBRelationDetect()
{
	return m_detAndBaseRectRel.uniqueKeys();
}

int QDetectObj::getDBRelationDetect(int baseIndex)
{
	return m_detAndBaseRectRel.key(baseIndex);
}

void QDetectObj::addDBRelation(int detectIndex, int baseIndex)
{
	QList<int> baseValues = m_detAndBaseRectRel.values(detectIndex);
	if (baseValues.contains(baseIndex))
	{
		return;
	}
	m_detAndBaseRectRel.insert(detectIndex, baseIndex);
}

void QDetectObj::removeDBRelation(int detectIndex, int baseIndex)
{
	m_detAndBaseRectRel.remove(detectIndex, baseIndex);
}

void QDetectObj::removeDBRelation(int detectIndex)
{
	m_detAndBaseRectRel.remove(detectIndex);
}

void QDetectObj::clearDBRelation()
{
	m_detAndBaseRectRel.clear();
}