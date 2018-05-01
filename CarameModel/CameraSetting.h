#pragma once

#include <QWidget>
#include "ui_CameraSetting.h"
#include "CameraCtrl.h"

#include "opencv/cv.h"

class IGraphicEditor;
class CameraSetting : public QWidget
{
	Q_OBJECT

public:
	CameraSetting(CameraCtrl* pCameraCtrl, QWidget *parent = Q_NULLPTR);
	~CameraSetting();

public:
	QWidget* getCaliTab();

protected slots:
	void onStateChangeCapture(int iState);
	void onStateChangeCaptureLight(int iState);
	void onStateChangeCaptureAsMatlab(int iState);
	void onStateChangeCrossEnable(int iState);

	void onCaptureModeIndexChanged(int iIndex);
	void onCaptureNumModeIndexChanged(int iIndex);
	void onSelCapturePath();
	void onSaveCapturePath();
	void onSelImgDLP1Path();
	void onSaveImgDLP1Path();
	void onSelImgDLP2Path();
	void onSaveImgDLP2Path();
	void onCalibration();

	void onStartSetupDLP();
	void onEndSetupDLP();
	void onCaptureDLP();
	void onCaptureLight();

private:
	double convertToPixel(double umValue);
	double convertToUm(double pixel);

private:
	void updateUI();

private:
	QIntValidator *inputIntRangePos;
	QIntValidator *inputIntRangeAll;
	QDoubleValidator *inputDoubleRangePos;
	QDoubleValidator *inputDoubleRangeAll;

private:
	Ui::CameraSetting ui;
	CameraCtrl* m_pCameraCtrl;
	IGraphicEditor *m_pGraphicsEditor;	
};
