#pragma once

#include <QWidget>
#include "ui_DLPTableCalibrationView.h"
#include "VisionAPI.h"

#include <memory>

using namespace AOI;

class ScanImageThread;
class DLPTableCalibrationRstWidget;
class DLPTableCalibrationView : public QWidget
{
    Q_OBJECT

public:
    DLPTableCalibrationView(QWidget *parent = Q_NULLPTR);
    ~DLPTableCalibrationView();

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void on_btnTopLeft_clicked();
    void on_btnBtmRight_clicked();

    void on_btnPrepareScanImage_clicked();
    void on_btnScanImage_clicked();
    void on_scanImage_done();

private:
    void _saveResult();
    void _displayResult();
    void _showJoyStick();

private:
    Ui::DLPTableCalibrationView ui;

    double      m_dLeftX;
    double      m_dTopY;
    double      m_dRightX;
    double      m_dBottomY;

    Vision::VectorOfVectorOfPoint2f  m_vecVecFrameCtr;
    int m_nFrameCountX;
    int m_nFrameCountY;

    ScanImageThread                 *m_pScanImageThread;
    std::shared_ptr<DLPTableCalibrationRstWidget> m_pDisplayWidget;
};
