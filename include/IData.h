#ifndef _DATA_INTERFACE_H_
#define _DATA_INTERFACE_H_

#include <QString>
#include "VisionStruct.h"

using namespace AOI;

enum DataTypeEnum
{
	EM_DATA_TYPE_TMP,
	EM_DATA_TYPE_OBJ
};

class QBoardObj;
class QDetectObj;
class QProfileObj;
class IData
{
public:	
	virtual QWidget* getDataEditor() = 0;
    virtual QWidget* getDataWidget() = 0;
    virtual QWidget* getDataList() = 0;

	virtual void incrementCycleTests() = 0;
	virtual void decrementCycleTests() = 0;
	virtual int	getCycleTests() = 0;

	virtual QBoardObj* getBoardObj() = 0;
	virtual int getObjNum(DataTypeEnum emDataType = EM_DATA_TYPE_OBJ) = 0;
	virtual QDetectObj* getObj(int nIndex, DataTypeEnum emDataType = EM_DATA_TYPE_OBJ) = 0;
	virtual void pushObj(QDetectObj* pObj, DataTypeEnum emDataType = EM_DATA_TYPE_OBJ) = 0;
	virtual void deleteObj(int nIndex, DataTypeEnum emDataType = EM_DATA_TYPE_OBJ) = 0;
	virtual void clearObjs(DataTypeEnum emDataType = EM_DATA_TYPE_OBJ) = 0;

    virtual bool createProject(QString& szFilePath) = 0;
    virtual bool openProject(QString& szFilePath) = 0;

	virtual bool saveDataBase(QString& szFilePath = QString(""), DataTypeEnum emDataType = EM_DATA_TYPE_OBJ) = 0;
	virtual bool loadDataBase(QString& szFilePath = QString(""), DataTypeEnum emDataType = EM_DATA_TYPE_OBJ) = 0;

	virtual unsigned int getCoreData(int nIndex) = 0;

	virtual int getProfObjNum() = 0;
	virtual int increaseProfObjIndex() = 0;
	virtual int getProfObjIndex() = 0;
	virtual QProfileObj* getProfObj(int nIndex) = 0;
	virtual void pushProfObj(QProfileObj* pObj) = 0;
	virtual void deleteProfObj(int nIndex) = 0;
	virtual void clearProfObjs() = 0;

	virtual bool saveProfDataBase(QString& szFilePath) = 0;
	virtual bool loadProfDataBase(QString& szFilePath) = 0;

    virtual bool doAlignment(const Vision::VectorOfMat &vecFrameImages ) = 0;
    virtual QString getDataStoreApiVersion() const = 0;
    virtual Vision::VectorOfMat getCombinedBigImages() const = 0;
    virtual cv::Mat getCombinedBigHeight() const = 0;
    virtual QString getCurrentProject() const = 0;

    virtual QString getDeviceType(long deviceID) const = 0;
    virtual bool copyDevice(long srcID, long destID) = 0;
};

#endif