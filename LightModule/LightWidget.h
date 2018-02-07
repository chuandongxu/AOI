#pragma once

#include <QWidget>
#include "ui_LightWidget.h"

class QLightCtrl;
class LightWidget : public QWidget
{
	Q_OBJECT

public:
	LightWidget(QLightCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~LightWidget();

public:
	void setLight();

private:
	void setLightValue(int chn, int value);

public slots:
	void onLightModeIndexChanged(int index);

	void onSliderChanged1(int i);
	void onSliderChanged2(int i);
	void onSliderChanged3(int i);
	void onSliderChanged4(int i);
	void onSliderChanged5(int i);
	void onSliderChanged6(int i);
	void onSliderChanged7(int i);

private:
	Ui::LightWidget ui;
	QLightCtrl* m_pCtrl;
};
