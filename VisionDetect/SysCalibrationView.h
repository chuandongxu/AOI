#pragma once

#include <QWidget>
#include "ui_SysCalibrationView.h"
#include "VisionCtrl.h"
#include "VisionAPI.h"
#include "opencv2/highgui.hpp"

class QCameraRunnable;
class IVisionUI;
class SysCalibrationView : public QWidget
{
	Q_OBJECT

public:
	SysCalibrationView(VisionCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~SysCalibrationView();

	QWidget* getTab();
    void guideDisplayImages();

private:
	void initUI();
	void initLimits();

	IVisionUI* getVisionUI();

private slots:
	// 3D Calibration
	void onDLPIndexChanged(int iState);
	void on3DCaliOpen();
	void on3DCaliRstOpen();
	void on3DCali();
	void onSave3DDetectCaliParams();

	void onSelectCaliTypeChanged(int iState);
	void on3DDetectCaliOpen();
	void on3DDetectCali();	
	void onSave3DDetectCaliHeightParams();

	void on3DDetectCaliComb();

	void onCaliGuide();
	void onCaliGuideNext();
	void onCaliGuideSkip();
	void onCaliGuidePrevious();

	void onDecryptImages();

private:
	double convertToPixel(double umValue);
	double convertToUm(double pixel);

	bool convertToGrayImage(QString& szFilePath, cv::Mat &matGray);
	bool decrptImages(QString& szFilePath);
	bool readImages(QString& szFilePath, AOI::Vision::VectorOfMat& matImgs);

	cv::Mat drawHeightGrid(const cv::Mat &matHeight, int nGridRow, int nGridCol);
	cv::Mat drawHeightGrid2(const cv::Mat &matHeight, int nGridRow, int nGridCol);
	cv::Mat drawHeightGray(const cv::Mat &matHeight);
    cv::Mat _readImage(const QString &strFilePath, int flags = cv::IMREAD_COLOR);

	bool startCaliGuide();
	void stopCaliGuide();
	bool guideReadImages(QVector<cv::Mat>& matImgs);
	bool guideReadImage(cv::Mat& matImg);

	void startCameraCapturing();
	bool stopCameraCaptureing();

	bool m_bGuideCali;
	int m_nCaliGuideStep;
	QVector<cv::Mat> m_guideImgMats;
	QCameraRunnable* m_pCameraRunnable;    

	Ui::SysCalibrationView ui;
	VisionCtrl* m_pCtrl;
	int m_nLevel;
	QMap<int, int> m_caliStepMap;

	QIntValidator *inputIntRangePos;
	QIntValidator *inputIntRangeAll;
	QDoubleValidator *inputDoubleRangePos;
	QDoubleValidator *inputDoubleRangeAll;
};
