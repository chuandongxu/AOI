#pragma once

#include "EditInspWindowBaseWidget.h"
#include "ui_InspWindowWidget.h"
#include "InspVoidWidget.h"
#include "DataStoreAPI.h"

using namespace NFG::AOI;

enum class INSP_WIDGET_INDEX {
    UNDEFINED = -1,
    FIND_LINE,
    INSP_HOLE,
    SIZE
};

const int INSP_WIDGET_INDEX_SIZE = static_cast<int>(INSP_WIDGET_INDEX::SIZE);

class InspWindowSelectWidget;

class InspWindowWidget : public QWidget
{
    Q_OBJECT

public:
    InspWindowWidget(QWidget *parent = Q_NULLPTR);
    ~InspWindowWidget();

    void setCurrentIndex(int index);
    void UpdateInspWindowList();
protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void on_btnAddWindow_clicked();
    void on_btnRemoveWindow_clicked();
    void on_btnTryInsp_clicked();
    void on_btnConfirmWindow_clicked();

    void onInspWindowState(const QVariantList &data);
    void onSelectedWindowChanged(int index);
private:
    Ui::InspWindowWidget ui;

    InspWindowBaseWidgetPtr     m_arrInspWindowWidget[INSP_WIDGET_INDEX_SIZE];
    Engine::WindowVector        m_vecCurrentDeviceWindows;
    OPERATION                   m_enOperation;
    INSP_WIDGET_INDEX           m_enCurrentInspWidget;
};
