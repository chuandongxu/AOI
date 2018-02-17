#ifndef _VISION_UI_H_
#define _VISION_UI_H_

#include <qwidget.h>
#include <qstring.h>

#include "../lib/VisionLibrary/include/opencv/cv.h"
#include "constants.h"

class QDetectObj;
class IVisionUI 
{
public:
	virtual void init() = 0;
	virtual void Show() = 0;
	virtual void unInit() = 0;

	virtual void setStateWidget(QWidget * stateWidget) = 0;
	virtual void setCneterwidget(QWidget * centWidget) = 0;
	virtual void setTitle(const QString & str,const QString &ver) = 0;

	virtual QWidget* getVisionView() = 0; // Display on the main UI

	virtual void setImage(const cv::Mat& matImage, bool bDisplay = true) = 0;
	virtual cv::Mat getImage() = 0;
	virtual void clearImage() = 0;
	virtual void addImageText(QString szText) = 0;
	virtual void displayImage(cv::Mat& image) = 0;

	virtual void setHeightData(cv::Mat& matHeight) = 0;
	virtual bool startUpCapture() = 0;
	virtual bool endUpCapture() = 0;

	virtual void load3DViewData(int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues) = 0;
	virtual void show3DView() = 0;

	virtual cv::Mat getSelectImage() = 0;
	virtual void clearSelect() = 0;
	virtual cv::Rect2f getSelectedROI() = 0;

	virtual void displayObjs(QVector<QDetectObj*> objs, bool bShowNumber = false) = 0;
    virtual void setDetectObjs(const QVector<QDetectObj> &vecDetectObjs) = 0;
    virtual QVector<QDetectObj> getDetectObjs() const = 0;
    virtual void setDeviceWindows(const QVector<cv::RotatedRect> &vecWindows) = 0;
    virtual void setSelectedFM(const QVector<cv::RotatedRect> &vecWindows) = 0;
    virtual void setViewState(VISION_VIEW_MODE enViewMode) = 0;
    virtual void getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) = 0;
};

#endif