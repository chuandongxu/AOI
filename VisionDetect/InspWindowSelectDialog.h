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
	void on_btnAlignmentWindow_clicked();
	void on_btnHeightDetectWindow_clicked();
    void on_btnGlobalWindow_clicked();
    void on_btnInspPolarityWindow_clicked();
    void on_btnInspContourWindow_clicked();
    void on_btnInspChipWindow_clicked();
    void on_btnInspLeadWindow_clicked();
    void on_btnInspBridgeWindow_clicked();
    void on_btnOcvWindow_clicked();
    void on_btn3DSolderWindow_clicked();
    void on_accept();

private:
    Ui::InspWindowSelectDialog ui;

    INSP_WIDGET_INDEX       m_enInspWidgetIndex;
};
