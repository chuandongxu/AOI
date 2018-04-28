#pragma once

#include <QWidget>
#include <QVector>
#include "ui_FiducialMarkWidget.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "DataCtrl.h"

using namespace AOI;
using namespace NFG::AOI;

class FiducialMarkWidget : public QWidget
{
    Q_OBJECT

public:
    FiducialMarkWidget(DataCtrl *pDataCtrl, QWidget *parent = Q_NULLPTR);
    ~FiducialMarkWidget();
    static QString formatAlignmentName(const Engine::Alignment &alignment, int No);

protected:
    virtual void showEvent(QShowEvent *event) override;
    int srchFiducialMark();
    int refreshFMWindow();
    cv::Mat _readFrameImageFromFolder(int nFrameX, int nFrameY);
    cv::Mat _getFrameImageFromBigImage(const cv::Mat &matBigImage, int nFrameX, int nFrameY, int nImageWidth, int nImageHeight, int nOverlapX, int nOverlapY);

private slots:    
    void on_btnSelectFiducialMark_clicked();
    void on_btnConfirmFiducialMark_clicked();
    void on_btnDoAlignment_clicked();
    void on_btnRemoveFM_clicked();
    void on_comboBoxChooseImage_indexChanged(int);
    
private:
    Ui::FiducialMarkWidget      ui;
    QVector<cv::RotatedRect>    m_vecFMCadWindow;
    Vision::VectorOfPoint2f     m_vecFMBigImagePos;
    float                       m_fCadOffsetX = 0.f;
    float                       m_fCadOffsetY = 0.f;
    int                         m_nBigImageWidth = 0;
    int                         m_nBigImageHeight = 0;

    Engine::AlignmentVector     m_vecAlignmentDB;
    DataCtrl                   *m_pDataCtrl;
};
