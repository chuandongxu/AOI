#pragma once

#include <QWidget>
#include "ui_LightCalibrationView.h"
#include "VisionCtrl.h"

class LightCalibrationView : public QWidget
{
	Q_OBJECT

public:
	LightCalibrationView(VisionCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~LightCalibrationView();

private slots:
	void onReadyPosJoystick();

private:
	Ui::LightCalibrationView ui;
	VisionCtrl* m_pCtrl;
};
