#pragma once

#include <QWidget>
#include <QVector>
#include "ui_FiducialMarkWidget.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"

using namespace AOI;

class FiducialMarkWidget : public QWidget
{
    Q_OBJECT

public:
    FiducialMarkWidget(QWidget *parent = Q_NULLPTR);
    ~FiducialMarkWidget();
protected:
    virtual void showEvent(QShowEvent *event) override;
    int srchFiducialMark();
    int refreshFMWindow();
private slots:    
    void on_btnSelectFiducialMark_clicked();
    void on_btnConfirmFiducialMark_clicked();
    void on_btnDoAlignment_clicked();
    
private:
    Ui::FiducialMarkWidget ui;
    QVector<cv::RotatedRect> m_vecFMCadWindow;
    Vision::VectorOfPoint2f m_vecFMBigImagePos;
    float m_fCadOffsetX = 0.f;
    float m_fCadOffsetY = 0.f;
    int m_nBigImageWidth = 0;
    int m_nBigImageHeight = 0;
};
