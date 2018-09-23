#include "LightWidget.h"
#include "lightctrl.h"
#include "LightDefine.h"
#include "SystemData.h"
#include "../include/IdDefine.h"
#include "../Common/eos.h"

#include "DataStoreAPI.h"

using namespace NFG::AOI;

LightWidget::LightWidget(QLightCtrl* pCtrl, QWidget *parent)
    : m_pCtrl(pCtrl), QWidget(parent)
{
    ui.setupUi(this);

    connect(ui.comboBox_selectLightMode, SIGNAL(currentIndexChanged(int)), SLOT(onLightModeIndexChanged(int)));
    QStringList ls;
    ls << QStringLiteral("白光") << QStringLiteral("低角度光") << QStringLiteral("彩色光") << QStringLiteral("均匀光") << QStringLiteral("3D灰阶图");
    ui.comboBox_selectLightMode->addItems(ls);

    QLineEdit * editCtrls[6] = { ui.lineEdit_1_name, ui.lineEdit_2_name, ui.lineEdit_3_name, ui.lineEdit_4_name, ui.lineEdit_5_name, ui.lineEdit_6_name };

    for (int i = 0; i < 6; i++)
    {
        QString name = m_pCtrl->getChName(i);
        editCtrls[i]->setText(name);

    }        

    connect(ui.pushButton_saveParams, SIGNAL(clicked()), SLOT(onSaveParams()));
    connect(ui.pushButton_loadParams, SIGNAL(clicked()), SLOT(onLoadParams()));

    connect(ui.horizontalSlider_1, SIGNAL(valueChanged(int)), SLOT(onSliderChanged1(int)));
    connect(ui.horizontalSlider_2, SIGNAL(valueChanged(int)), SLOT(onSliderChanged2(int)));
    connect(ui.horizontalSlider_3, SIGNAL(valueChanged(int)), SLOT(onSliderChanged3(int)));
    connect(ui.horizontalSlider_4, SIGNAL(valueChanged(int)), SLOT(onSliderChanged4(int)));
    connect(ui.horizontalSlider_5, SIGNAL(valueChanged(int)), SLOT(onSliderChanged5(int)));
    connect(ui.horizontalSlider_6, SIGNAL(valueChanged(int)), SLOT(onSliderChanged6(int)));    
    connect(ui.horizontalSlider_7, SIGNAL(valueChanged(int)), SLOT(onSliderChanged7(int)));    
}

LightWidget::~LightWidget()
{
}

void LightWidget::init()
{
    loadConfig();
    //updateUI();
}

void LightWidget::startUpLight()
{
    if (m_vecLights.size() <= 0) return;
    Engine::Light light = m_vecLights[0];

    for (int i = 0; i < 6; i++)
    {
        int nLightInt = light.vecLightIntensity[i];

        double dLumRate = nLightInt / 100.0;
        int nLum = m_pCtrl->getChLuminace(i);
        m_pCtrl->setLuminance(i, nLum * dLumRate);
    }
}

void LightWidget::endUpLight()
{
    for (int i = 0; i < 6; i++)
    {      
        int nLum = m_pCtrl->getChLuminace(i);
        m_pCtrl->setLuminance(i, nLum);
    }
}

void LightWidget::onLightModeIndexChanged(int index)
{
    int nIndex = ui.comboBox_selectLightMode->currentIndex();

    updateUI();

    QEos::Notify(EVENT_IMAGE_STATE, 0, IMAGE_STATE_CHANGE, nIndex);
}

void LightWidget::onSaveParams()
{
    saveConfig();
}

void LightWidget::onLoadParams()
{
    loadConfig();
    updateUI();
}

void LightWidget::setLightValue(int chn, int value)
{
    if (m_vecLights.size() <= 0) return;

    Engine::Light& light = m_vecLights[0];
    light.vecLightIntensity[chn] = value;

    double dLumRate = value / 100.0;
    int nLum = m_pCtrl->getChLuminace(chn);
    m_pCtrl->setLuminance(chn, nLum * dLumRate);

    //m_pCtrl->saveLuminance(chn);
}

void LightWidget::onSliderChanged1(int lum)
{
    QString str = QString::number(lum);
    ui.lineEdit_1->setText(str);

    setLightValue(0, lum);
}

void LightWidget::onSliderChanged2(int lum)
{
    QString str = QString::number(lum);
    ui.lineEdit_2->setText(str);

    setLightValue(1, lum);
}

void LightWidget::onSliderChanged3(int lum)
{
    QString str = QString::number(lum);
    ui.lineEdit_3->setText(str);

    setLightValue(2, lum);
}

void LightWidget::onSliderChanged4(int lum)
{
    QString str = QString::number(lum);
    ui.lineEdit_4->setText(str);

    setLightValue(3, lum);
}

void LightWidget::onSliderChanged5(int lum)
{
    QString str = QString::number(lum);
    ui.lineEdit_5->setText(str);

    setLightValue(4, lum);
}

void LightWidget::onSliderChanged6(int lum)
{
    QString str = QString::number(lum);
    ui.lineEdit_6->setText(str);

    setLightValue(5, lum);
}

void LightWidget::onSliderChanged7(int lum)
{
    if (m_vecLights.size() <= 0) return;

    QString str = QString::number(lum);
    ui.lineEdit_7->setText(str);
    
    Engine::Light& light = m_vecLights[0];
    light.expTime = lum;
}

void LightWidget::loadConfig()
{
    m_vecLights.clear();
    auto result = Engine::GetLights(m_vecLights);
    if (result != Engine::OK) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        System->setTrackInfo(QString("Error at GetLights, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
        return;
    }
}

void LightWidget::saveConfig()
{
    auto result = Engine::SetLights(m_vecLights);
    if (result != Engine::OK) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        System->setTrackInfo(QString("Error at SetLights, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
        return;
    }
    System->setTrackInfo(QString("Success save lights..."));
}

void LightWidget::updateUI()
{
    if (m_vecLights.size() <= 0) return;

    Engine::Light light = m_vecLights[0];

    QSlider * sliderCtrls[6] = { ui.horizontalSlider_1, ui.horizontalSlider_2, ui.horizontalSlider_3, ui.horizontalSlider_4, ui.horizontalSlider_5, ui.horizontalSlider_6 };
    QLineEdit * editLums[6] = { ui.lineEdit_1, ui.lineEdit_2, ui.lineEdit_3, ui.lineEdit_4, ui.lineEdit_5, ui.lineEdit_6 };

    for (int i = 0; i < 6; i++)
    {
        int nLightInt = light.vecLightIntensity[i];
        sliderCtrls[i]->setValue(nLightInt);
        editLums[i]->setText(QString("%1").arg(nLightInt));
    }

    double dLightExposure = light.expTime;
    ui.horizontalSlider_7->setValue(dLightExposure);
    ui.lineEdit_7->setText(QString("%1").arg(dLightExposure));
}

