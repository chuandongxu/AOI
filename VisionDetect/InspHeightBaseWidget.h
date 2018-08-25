#pragma once

#include <QWidget>
#include "ui_InspHeightBaseWidget.h"

#include "EditInspWindowBaseWidget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <memory>

class InspHeightBaseWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    InspHeightBaseWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~InspHeightBaseWidget();

    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

private slots:
    void on_btnSelectROI_clicked();

private:
    Ui::InspHeightBaseWidget ui;

    std::unique_ptr<QLineEdit>  m_pEditColor;
    std::unique_ptr<QLineEdit>  m_pEditMinRange;
    std::unique_ptr<QLineEdit>  m_pEditMaxRange;
    std::unique_ptr<QLineEdit>  m_pEditRnParam;
    std::unique_ptr<QLineEdit>  m_pEditTnParam;

    cv::Vec3b m_color;
};
