#pragma once

#include <QWidget>
#include "ui_TableCalibrationView.h"
#include "VisionCtrl.h"

const int _TestImageNum = 2;

class TableCalibrationView : public QWidget
{
    Q_OBJECT

public:
    TableCalibrationView(VisionCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
    ~TableCalibrationView();

private:
    void initUI();

private slots:
    void onReadyPosJoystick();
    void onMoveToReady();

    void onStart();
    void onEnd();

    void onRunTest();

    void onSaveCali();

private:
    bool guideReadImage(cv::Mat& matImg);

private:
    Ui::TableCalibrationView ui;
    VisionCtrl* m_pCtrl;
    bool m_bGuideCali;
    cv::Mat m_matImages[_TestImageNum];
};
