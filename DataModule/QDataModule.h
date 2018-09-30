#pragma once

#include "datamodule_global.h"

#include "../Common/modulebase.h"
#include "../include/IData.h"
#include "DataCtrl.h"
#include "DataWidget.h"

class SearchDeviceWidget;
class DataEditor;
class QDataModule : public QModuleBase, public IData
{
public:
	QDataModule(int id, const QString &name);
	~QDataModule();

	virtual void addSettingWiddget(QTabWidget * tabWidget); 

	virtual QWidget* getDataEditor() override;
    virtual QWidget* getDataWidget() override;
    virtual QWidget* getDeviceListWidget() override;

	virtual void incrementCycleTests();
	virtual void decrementCycleTests();
	virtual int	getCycleTests();

	virtual QBoardObj* getBoardObj();
	virtual int getObjNum(DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);
	virtual QDetectObj* getObj(int nIndex, DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);
	virtual void pushObj(QDetectObj* pObj, DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);
	virtual void deleteObj(int nIndex, DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);
	virtual void clearObjs(DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);

    virtual bool createProject(QString& szFilePath) override;
    virtual bool openProject(QString& szFilePath) override;

	virtual bool saveDataBase(QString& szFilePath = QString(""), DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);
	virtual bool loadDataBase(QString& szFilePath = QString(""), DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);

	virtual unsigned int getCoreData(int nIndex);

	virtual int getProfObjNum();
	virtual int increaseProfObjIndex();
	virtual int getProfObjIndex();
	virtual QProfileObj* getProfObj(int nIndex);
	virtual void pushProfObj(QProfileObj* pObj);
	virtual void deleteProfObj(int nIndex);
	virtual void clearProfObjs();

	virtual bool saveProfDataBase(QString& szFilePath);
	virtual bool loadProfDataBase(QString& szFilePath);

    virtual QString getDataStoreApiVersion() const override { return m_ctrl.getDataStoreApiVersion(); }
    virtual Vision::VectorOfMat getCombinedBigImages() const override { return m_ctrl.getCombinedBigImages(); }
    virtual cv::Mat getCombinedBigHeight() const override { return m_ctrl.getCombinedBigHeight(); }
    virtual QString getCurrentProject() const override { return m_strCurrentProject; }

    virtual QString getDeviceType(long deviceID) const override;
    virtual bool copyDeviceWindow(long srcID, long destID) override;
    virtual bool copyDeviceWindowAsMirror(long srcID, bool bHorizontal, QVector<int>& winIDs, QVector<int>& groupIDs) override;

    virtual bool displayRecord(int recordID) override;

private:
	DataCtrl m_ctrl;
    DataEditor *m_pDataEditor;
    QWidget *m_pDataWidget;
    SearchDeviceWidget *m_pDeviceListWidget;
    QString  m_strCurrentProject;
};
