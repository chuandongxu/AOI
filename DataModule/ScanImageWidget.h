#pragma once

#include <QWidget>
#include "ui_ScanImageWidget.h"
#include "VisionAPI.h"

class ScanImageWidget : public QWidget
{
    Q_OBJECT

public:
    ScanImageWidget(QWidget *parent = Q_NULLPTR);
    ~ScanImageWidget();

private slots:
	void on_btnCombineImageParamsSave_clicked();

    void on_btnCombineLoadImage_clicked();
    void on_btnSelectFrameImages_clicked();
    void on_btnSelectCombinedImage_clicked();
	

private:
    cv::Mat combineImage(const QString &strFolder);
    void updateImageDeviceWindows(const cv::Mat &matImage);
private:
    Ui::ScanImageWidget ui;
};
