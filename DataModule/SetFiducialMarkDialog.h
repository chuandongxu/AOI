#pragma once

#include <QDialog>
#include "ui_SetFiducialMarkDialog.h"
#include "VisionAPI.h"

using namespace AOI;

class SetFiducialMarkDialog : public QDialog
{
    Q_OBJECT

public:
    SetFiducialMarkDialog(QWidget *parent = Q_NULLPTR);
    ~SetFiducialMarkDialog();
    bool getUseStandardShape();
    float getFiducialMarkSize();
    bool getIsFiducialMarkDark();
    Vision::PR_FIDUCIAL_MARK_TYPE getFiducialMarkShape();
private:
    Ui::SetFiducialMarkDialog ui;
};
