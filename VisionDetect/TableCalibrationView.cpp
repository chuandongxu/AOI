#include "TableCalibrationView.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/ICamera.h"
#include "../include/IMotion.h"
#include "../include/IVisionUI.h"

#include "../lib/VisionLibrary/include/VisionAPI.h"
#define ToInt(value)                (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace AOI;

TableCalibrationView::TableCalibrationView(VisionCtrl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	initUI();
}

TableCalibrationView::~TableCalibrationView()
{
}

void TableCalibrationView::initUI()
{
}
