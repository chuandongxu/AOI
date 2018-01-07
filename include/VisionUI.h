#ifndef _VISION_UI_H_
#define _VISION_UI_H_

#include <qwidget.h>
#include <qstring.h>

#include "../lib/VisionLibrary/include/opencv/cv.h"

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

	virtual void setImage(cv::Mat& matImage, bool bDisplay = true) = 0;
	virtual cv::Mat getImage() = 0;
	virtual void clearImage() = 0;
	virtual void addImageText(QString szText) = 0;
	virtual void displayImage(cv::Mat& image) = 0;

	virtual void setHeightData(cv::Mat& matHeight) = 0;
	virtual bool startUpCapture() = 0;
	virtual bool endUpCapture() = 0;	
};

#endif