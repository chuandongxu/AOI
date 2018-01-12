#include "ScanImageWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include "../include/VisionUI.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "opencv.hpp"

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

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    auto matImage = cv::imread ( selectedImage.toStdString() );
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SET_FIDUCIAL_MARK);
    pUI->setImage ( matImage, true );
}