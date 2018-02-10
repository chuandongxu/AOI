#include "QDataModule.h"
#include "../common/SystemData.h"
#include "DataSetting.h"
#include "QStatisticsWidget.h"
#include "DataEditor.h"

QDataModule::QDataModule(int id, const QString &name)
	:QModuleBase(id, name)
{
	m_pStatisticsWidget = new QStatisticsWidget(&m_ctrl);
	m_pDataEditor = new DataEditor();
    m_pDataWidget = new DataWidget();
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
		//tabWidget->addTab(new DataSetting(&m_ctrl), QStringLiteral(" ˝æ›…Ë÷√"));
	}
}

QWidget* QDataModule::getStatisticsWidget()
{
	return m_pStatisticsWidget;
}

QWidget* QDataModule::getDataEditor()
{
	return m_pDataEditor;
}

QWidget* QDataModule::getDataWidget()
{
    return m_pDataWidget;
}

void QDataModule::incrementCycleTests()
{
	m_ctrl.incrementCycleTests();
}

void QDataModule::decrementCycleTests()
{
	m_ctrl.decrementCycleTests();
}

int	QDataModule::getCycleTests()
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
    return m_ctrl.createProject ( szFilePath );
}

bool QDataModule::openProject(QString& szFilePath)
{
    return m_ctrl.openProject ( szFilePath );
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

QMOUDLE_INSTANCE(QDataModule)
