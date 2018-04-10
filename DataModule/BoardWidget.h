#pragma once

#include <QWidget>
#include "ui_BoardWidget.h"

class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    BoardWidget(QWidget *parent = Q_NULLPTR);
    ~BoardWidget();

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void on_btnTopLeft_clicked();
    void on_btnBtmRight_clicked();

    void _displayResult();
private:
    Ui::BoardWidget ui;

    double      m_dLeftX;
    double      m_dTopY;
    double      m_dRightX;
    double      m_dBottomY;
};
