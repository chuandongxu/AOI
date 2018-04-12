#pragma once

#include <QWidget>
#include "ui_TableCalibrationView.h"
#include "VisionCtrl.h"

class TableCalibrationView : public QWidget
{
	Q_OBJECT

public:
	TableCalibrationView(VisionCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~TableCalibrationView();

private:
	void initUI();

private:
	Ui::TableCalibrationView ui;
	VisionCtrl* m_pCtrl;
};
