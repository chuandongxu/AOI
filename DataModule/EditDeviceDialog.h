#ifndef EDITDEVICEDIALOG_H
#define EDITDEVICEDIALOG_H

#include <QDialog>
#include "ui_EditDeviceDialog.h"

class EditDeviceDialog : public QDialog
{
    Q_OBJECT

public:
    EditDeviceDialog(QWidget *parent = 0);
    ~EditDeviceDialog();
    void getDeviceInfo(std::string& name, std::string& package, std::string& type) const;
    void setDeviceInfo(const std::string& name, const std::string& package, const std::string& type);

private slots:
    void on_accept();

private:
    Ui::EditDeviceDialog ui;
};

#endif // EDITDEVICEDIALOG_H
