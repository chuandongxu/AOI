#ifndef VISIONUI_H
#define VISIONUI_H

#include "../include/IVisionUI.h"
#include "../Common/modulebase.h"
#include "VisionView.h"

class QVisionUIModule : public IVisionUI, public QModuleBase
{
public:
    QVisionUIModule(int id,const QString &name);
    ~QVisionUIModule();

    virtual void init();
    virtual void Show();
    virtual void unInit();

    void setStateWidget(QWidget * stateWidget);
    void setTitle(const QString & str,const QString &ver);

    virtual QWidget* getVisionView();
    virtual void setImage(const cv::Mat& matImage, bool bDisplay);
    virtual void setHeightData(const cv::Mat& matHeight);
    virtual cv::Mat getHeightData() const;
    virtual bool startUpCapture(bool bPromptSelect);
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
    Vision::VectorOfRect getSubROIs() const override { return m_pVisionView->getSubROIs(); }
    void setSubROIs(const Vision::VectorOfRect &vecRects) override { m_pVisionView->setSubROIs(vecRects); }
    cv::Rect getSrchWindow() const override { return m_pVisionView->getSrchWindow(); }
    void setSrchWindow(const cv::Rect &rectSrchWindow) override { m_pVisionView->setSrchWindow(rectSrchWindow); }

    virtual void displayObjs(QVector<QDetectObj*> objs, bool bShowNumber) override;
    virtual void setDetectObjs(const QVector<QDetectObj> &vecDetectObjs) override;
    virtual void setCurrentDetectObj(const QDetectObj &detectObj) override;
    virtual QVector<QDetectObj> getDetectObjs() const override;
    virtual void setDeviceWindows(const VisionViewDeviceVector &vecWindows) override;
    virtual const VisionViewDeviceVector& getDeviceWindows() const override;
    virtual void setViewState(VISION_VIEW_MODE enViewMode) override;
    virtual void getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) override; 
    virtual VisionViewDevice getSelectedDevice() override;
    virtual void setConfirmedFM(const VisionViewFMVector &vecFM) override;
    virtual void setCurrentFM(const VisionViewFM &fm) override;
    virtual VisionViewFM getCurrentFM() const override;
    virtual void disableBtnWhenAutoRun() override;
    virtual void enableBtnAfterAutoRun() override;

private:
    QWidget*    m_mainWidget;
    VisionView* m_pVisionView;
};

#endif // VISIONUI_H
