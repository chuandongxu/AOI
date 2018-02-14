#pragma once

#include <QWidget>
#include <QComboBox>
#include "ui_InspVoidWidget.h"

class InspVoidWidget : public QWidget
{
    Q_OBJECT

public:
    InspVoidWidget(QWidget *parent = Q_NULLPTR);
    ~InspVoidWidget();
    
private slots:    
    void on_inspModeChanged(int index);
    void on_btnTryInsp_clicked();
    void on_btnConfirmWindow_clicked();

private:
    Ui::InspVoidWidget ui;
    QComboBox           *m_pComboBoxInspMode;
};
