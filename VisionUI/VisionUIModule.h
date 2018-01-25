﻿#ifndef VISIONUI_H
#define VISIONUI_H

#include "visionui_global.h"
#include "../include/VisionUI.h"
#include "../Common/modulebase.h"

class VisionView;
class QVisionUIModule : public IVisionUI, public QModuleBase
{
public:
	QVisionUIModule(int id,const QString &name);
	~QVisionUIModule();

public:
	virtual void init();
	virtual void Show();
	virtual void unInit();

	void setStateWidget(QWidget * stateWidget);
	void setCneterwidget(QWidget * centWidget);
	void setTitle(const QString & str,const QString &ver);

	virtual QWidget* getVisionView();
	virtual void setImage(const cv::Mat& matImage, bool bDisplay);
	virtual void setHeightData(cv::Mat& matHeight);
	virtual bool startUpCapture();
	virtual bool endUpCapture();

	virtual cv::Mat getImage();
	virtual void clearImage();
	virtual void addImageText(QString szText);
	virtual void displayImage(cv::Mat& image);

	virtual void load3DViewData(int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues);
	virtual void show3DView();

	virtual void setSelect();
	virtual cv::Mat getSelectImage();
	virtual void clearSelect();
	virtual cv::Rect2f getSelectScale();

	virtual void displayObjs(QVector<QDetectObj*> objs, bool bShowNumber) override;
    virtual void setDeviceWindows(const QVector<cv::RotatedRect> &vecWindows) override;
    virtual void setSelectedFM(const QVector<cv::RotatedRect> &vecWindows) override;
    virtual void setViewState(VISION_VIEW_MODE enViewMode) override;
    virtual void getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) override; 

private:
	void * m_mainWidget;
	VisionView* m_pVisionView;
};

#endif // VISIONUI_H
