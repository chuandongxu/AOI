#pragma once

#include <QMainWindow>
#include "ui_VisionDetectSetting.h"
#include "VisionCtrl.h"

class VisionDetectSetting : public QMainWindow
{
	Q_OBJECT

public:
	VisionDetectSetting(VisionCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~VisionDetectSetting();

private:
	Ui::VisionDetectSetting ui;	
	VisionCtrl* m_pCtrl;
};
