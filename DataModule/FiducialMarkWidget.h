#pragma once

#include <QWidget>
#include <QVector>
#include "ui_FiducialMarkWidget.h"
#include "opencv.hpp"

class FiducialMarkWidget : public QWidget
{
    Q_OBJECT

public:
    FiducialMarkWidget(QWidget *parent = Q_NULLPTR);
    ~FiducialMarkWidget();

private slots:    
    void on_btnAddFiducialMark_clicked();
    void on_btnConfirmFiducialMark_clicked();
    void on_btnDoAlignment_clicked();
private:
    Ui::FiducialMarkWidget ui;
    QVector<cv::RotatedRect> m_vecFMCadWindow;
    QVector<cv::RotatedRect> m_vecFMImageWindow;
};
