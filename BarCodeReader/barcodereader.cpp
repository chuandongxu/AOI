#include "barcodereader.h"
#include "BarCodeReaderSetting.h"
#include "../common/SystemData.h"
BarCodeReader::BarCodeReader(int id,const QString &name)
	:QModuleBase(id,name),m_pDevMgr(NULL)
{
	m_pDevMgr = new QBarCodeDevMgr(NULL);
}

BarCodeReader::~BarCodeReader()
{
	if(m_pDevMgr)delete m_pDevMgr;
}

void BarCodeReader::addSettingWiddget(QTabWidget * tabWidget)
{
	QString user;
	int level = 0;
	System->getUser(user,level);
	if(USER_LEVEL_MANAGER >level) return ;
	if(tabWidget)
	{
		tabWidget->addTab(new QBarCodeReaderSetting(m_pDevMgr),QStringLiteral("读码器设置"));
	}
}

QString BarCodeReader::readBarcode(int devIdx)
{
	if(m_pDevMgr)
	{
		QBarCodeDevice * pDev = m_pDevMgr->getDevice(devIdx);
		if(pDev)return pDev->readBarCode();
	}

	return "";
}

QMOUDLE_INSTANCE(BarCodeReader)