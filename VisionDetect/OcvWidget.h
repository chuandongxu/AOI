#ifndef OCVWIDGET_H
#define OCVWIDGET_H

#include <QLineEdit>
#include <QComboBox>

#include "ui_OcvWidget.h"
#include "EditInspWindowBaseWidget.h"
#include "SpecAndResultWidget.h"

class OcvWidget : public EditInspWindowBaseWidget
{
    Q_OBJECT

public:
    OcvWidget(InspWindowWidget *parent = 0);
    ~OcvWidget();

    virtual void setDefaultValue() override;
    virtual void setCurrentWindow(const Engine::Window &window) override;
    virtual void tryInsp() override;
    virtual void confirmWindow(OPERATION enOperation) override;

private slots:
    void on_btnLrnOcv_clicked();
    void on_btnEditOcv_clicked();

private:
    bool _learnOcv(int &recordId);
    bool _inspOcv(const std::vector<Int32> &vecRecordId, bool bShowResult = true);

    Ui::OcvWidget ui;
    std::unique_ptr<QLineEdit>  m_pEditCharCount;
    SpecAndResultWidgetPtr      m_pSpecAndResultMinScore;
    std::unique_ptr<QComboBox>  m_pComboBoxCharDirection;
};

#endif // OCVWIDGET_H
