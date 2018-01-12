#pragma once

#include <QWidget>
#include "ui_VisionDetectRunView.h"

#include "VisionCtrl.h"
#include "VLMaskEditor.h"
#include "VLCellEditor.h"
#include "VLProfileEditor.h"

using VectorOfFloat = std::vector < float > ;
using VectorOfVectorOfFloat = std::vector < VectorOfFloat > ;

class IGraphicEditor;
class QResultDisplay;
class QColorImageDisplay;
class QDlpMTFRsltDisplay;
class QCameraRunnable;
class IVisionUI;
class VisionDetectRunView : public QWidget
{
	Q_OBJECT

public:
	VisionDetectRunView(VisionCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~VisionDetectRunView();

public:
	QWidget* getCellEditorView();

private:
	void initUI();
	void initLimits();

	IVisionUI* getVisionUI();

private slots:
	void onObjEvent(const QVariantList &data);

	void onSrhLineDirIndexChanged(int iIndex);
	void onSrhLineROI();
	void onSrhLineDetect();
	void onSaveDetectParam();

	void onTmpMatchObjMotionIndexChanged(int iIndex);
	void onSaveTmpMatchParam();
	void onTmpMatchOpen();
	void onTmpMatchSelectROI();
	void onTmpMatchDeleteROI();
	void onTmpMatchSelectDetect();
	void onTmpMatchDetect();
	void onComBoxSubPixel(int iState);


	void onSaveDetectEdgeParams();
	void onDetectEdgeROI();
	void onDetectEdge();

	void onSrhCircleROI();
	void onSrhCircleDetect();


	void onDetectGrayScaleOpen();
	void onSaveDetectGrayScaleParams();
	void onDetectGrayScale();

	// 3D Calibration
	void onDLPIndexChanged(int iState);
	void on3DCaliOpen();
	void on3DCaliRstOpen();
	void on3DCali();
	void onSave3DDetectCaliParams();

	void onSelectCaliTypeChanged(int iState);
	void on3DDetectCaliOpen();
	void on3DDetectCali();
	void on3DDetectCaliNegOpen();
	void on3DDetectCaliNeg();
	void on3DDetectCaliComb();
	void onSave3DDetectCaliHeightParams();

	void on3DDetectOpen();
	void on3DDetect();
	void onPhaseShiftValueChanged(double dValue);
	void on3DDetectMerge();
	void on3DDetectShow();
	void onSave3DDetectParams();
	void on3DHeightDetect();
	void onSaveDataParams();
	void on3DHeightCellObjEdit();

	void on3DProfileEdit();

	void onCaliGuide();
	void onCaliGuideNext();
	void onCaliGuidePrevious();

	// MTF calculation	
	void onEditMTFLoc1();
	void onEditMTFLocLearn1();
	void onSearchMTFLoc1();
	void onEditMTFDetect1();
	void onEditMTFLoc2();
	void onEditMTFLocLearn2();
	void onSearchMTFLoc2();
	void onEditMTFDetect2();
	void onCalcMTF();
	void onLoadMTFFile();
	void onSaveMTFFile();

	// DLP MTF Detect
	void onDlpMTFOpen();
	void onSaveDlpMTFParams();
	void onDetectDlpMTF();

	// DLP Pattern distortion Detect
	void onDlpPDOpen();
	void onDetectDlpPD();

private:
	void displayMTFRect();

	double convertToPixel(double umValue);
	double convertToUm(double pixel);

	cv::Mat drawHeightGrid(const cv::Mat &matHeight, int nGridRow, int nGridCol);
	cv::Mat drawHeightGrid2(const cv::Mat &matHeight, int nGridRow, int nGridCol);
	cv::Mat drawHeightGray(const cv::Mat &matHeight);

	void saveMTFData();
	void loadMTFData();

	cv::Point2f rotateByPoint(cv::Point2f ptSrc, cv::Point2f ptCenter, double angle);

	bool convertToGrayImage(QString& szFilePath, cv::Mat &matGray);
	bool readImages(QString& szFilePath, AOI::Vision::VectorOfMat& matImgs);

	void updatePhaseShift(double dValue);

public:
	void guideDisplayImages();

private:
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

private:
	Ui::VisionDetectRunView ui;
	int m_nLevel;
	VisionCtrl* m_pCtrl;
	QVLMaskEditor *m_pVLMaskEditor;
	QVLCellEditor *m_pVLCellObjEditor;
	QVLProfileEditor *m_pVLProflieEditor;

	QGraphicsScene * m_TmpMatchScene;
	cv::Mat m_tmpMatchImg;
	cv::Mat m_tmpMatchROI;
	cv::Rect m_tmpMatchRect;
	int m_tmpRecordID;

	cv::Rect m_detectEdgeROI;
	cv::Mat m_detectEdgeImg;
	cv::Rect m_srhCircleROI;

	QStringList m_detectGrayScaleFiles;

	int m_nImageRow;
	int m_nImageCol;
	cv::Mat m_3DMatHeightMerge;
	QVector<cv::Mat> m_3DMatHeights;
	IGraphicEditor *m_pGraphicsEditor;
	QResultDisplay *m_pResultDisplay;
	QColorImageDisplay* m_pColorDisplay;

	QGraphicsScene * m_TmpMatchMTFScene1;
	QGraphicsScene * m_TmpMatchMTFScene2;
	int m_nMTFLocRecord1;
	int m_nMTFLocRecord2;
	cv::Rect m_MTFLocROI1;
	cv::Mat m_MTFRtImage;
	cv::Rect m_MTFDetectROI1;
	cv::Rect m_MTFLocROI2;
	cv::Rect m_MTFDetectROI2;

	QDlpMTFRsltDisplay* m_pDlpMTFDisplay;
private:
	QIntValidator *inputIntRangePos;
	QIntValidator *inputIntRangeAll;
	QDoubleValidator *inputDoubleRangePos;
	QDoubleValidator *inputDoubleRangeAll;	
};
