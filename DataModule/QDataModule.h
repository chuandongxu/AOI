#pragma once

#include "datamodule_global.h"

#include "../Common/modulebase.h"
#include "../include/IData.h"
#include "DataCtrl.h"
#include "DataWidget.h"

class QDataModule : public QModuleBase, public IData
{
public:
	QDataModule(int id, const QString &name);
	~QDataModule();

	virtual void addSettingWiddget(QTabWidget * tabWidget);    

	virtual QWidget* getToolWidget(bool bDataTool) override;
	virtual QWidget* getDataEditor() override;
    virtual QWidget* getDataWidget() override;

	virtual void incrementCycleTests();
	virtual void decrementCycleTests();
	virtual int	getCycleTests();

	virtual QBoardObj* getBoardObj();
	virtual int getObjNum(DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);
	virtual QDetectObj* getObj(int nIndex, DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);
	virtual void pushObj(QDetectObj* pObj, DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);
	virtual void deleteObj(int nIndex, DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);
	virtual void clearObjs(DataTypeEnum emDataType = EM_DATA_TYPE_OBJ);

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

private:
	DataCtrl m_ctrl;
	QWidget* m_pDataToolWidget;
	QWidget* m_pStatisticsWidget;
	QWidget* m_pDataEditor;
    QWidget* m_pDataWidget;
};
