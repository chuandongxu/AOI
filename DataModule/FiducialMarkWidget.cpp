#include "FiducialMarkWidget.h"
#include "../include/VisionUI.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"

FiducialMarkWidget::FiducialMarkWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

FiducialMarkWidget::~FiducialMarkWidget()
{
}

void FiducialMarkWidget::on_btnAdjustCAD_clicked()
{
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SET_FIDUCIAL_MARK);
}

void FiducialMarkWidget::on_btnAddFiducialMark_clicked()
{
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SET_FIDUCIAL_MARK);
}