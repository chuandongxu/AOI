#include "QDataModule.h"
#include "../common/SystemData.h"
#include "../Common/eos.h"
#include "../include/IdDefine.h"
#include "DataSetting.h"
#include "DataEditor.h"
#include "SearchDeviceWidget.h"

QDataModule::QDataModule(int id, const QString &name)
    :QModuleBase(id, name)
{
    m_pDataEditor = new DataEditor();
    m_pDataWidget = new DataWidget(&m_ctrl);
    m_pDeviceListWidget = new SearchDeviceWidget();
}

QDataModule::~QDataModule()
{
}

void QDataModule::addSettingWiddget(QTabWidget * tabWidget)
{
    QString user;
    int level = 0;
    System->getUser(user, level);
    if (USER_LEVEL_MANAGER > level) return;
    if (tabWidget)
    {
        //tabWidget->addTab(new DataSetting(&m_ctrl), QStringLiteral("Êý¾ÝÉèÖÃ"));
    }
}

QWidget* QDataModule::getDataEditor()
{
    return m_pDataEditor;
}

QWidget* QDataModule::getDataWidget()
{
    return m_pDataWidget;
}

QWidget* QDataModule::getDeviceListWidget()
{
    return m_pDeviceListWidget;
}

void QDataModule::incrementCycleTests()
{
    m_ctrl.incrementCycleTests();
}

void QDataModule::decrementCycleTests()
{
    m_ctrl.decrementCycleTests();
}

int    QDataModule::getCycleTests()
{
    return m_ctrl.getCycleTests();
}

QBoardObj* QDataModule::getBoardObj()
{
    return m_ctrl.getBoardObj();
}

int QDataModule::getObjNum(DataTypeEnum emDataType)
{
    return m_ctrl.getObjNum(emDataType);
}

QDetectObj* QDataModule::getObj(int nIndex, DataTypeEnum emDataType)
{
    return m_ctrl.getObj(nIndex, emDataType);
}

void QDataModule::pushObj(QDetectObj* pObj, DataTypeEnum emDataType)
{
    m_ctrl.pushObj(pObj, emDataType);
}

void QDataModule::deleteObj(int nIndex, DataTypeEnum emDataType)
{
    m_ctrl.deleteObj(nIndex, emDataType);
}

void QDataModule::clearObjs(DataTypeEnum emDataType)
{
    m_ctrl.clearObjs(emDataType);
}

bool QDataModule::createProject(QString& szFilePath)
{
    bool bResult = m_ctrl.createProject(szFilePath);
    if (bResult)
    {
        m_strCurrentProject = szFilePath;
        QEos::Notify(EVENT_SEARCH_DEVICE_STATE, 0, SEARCH_ALL_DEVICE);
    }
    return bResult;
}

bool QDataModule::openProject(QString& szFilePath)
{
    bool bResult = m_ctrl.openProject(szFilePath);
    if (bResult)
    {
        m_strCurrentProject = szFilePath;
        QEos::Notify(EVENT_SEARCH_DEVICE_STATE, 0, SEARCH_ALL_DEVICE);
    }
    return bResult;
}

bool QDataModule::saveDataBase(QString& szFilePath, DataTypeEnum emDataType)
{
    return m_ctrl.saveDataBase(szFilePath, emDataType);
}

bool QDataModule::loadDataBase(QString& szFilePath, DataTypeEnum emDataType)
{
    return m_ctrl.loadDataBase(szFilePath, emDataType);
}

unsigned int QDataModule::getCoreData(int nIndex)
{
    return m_ctrl.getCoreData(nIndex);
}

int QDataModule::getProfObjNum()
{
    return m_ctrl.getProfObjNum();
}

int QDataModule::increaseProfObjIndex()
{
    return m_ctrl.increaseProfObjIndex();
}

int QDataModule::getProfObjIndex()
{
    return m_ctrl.getProfObjIndex();
}

QProfileObj* QDataModule::getProfObj(int nIndex)
{
    return m_ctrl.getProfObj(nIndex);
}

void QDataModule::pushProfObj(QProfileObj* pObj)
{
    m_ctrl.pushProfObj(pObj);
}

void QDataModule::deleteProfObj(int nIndex)
{
    m_ctrl.deleteProfObj(nIndex);
}

void QDataModule::clearProfObjs()
{
    m_ctrl.clearProfObjs();
}

bool QDataModule::saveProfDataBase(QString& szFilePath)
{
    return m_ctrl.saveProfDataBase(szFilePath);
}

bool QDataModule::loadProfDataBase(QString& szFilePath)
{
    return m_ctrl.loadProfDataBase(szFilePath);
}

QString QDataModule::getDeviceType(long deviceID) const
{
    return m_pDeviceListWidget->getDeviceType(deviceID);
}

bool QDataModule::copyDeviceWindow(long srcID, long destID)
{
    return m_pDeviceListWidget->copyDeviceWindow(srcID, destID);
}

bool QDataModule::copyDeviceWindowAsMirror(long srcID, bool bHorizontal)
{
    return m_pDeviceListWidget->copyDeviceWindowAsMirror(srcID, bHorizontal);
}

bool QDataModule::displayRecord(int recordID)
{
    return m_pDataEditor->displayRecord(recordID);
}

QMOUDLE_INSTANCE(QDataModule)
