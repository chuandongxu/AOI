#pragma once

#include <QWidget>
#include "ui_HeightDetectWidget.h"

#include "EditInspWindowBaseWidget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <qlistview.h>
#include <QStandardItemModel>
#include <memory>

#include "opencv/cv.h"

class HeightDetectWidget : public EditInspWindowBaseWidget
{
	Q_OBJECT

public:
	HeightDetectWidget(InspWindowWidget *parent = Q_NULLPTR);
	~HeightDetectWidget();
	virtual void setDefaultValue() override;
	virtual void setCurrentWindow(const Engine::Window &window) override;
	virtual void tryInsp() override;
	virtual void confirmWindow(OPERATION enOperation) override;

private:
	Ui::HeightDetectWidget ui;

	std::unique_ptr<QCheckBox>  m_pCheckBoxMeasure;
	std::unique_ptr<QLineEdit>  m_pEditMinRange;
	std::unique_ptr<QLineEdit>  m_pEditMaxRange;
};
