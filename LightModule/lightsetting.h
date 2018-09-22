#ifndef LIGHTSETTING_H
#define LIGHTSETTING_H

#include <QWidget>
#include "ui_lightsetting.h"
#include "DeviceForm.h"
#include "lightctrl.h"

class QLightSetting : public QWidget
{
    Q_OBJECT

public:
    QLightSetting(QLightCtrl * pCtrl,QWidget *parent = 0);
    ~QLightSetting();

protected slots:
    void onAddLightCtrl();
    void onDelLightCtrl();

private:
    //QList<DeviceForm*> m_forms;
    Ui::QLightSetting ui;
    QLightCtrl * m_lightCtrl;
};

#endif // LIGHTSETTING_H
