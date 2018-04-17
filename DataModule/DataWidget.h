#pragma once

#include <QWidget>
#include "ui_DataWidget.h"
#include "DataCtrl.h"

class DataWidget : public QWidget
{
    Q_OBJECT

public:
    DataWidget(DataCtrl *pDataCtrl, QWidget *parent = Q_NULLPTR);
    ~DataWidget();

private:
    Ui::DataWidget ui;
};
