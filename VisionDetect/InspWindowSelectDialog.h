#pragma once

#include <QDialog>
#include "ui_InspWindowSelectDialog.h"
#include "InspWindowWidget.h"

class InspWindowSelectDialog : public QDialog
{
    Q_OBJECT

public:
    InspWindowSelectDialog(QWidget *parent = Q_NULLPTR);
    ~InspWindowSelectDialog();
    INSP_WIDGET_INDEX getWindowIndex() const { return m_enInspWidgetIndex; }

private slots:
    void on_btnFindLineWindow_clicked();
    void on_btnCaliperCircleWindow_clicked();
    void on_btnVoidInspWindow_clicked();
    void on_accept();

private:
    Ui::InspWindowSelectDialog ui;

    INSP_WIDGET_INDEX       m_enInspWidgetIndex;
};
