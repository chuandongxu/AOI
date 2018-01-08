#pragma once

#include "visiondetect_global.h"

#include "../Common/modulebase.h"
#include "../include/IVision.h"
#include "VisionCtrl.h"

class VisionDetectRunView;
class  VisionDetect : public QModuleBase, public IVision
{
public:
	VisionDetect(int id, const QString &name);
	~VisionDetect();

	virtual void addSettingWiddget(QTabWidget * tabWidget);

public:
	virtual QWidget* getDetectView();
	virtual QWidget* getCellEditorView();

public:
	virtual bool loadCmdData(int nStation);

	virtual bool setHeightData(cv::Mat& matHeight);
	virtual bool setProfGrayImg(cv::Mat& imgGray);
	virtual bool setProf3DImg(QImage& img3D);
	virtual bool setProfData(QVector<cv::Point2d>& profDatas);
	virtual bool setProfRange(double x1, double y1, double x2, double y2);
	virtual void prepareNewProf();

	virtual bool calculate3DHeight(int nStation, QVector<cv::Mat>& imageMats, cv::Mat& heightMat, cv::Mat& matHeightResultImg);
	virtual bool generateGrayImage(QVector<cv::Mat>& imageMats, cv::Mat& grayMat);
	virtual bool matchPosition(cv::Mat& matDisplay, QVector<QDetectObj*>& objTests);
	virtual bool calculateDetectHeight(cv::Mat& matHeight, QVector<QDetectObj*>& objTests);
	virtual bool merge3DHeight(QVector<cv::Mat>& matHeights, cv::Mat& matHeight);

	virtual bool matchAlignment(cv::Mat& matDisplay, QVector<QProfileObj*>& objProfTests);
	virtual bool calculateDetectProfile(cv::Mat& matHeight, QVector<QProfileObj*>& objProfTests);

private:
	VisionCtrl m_ctrl;
	VisionDetectRunView* m_pDetectView;
};
