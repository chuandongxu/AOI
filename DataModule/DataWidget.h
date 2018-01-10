#pragma once

#include <QWidget>
#include "ui_DataWidget.h"

class DataWidget : public QWidget
{
    Q_OBJECT

public:
    DataWidget(QWidget *parent = Q_NULLPTR);
    ~DataWidget();

private:
    Ui::DataWidget ui;
};
