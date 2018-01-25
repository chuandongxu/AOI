#include "SetFiducialMarkDialog.h"

SetFiducialMarkDialog::SetFiducialMarkDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
}

SetFiducialMarkDialog::~SetFiducialMarkDialog()
{
}

bool SetFiducialMarkDialog::getUseStandardShape()
{
    return ui.rbUseStandardShape->isChecked();
}

float SetFiducialMarkDialog::getFiducialMarkSize()
{
    return ui.lineEditSize->text().toFloat();
}

bool SetFiducialMarkDialog::getIsFiducialMarkDark()
{
    return ui.checkBoxDark->isChecked();
}

Vision::PR_FIDUCIAL_MARK_TYPE SetFiducialMarkDialog::getFiducialMarkShape()
{
    return static_cast<Vision::PR_FIDUCIAL_MARK_TYPE> ( ui.comboBoxShape->currentIndex() );
}