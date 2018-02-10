#pragma once

#include <QObject>
#include <qmutex.h>
#include <QVector>

#include "../include/IData.h"

class QBoardObj;
class QDetectObj;
class QProfileObj;
class DataCtrl : public QObject
{
	Q_OBJECT

public:
	DataCtrl(QObject *parent = NULL);
	~DataCtrl();

public:
	void setInfiniteCycles(bool bInfinite);
	void incrementCycleTests();
	void decrementCycleTests();
	int	getCycleTests();

	QBoardObj* getBoardObj();
	int getObjNum(DataTypeEnum emDataType);
	QDetectObj* getObj(int nIndex, DataTypeEnum emDataType);
	void pushObj(QDetectObj* pObj, DataTypeEnum emDataType);
	void deleteObj(int nIndex, DataTypeEnum emDataType);
	void clearObjs(DataTypeEnum emDataType);

    bool createProject(QString& szFilePath);
    bool openProject(QString& szFilePath);

	bool saveDataBase(QString& szFilePath, DataTypeEnum emDataType);
	bool loadDataBase(QString& szFilePath, DataTypeEnum emDataType);
	void clearDataBase();

	unsigned int getCoreData(int nIndex);

	int getProfObjNum();
	int increaseProfObjIndex();
	int getProfObjIndex();
	QProfileObj* getProfObj(int nIndex);
	void pushProfObj(QProfileObj* pObj);
	void deleteProfObj(int nIndex);
	void clearProfObjs();

	bool saveProfDataBase(QString& szFilePath);
	bool loadProfDataBase(QString& szFilePath);
	void clearProfDataBase();
    bool doAlignment(const Vision::VectorOfMat &vecFrameImages);
private:
	void clearFiles(const QString &folderFullPath);

private:
	QVector<QDetectObj*>& getCellType(DataTypeEnum emDataType);

private:
	QMutex m_mutex;
	int m_nCycleTestNum;
	bool m_bInfiniteCycles;

	QBoardObj* m_boardObj;
	QVector<QDetectObj*> m_cellTmpObjs;
	QVector<QDetectObj*> m_cellTestObjs;

	QVector<QProfileObj*> m_profileObjs;
	int m_nProfileIndex;
};
