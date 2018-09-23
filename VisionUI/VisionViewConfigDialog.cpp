#include "VisionViewConfigDialog.h"

VisionViewConfigDialog::VisionViewConfigDialog(VisionViewWidget *pVisionViewWidget, QWidget *parent)
    : m_pVisionViewWidget(pVisionViewWidget), QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.checkBoxDisplayDeviceWindow, SIGNAL(stateChanged(int)), SLOT(onCheckBoxDisplayDeviceWindow(int)));
    connect(ui.checkBoxDisplayInspectWindow, SIGNAL(stateChanged(int)), SLOT(onCheckBoxDisplayInspectWindow(int)));

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(size());
}

VisionViewConfigDialog::~VisionViewConfigDialog() {
}

void VisionViewConfigDialog::closeEvent(QCloseEvent *e) {
    this->hide();
}

void VisionViewConfigDialog::onCheckBoxDisplayDeviceWindow(int iState) {
    bool bDisplay = iState == Qt::Checked;
    m_pVisionViewWidget->setDisplayDeviceWindow(bDisplay);
}

void VisionViewConfigDialog::onCheckBoxDisplayInspectWindow(int iState) {
    bool bDisplay = iState == Qt::Checked;
    m_pVisionViewWidget->setDisplayInspectWindow(bDisplay);
}
