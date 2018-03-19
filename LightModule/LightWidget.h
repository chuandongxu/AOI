#pragma once

#include <QWidget>
#include "ui_LightWidget.h"
#include "DataStoreStruct.h"

class QLightCtrl;
class LightWidget : public QWidget
{
	Q_OBJECT

public:
	LightWidget(QLightCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~LightWidget();

public:
	void setLight(int nLight);

private:
	void setLightValue(int chn, int value);

	void loadConfig();
	void saveConfig();
	void updateUI();

public slots:
	void onLightModeIndexChanged(int index);
	void onSaveParams();
	void onLoadParams();

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
	NFG::AOI::Engine::LightVector m_vecLights;
};
