#pragma once

#include <QWidget>
#include "ui_ScanImageWidget.h"

class ScanImageWidget : public QWidget
{
    Q_OBJECT

public:
    ScanImageWidget(QWidget *parent = Q_NULLPTR);
    ~ScanImageWidget();

private slots:
    void on_btnSelectCombinedImage_clicked();

private:
    Ui::ScanImageWidget ui;
};
