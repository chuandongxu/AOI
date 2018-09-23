#pragma once

#include <QWidget>
#include "ui_DataSetting.h"
#include "DataCtrl.h"

class DataSetting : public QWidget
{
    Q_OBJECT

public:
    DataSetting(DataCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
    ~DataSetting();

private:
    Ui::DataSetting ui;
    DataCtrl* m_pCtrl;
};
