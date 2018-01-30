#include "LightWidget.h"
#include "lightctrl.h"
#include "LightDefine.h"
#include "SystemData.h"

LightWidget::LightWidget(QLightCtrl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	QLineEdit * editCtrls[6] = { ui.lineEdit_1_name, ui.lineEdit_2_name, ui.lineEdit_3_name, ui.lineEdit_4_name, ui.lineEdit_5_name, ui.lineEdit_6_name };
	QSlider * sliderCtrls[6] = { ui.horizontalSlider_1, ui.horizontalSlider_2, ui.horizontalSlider_3, ui.horizontalSlider_4, ui.horizontalSlider_5, ui.horizontalSlider_6 };
	QLineEdit * editLums[6] = { ui.lineEdit_1, ui.lineEdit_2, ui.lineEdit_3, ui.lineEdit_4, ui.lineEdit_5, ui.lineEdit_6 };

	QLightDevice * device = m_pCtrl->getLightDevice(0);
	if (device)
	{
		for (int i = 0; i < 4; i++)
		{
			QString key = QString("%0-%1%2").arg(device->getDeviceName()).arg(NAMED_CH).arg(i);
			QVariant data = System->getParam(key);
			editCtrls[i]->setText(data.toString());

			key = QString("%0-%1%2").arg(device->getDeviceName()).arg(LIGHT_INT_CH).arg(i);
			data = System->getParam(key);
			int nLightInt = data.toInt();
			sliderCtrls[i]->setValue(nLightInt);
			editLums[i]->setText(data.toString());

			key = QString("%0-%1%2").arg(device->getDeviceName()).arg(LUM_CH).arg(i);
			data = System->getParam(key);
			device->setChLuminance(i, data.toInt() * nLightInt / 100.0);
		}
	}

	device = m_pCtrl->getLightDevice(1);
	if (device)
	{
		for (int i = 0; i < 2; i++)
		{
			QString key = QString("%0-%1%2").arg(device->getDeviceName()).arg(NAMED_CH).arg(i);
			QVariant data = System->getParam(key);
			editCtrls[i + 4]->setText(data.toString());

			key = QString("%0-%1%2").arg(device->getDeviceName()).arg(LIGHT_INT_CH).arg(i);
			data = System->getParam(key);
			int nLightInt = data.toInt();
			sliderCtrls[i + 4]->setValue(nLightInt);
			editLums[i + 4]->setText(data.toString());

			key = QString("%0-%1%2").arg(device->getDeviceName()).arg(LUM_CH).arg(i);
			data = System->getParam(key);
			device->setChLuminance(i, data.toInt() * nLightInt / 100.0);
		}
	}

	double dLightExposure = System->getParam("motion_trigger_light_exposure").toDouble();	
	ui.horizontalSlider_7->setValue(dLightExposure);
	ui.lineEdit_7->setText(QString("%1").arg(dLightExposure));

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

void LightWidget::setLight()
{
	QLightDevice * device = m_pCtrl->getLightDevice(0);
	if (device)
	{
		for (int i = 0; i < 4; i++)
		{
			QString key = QString("%0-%1%2").arg(device->getDeviceName()).arg(LIGHT_INT_CH).arg(i);
			QVariant data = System->getParam(key);
			int nLightInt = data.toInt();

			key = QString("%0-%1%2").arg(device->getDeviceName()).arg(LUM_CH).arg(i);
			data = System->getParam(key);
			device->setChLuminance(i, data.toInt() * nLightInt / 100.0);
		}
	}

	device = m_pCtrl->getLightDevice(1);
	if (device)
	{
		for (int i = 0; i < 2; i++)
		{
			QString key = QString("%0-%1%2").arg(device->getDeviceName()).arg(LIGHT_INT_CH).arg(i);
			QVariant data = System->getParam(key);
			int nLightInt = data.toInt();

			key = QString("%0-%1%2").arg(device->getDeviceName()).arg(LUM_CH).arg(i);
			data = System->getParam(key);
			device->setChLuminance(i, data.toInt() * nLightInt / 100.0);
		}
	}
}

void LightWidget::setLightValue(int chn, int value)
{
	int i = 0;

	QLightDevice * device = NULL;
	if (chn >= 0 && chn < 4)
	{
		device = m_pCtrl->getLightDevice(0);
		i = chn;
	}		
	else if (chn >= 4 && chn < 8)
	{
		i = chn - 4;
		device = m_pCtrl->getLightDevice(1);
	}

	if (device)
	{
		QString key = QString("%0-%1%2").arg(device->getDeviceName()).arg(LIGHT_INT_CH).arg(i);
		QVariant data = value;
		System->setParam(key, data);

		key = QString("%0-%1%2").arg(device->getDeviceName()).arg(LUM_CH).arg(i);
		data = System->getParam(key);
		device->setChLuminance(i, data.toInt() * value / 100.0);
	}
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
	QString str = QString::number(lum);
	ui.lineEdit_7->setText(str);

	System->setParam("motion_trigger_light_exposure", lum);
}

