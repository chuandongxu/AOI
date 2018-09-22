#pragma once

#include <QDialog>
#include "ui_QCaliGuideDialog.h"

class QCaliGuideDialog : public QDialog
{
    Q_OBJECT

public:
    QCaliGuideDialog(QWidget *parent = Q_NULLPTR);
    ~QCaliGuideDialog();

protected slots:
    void onOk();
    void onExit();

private:
    Ui::QCaliGuideDialog ui;
};
