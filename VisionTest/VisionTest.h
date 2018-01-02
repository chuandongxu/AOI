#pragma once

#include "visiontest_global.h"

#include "../Common/modulebase.h"
#include "../include/IVisionTest.h"
#include "VisionTestCtrl.h"

class VisionTest : public QModuleBase, public IVisionTest
{
public:
	VisionTest(int id, const QString &name);
	~VisionTest();

	virtual void addSettingWiddget(QTabWidget * tabWidget);

private:
	VisionTestCtrl m_ctrl;
};
