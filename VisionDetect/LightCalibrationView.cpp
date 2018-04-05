#include "LightCalibrationView.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/ICamera.h"
#include "../include/IMotion.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace AOI;

LightCalibrationView::LightCalibrationView(VisionCtrl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pushButton_Joystick, SIGNAL(clicked()), SLOT(onReadyPosJoystick()));
}

LightCalibrationView::~LightCalibrationView()
{
}

void LightCalibrationView::onReadyPosJoystick()
{
	IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (pMotion)
	{
		pMotion->setJoystickXMotor(AXIS_MOTOR_X, 1.2, ui.lineEdit_readyPosX);
		pMotion->setJoystickYMotor(AXIS_MOTOR_Y, 1.2, ui.lineEdit_readyPosY);

		pMotion->startJoystick();
	}
}
