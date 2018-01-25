#pragma once
#include <QObject>
#include <QMultiMap>
#include <QVector>

#include "opencv/cv.h"

class QBoardObj : public QObject
{
	Q_OBJECT
public:
	QBoardObj(int nID, QString name, QObject *parent = NULL);
	~QBoardObj();


	QBoardObj(const QBoardObj &other);//复制构造函数 
	QBoardObj & operator = (const QBoardObj &other);//赋值符 

public:
	QString getName(){ return m_objName; }
	void setName(QString& name){ m_objName = name; }
	int		getID(){ return m_nID; }
	void	setID(int nID){ m_nID = nID; }

	bool isBoardCreated();
	void addBoardAlign(cv::RotatedRect& rect, int nRecordID);
	int getBoardAlignNum();	
	void removeBoardAlign(int index);
	cv::RotatedRect getBoardAlign(int index);
	int getRecordID(int index);
	void clearBoardAlignments();

private:
	int m_nID;
	QString m_objName;

	bool m_bBoardAlignCreated;
	std::vector<cv::RotatedRect> m_boardAlignRects;
	std::vector<int> m_boardAlignRecords;
};

class QDetectObj : public QObject
{
	Q_OBJECT

public:
	QDetectObj(int nID, QString name, QObject *parent = NULL);
	~QDetectObj();

	QDetectObj(const QDetectObj &other);//复制构造函数 
	QDetectObj & operator = (const QDetectObj &other);//赋值符 

public:
	QString getName(){ return m_objName; }
	void setName(QString& name){ m_objName = name; }
	int		getID(){ return m_nID; }

	void setType(QString& szType){ m_objType = szType; }
	QString getType(){ return m_objType; }

	void setBitmap(QString& szFilePath){m_objBitmap = szFilePath;}
	QString getBitmap(){ return m_objBitmap; }

	void setObjPos(double posX, double posY);
	double getX();
	double getY();
	double getAngle();
	double getWidth();
	double getHeight();

	bool isFrameCreated();
	void setFrame(cv::RotatedRect& rect);
	cv::RotatedRect getFrame();

	bool isLocCreated();
	void setLoc(cv::RotatedRect& rect);
	int getLocID();
	cv::RotatedRect getLoc();
	void clearLoc();
	void setRecordID(int nRecordID);
	int getRecordID();

	bool isHgtDetectCreated();
	void addHeightDetect(cv::RotatedRect& rect);
	int getHeightDetectNum();
	void setHeightDetect(int index, cv::RotatedRect& rect);
	void removeHeightDetect(int index);
	cv::RotatedRect getHeightDetect(int index);
	void setHeightValue(int index, double dValue);
	double getHeightValue(int index);
	bool isTested();
	void clearHeightDetect();
	void getHeightDetectBase(int index, QVector<cv::Rect>& rectBases);

	bool isHgtBaseCreated();
	void addHeightBase(cv::RotatedRect& rect);
	int getHeightBaseNum();
	void setHeightBase(int index, cv::RotatedRect& rect);
	void removeHeightBase(int index);
	cv::RotatedRect getHeightBase(int index);
	void clearHeightBase();

	int getDBRelationNum();
	QList<int> getDBRelationBase(int detectIndex);
	QList<int> getDBRelationDetect();
	int getDBRelationDetect(int baseIndex);
	void addDBRelation(int detectIndex, int baseIndex);
	void removeDBRelation(int detectIndex, int baseIndex);
	void removeDBRelation(int detectIndex);
	void clearDBRelation();

private:
	void changeCoordinateOfObjParts(cv::RotatedRect& rect);
	void calcCoordinateOfObjPart(cv::RotatedRect& rect, double dAngle);

private:
	int m_nID;
	QString m_objName;
	QString m_objBitmap;
	QString m_objType;

	//double m_objPosX;
	//double m_objPosY;

	bool m_bframeCreated;	
	cv::RotatedRect m_frameRect;

	bool m_bLocCreated;
	cv::RotatedRect m_locationRect;
	int m_locRecordID;

	bool m_bhgtDetectCreated;
	std::vector<cv::RotatedRect> m_heightDetectRect;

	bool m_bhgtBaseCreated;
	std::vector<cv::RotatedRect> m_heightDetectBaseRect;

	QMultiMap<int, int> m_detAndBaseRectRel;

	//detect height value
	std::vector<float> m_heightDetectValues;
	bool m_bTested;
};
