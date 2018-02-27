#ifndef VISIONUI_H
#define VISIONUI_H

#include "visionui_global.h"
#include "../include/IVisionUI.h"
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

	virtual cv::Mat getSelectImage();
	virtual void clearSelect() override;
	virtual cv::Rect2f getSelectedROI() override;

	virtual void displayObjs(QVector<QDetectObj*> objs, bool bShowNumber) override;
    virtual void setDetectObjs(const QVector<QDetectObj> &vecDetectObjs) override;
    virtual void setCurrentDetectObj(const QDetectObj &detectObj) override;
    virtual QVector<QDetectObj> getDetectObjs() const override;
    virtual void setDeviceWindows(const VisionViewDeviceVector &vecWindows) override;
    virtual void setSelectedFM(const QVector<cv::RotatedRect> &vecWindows) override;
    virtual void setViewState(VISION_VIEW_MODE enViewMode) override;
    virtual void getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) override; 
    virtual VisionViewDevice getSelectedDevice() override;
private:
	void * m_mainWidget;
	VisionView* m_pVisionView;
};

#endif // VISIONUI_H
