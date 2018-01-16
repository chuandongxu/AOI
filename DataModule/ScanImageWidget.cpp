#include "ScanImageWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include "../include/VisionUI.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "opencv.hpp"
#include "DataStoreAPI.h"
#include "SystemData.h"

using namespace NFG::AOI;

ScanImageWidget::ScanImageWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

ScanImageWidget::~ScanImageWidget()
{
}

void ScanImageWidget::on_btnSelectCombinedImage_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilter(tr("Image Files (*.png *.jpg *.bmp)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())  {
        fileNames = dialog.selectedFiles();
    }else
        return;

    auto selectedImage = fileNames[0];
    auto matImage = cv::imread ( selectedImage.toStdString() );

    Engine::DeviceVector vecDevice;
    auto result = Engine::GetAllDevices ( vecDevice );
    if ( Engine::OK != result ) {
        String errorType, errorMessage;
        Engine::GetErrorDetail ( errorType, errorMessage );
        errorMessage = "Failed to get devices from project, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    bool bBoardRotated = ui.checkBoxBoardRotated->isChecked();
    double dResolutionX = ui.lineEditResolutionX->text().toDouble();
    double dResolutionY = ui.lineEditResolutionY->text().toDouble();

    QVector<cv::RotatedRect> vecDeviceWindows;
    for ( const auto &device : vecDevice ) {
        auto x = device.x / dResolutionX;
        auto y = device.y / dResolutionY;
        if ( bBoardRotated )
            x = matImage.cols - x;
        else
            y = matImage.rows - y; //In cad, up is positive, but in image, down is positive.
        auto width  = device.width  / dResolutionX;
        auto height = device.height / dResolutionY;
        cv::RotatedRect deviceWindow ( cv::Point2f(x, y), cv::Size2f(width, height), device.angle );
        vecDeviceWindows.push_back ( deviceWindow );
    }

    System->setSysParam("CAM_RESOLUTION_X", dResolutionX);
	System->setSysParam("CAM_RESOLUTION_Y", dResolutionY);
    System->setSysParam("BOARD_ROTATED", bBoardRotated );

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SET_FIDUCIAL_MARK);
    pUI->setImage ( matImage, true );
    pUI->setDeviceWindows ( vecDeviceWindows );
}