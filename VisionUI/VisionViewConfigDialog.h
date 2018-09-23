#ifndef VISIONVIEWCONFIGDIALOG_H
#define VISIONVIEWCONFIGDIALOG_H

#include <QDialog>
#include "ui_VisionViewConfigDialog.h"
#include "VisionViewWidget.h"

class VisionViewConfigDialog : public QDialog
{
    Q_OBJECT

public:
    VisionViewConfigDialog(VisionViewWidget *pVisionViewWidget, QWidget *parent = 0);
    ~VisionViewConfigDialog();

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void onCheckBoxDisplayDeviceWindow(int iState);
    void onCheckBoxDisplayInspectWindow(int iState);

private:
    Ui::VisionViewConfigDialog ui;
    VisionViewWidget *m_pVisionViewWidget;
};

#endif // VISIONVIEWCONFIGDIALOG_H
