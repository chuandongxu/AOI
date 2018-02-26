#pragma once

#include <QWidget>
#include <QComboBox>
#include <memory>
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
    Ui::InspVoidWidget          ui;
    std::unique_ptr<QComboBox>  m_pComboBoxInspMode;
    std::unique_ptr<QLineEdit>  m_pEditMaxAreaRatio;
    std::unique_ptr<QLineEdit>  m_pEditMinAreaRatio;
    std::unique_ptr<QLineEdit>  m_pEditMaxHoleCount;
    std::unique_ptr<QLineEdit>  m_pEditMinHoleCount;
    std::unique_ptr<QLineEdit>  m_pEditMaxHoleArea;
    std::unique_ptr<QLineEdit>  m_pEditMinHoleArea;
};
