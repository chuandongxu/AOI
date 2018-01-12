#pragma once

#include <QWidget>
#include "ui_FiducialMarkWidget.h"

class FiducialMarkWidget : public QWidget
{
    Q_OBJECT

public:
    FiducialMarkWidget(QWidget *parent = Q_NULLPTR);
    ~FiducialMarkWidget();

private slots:
    void on_btnAdjustCAD_clicked();
    void on_btnAddFiducialMark_clicked();
private:
    Ui::FiducialMarkWidget ui;
};
