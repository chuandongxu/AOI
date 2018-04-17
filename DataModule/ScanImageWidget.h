#pragma once

#include <QWidget>
#include "ui_ScanImageWidget.h"
#include "VisionAPI.h"
#include "ScanImageThread.h"
#include "DataCtrl.h"

using namespace AOI;

class ScanImageWidget : public QWidget
{
    Q_OBJECT

public:
    ScanImageWidget(DataCtrl *pDataCtrl, QWidget *parent = Q_NULLPTR);
    ~ScanImageWidget();

private slots:
	void on_btnCombineImageParamsSave_clicked();
    void on_btnPrepareScanImage_clicked();
    void on_btnScanImage_clicked();
    void on_btnCombineLoadImage_clicked();
    void on_btnSelectFrameImages_clicked();
    void on_btnSelectCombinedImage_clicked();
    void on_scanImage_done();
    void on_comboBoxDisplayImage_indexChanged(int);

private:
    cv::Mat combineImage(const QString &strFolder);
    void updateImageDeviceWindows(const cv::Mat &matImage);
    bool _moveToCapturePos(float fPosX, float fPosY);

private:
    Ui::ScanImageWidget              ui;
    Vision::VectorOfVectorOfPoint2f  m_vecVecFrameCtr;
    ScanImageThread                 *m_pScanImageThread;
    DataCtrl                        *m_pDataCtrl;
};
