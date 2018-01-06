#include "VisionDetectSetting.h"

#include "visiondetect_global.h"

#include "../Common/SystemData.h"
#include "../include/IdDefine.h"
#include "../Common/eos.h"
#include <QMessageBox>
#include <QtMath>
#include <QThread>
#include <QDebug>
#include <QThreadPool>

#include "../Common/ModuleMgr.h"
#include "../include/ICamera.h"
#include "../include/IDlp.h"

VisionDetectSetting::VisionDetectSetting(VisionCtrl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QMainWindow(parent)
{
	ui.setupUi(this);	
}

VisionDetectSetting::~VisionDetectSetting()
{	
}