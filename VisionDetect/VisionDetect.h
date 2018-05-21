#pragma once

#include "visiondetect_global.h"

#include "../Common/modulebase.h"
#include "../include/IVision.h"
#include "VisionCtrl.h"
#include "InspWindowWidget.h"

class VisionDetectRunView;
class SysCalibrationView;
class LightCalibrationView;
class TableCalibrationView;
class QColorWeight;
class Inspect3DProfileWidget;
class  VisionDetect : public QModuleBase, public IVision
{
public:
	VisionDetect(int id, const QString &name);
	~VisionDetect();

	virtual void addSettingWiddget(QTabWidget * tabWidget);

public:
	virtual QWidget* getDetectView();
	virtual QWidget* getCellEditorView();
	virtual QWidget* getCaliView();
    virtual QWidget* getInspWindowView() override;
	virtual QWidget* getColorWeightView() override;
    virtual QWidget* getInspect3DProfileView() override;

public:
	virtual bool loadCmdData(int nStation);

	virtual bool setHeightData(const cv::Mat& matHeight);
	virtual bool setProfGrayImg(cv::Mat& imgGray);
	virtual bool setProf3DImg(QImage& img3D);
	virtual bool setProfData(QVector<cv::Point2d>& profDatas);
	virtual bool setProfRange(double x1, double y1, double x2, double y2);
	virtual void prepareNewProf();

	virtual bool calculate3DHeight(int nStation, QVector<cv::Mat>& imageMats, cv::Mat& heightMat, cv::Mat& matHeightResultImg);
	virtual bool generateAverageImage(const QVector<cv::Mat>& imageMats, cv::Mat& grayMat) override;
	virtual bool matchPosition(cv::Mat& matDisplay, QVector<QDetectObj*>& objTests);
	virtual bool calculateDetectHeight(cv::Mat& matHeight, QVector<QDetectObj*>& objTests);
	virtual bool merge3DHeight(QVector<cv::Mat>& matHeights, cv::Mat& matHeight);
	virtual bool mergeImage(QVector<cv::Mat>& matInputImages, QVector<cv::Mat>& matOutputImages);

	virtual bool matchAlignment(cv::Mat& matDisplay, QVector<QProfileObj*>& objProfTests);
	virtual bool calculateDetectProfile(cv::Mat& matHeight, QVector<QProfileObj*>& objProfTests);

	virtual cv::Mat generateGrayImage(cv::Mat& img, cv::Point ptPos) override;
	virtual cv::Mat generateColorImage(cv::Mat& img, cv::Point ptPos) override;

    virtual void setColorWidgetImage(const cv::Mat &matImg) override;
    virtual cv::Mat getColorWidgetProcessedImage() override;
    virtual QString getVisionLibraryVersion() const { return m_ctrl.getVisionLibraryVersion(); }

    virtual bool setInspect3DHeight(QVector<cv::Mat>& matHeights) override;
    virtual void inspect3DProfile() override;

private:
	VisionCtrl m_ctrl;
	VisionDetectRunView* m_pDetectView;
	SysCalibrationView* m_pCaliView; 
	LightCalibrationView* m_pLightCaliView;
	TableCalibrationView* m_pTableCaliView;
	QColorWeight* m_pColorWeightView;
    Inspect3DProfileWidget* m_pInspect3DProfileView;
    InspWindowWidget* m_pInspWindowView;
};
