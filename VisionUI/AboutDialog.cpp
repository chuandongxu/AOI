#include "AboutDialog.h"
#include "../include/Version.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    ui.lineEditProductSwVersion->setText(AOI_SW_VERSION);
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::setVisionLibraryVersion(const QString &strVersion)
{
    ui.lineEditVisionLibraryVersion->setText(strVersion);
}

void AboutDialog::setDataStoreApiVersion(const QString &strVersion)
{
    ui.lineEditDataStoreApiVersion->setText(strVersion);
}