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

private:
	void initUI();

private slots:
	void onReadyPosJoystick();
	void onMoveToReady();

	void onStart();
	void onEnd();
	void onCaptureLight();

	void onSelectLightIndexChanged(int iIndex);
	void onSliderChanged(int lum);

	void onDetectGrayScale();

	void onSaveCali();

private:
	bool guideReadImage(cv::Mat& matImg, int nSelectLight);
	int getLightLum(int nIndex);
	void setLightLum(int nIndex, int lum);

private:
	Ui::LightCalibrationView ui;
	VisionCtrl* m_pCtrl;
	bool m_bGuideCali;
	cv::Mat m_matImage;
};
