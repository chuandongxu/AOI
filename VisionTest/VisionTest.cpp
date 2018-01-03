#include "VisionTest.h"
#include "../common/SystemData.h"
#include "VisionTestSetting.h"

VisionTest::VisionTest(int id, const QString &name)
	: QModuleBase(id, name)
{
}

VisionTest::~VisionTest()
{

}

void VisionTest::addSettingWiddget(QTabWidget * tabWidget)
{
	QString user;
	int level = 0;
	System->getUser(user, level);
	if (USER_LEVEL_MANAGER > level) return;
	if (tabWidget)
	{
		tabWidget->addTab(new VisionTestSetting(&m_ctrl), QStringLiteral("自动测试"));
	}
}


QMOUDLE_INSTANCE(VisionTest)
