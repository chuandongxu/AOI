
#include <QMessageBox>

#include "EditDeviceDialog.h"

EditDeviceDialog::EditDeviceDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(on_accept()));
}

EditDeviceDialog::~EditDeviceDialog() {
}

void EditDeviceDialog::on_accept() {
    if (ui.lineEditName->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", QStringLiteral("请输入元件名称!"));
        return;
    }
    accept();
}

void EditDeviceDialog::getDeviceInfo(std::string& name, std::string& package, std::string& type) const {
    name = ui.lineEditName->text().toStdString();
    package = ui.lineEditPackage->text().toStdString();
    type = ui.lineEditType->text().toStdString();
}

void EditDeviceDialog::setDeviceInfo(const std::string& name, const std::string& package, const std::string& type) {
    ui.lineEditName->setText(name.c_str());
    ui.lineEditPackage->setText(package.c_str());
    ui.lineEditType->setText(type.c_str());
}
