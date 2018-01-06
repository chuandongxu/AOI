#ifndef VISIONUI_H
#define VISIONUI_H

#include "visionui_global.h"
#include "../include/VisionUI.h"
#include "../Common/modulebase.h"

class QMainView;
class QVisionUIModule : public IVisionUI,public QModuleBase
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

	virtual QWidget* getMainView();
	virtual void setImage(cv::Mat& matImage, bool bDisplay);
	virtual void setHeightData(cv::Mat& matHeight);
	virtual bool startUpCapture();
	virtual bool endUpCapture();

	virtual cv::Mat getImage();
	virtual void clearImage();
	virtual void addImageText(QString szText);
	virtual void displayImage(cv::Mat& image);

private:
	void * m_mainWidget;
	QMainView* m_pMainView;
};

#endif // VISIONUI_H
