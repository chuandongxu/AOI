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
    static void updateDeviceWindows();

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void on_btnCombineImageParamsSave_clicked();
    void on_btnPrepareScanImage_clicked();
    void on_btnScanImage_clicked();
    void on_btnCombineLoadImage_clicked();
    void on_btnSelectFrameImages_clicked();
    void on_btnSelectCombinedImage_clicked();
    void on_scanImage_done();
    void on_comboBoxDisplayImage_indexChanged(int);
    void on_btnSaveScanImage_clicked();
    void on_btnOpenScanImage_clicked();

private:
    cv::Mat _combineImage(const QString &strFolder);
    bool _moveToCapturePos(float fPosX, float fPosY);
    void _updateImageDeviceWindows(const cv::Mat &matImage);

private:
    Ui::ScanImageWidget              ui;
    Vision::VectorOfVectorOfPoint2f  m_vecVecFrameCtr;
    ScanImageThread                 *m_pScanImageThread;
    DataCtrl                        *m_pDataCtrl;
};
