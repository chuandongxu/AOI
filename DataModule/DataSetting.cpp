#include "DataSetting.h"

DataSetting::DataSetting(DataCtrl* pCtrl, QWidget *parent)
    : m_pCtrl(pCtrl), QWidget(parent)
{
    ui.setupUi(this);
}

DataSetting::~DataSetting()
{
}
