#pragma once

#include <QWidget>
#include "ui_AlignmentWidget.h"

#include "EditInspWindowBaseWidget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <memory>

class AlignmentWidget : public EditInspWindowBaseWidget
{
	Q_OBJECT

public:
	AlignmentWidget(InspWindowWidget *parent = Q_NULLPTR);
	~AlignmentWidget();
	virtual void setDefaultValue() override;
	virtual void setCurrentWindow(const Engine::Window &window) override;
	virtual void tryInsp() override;
	virtual void confirmWindow(OPERATION enOperation) override;

private:
    bool _learnTemplate(int &recordId);
    bool _srchTemplate(int recordId, bool bShowResult = true);

private:
	Ui::AlignmentWidget         ui;

	std::unique_ptr<QComboBox>	m_pComboBoxAlgorithm;
	std::unique_ptr<QLineEdit>  m_pEditRecordID;
	std::unique_ptr<QCheckBox>  m_pCheckBoxSubPixel;
	std::unique_ptr<QComboBox>	m_pComboBoxMotion;
	std::unique_ptr<QLineEdit>  m_pEditMinScore;
    bool                        m_bIsTryInspected = false;
};
