#pragma once

#include <QWidget>
#include "ui_InspWindowSelectWidget.h"
#include "InspWindowWidget.h"

class InspWindowWidget;

class InspWindowSelectWidget : public QWidget
{
    Q_OBJECT

public:
    InspWindowSelectWidget(InspWindowWidget *parent = Q_NULLPTR);
    ~InspWindowSelectWidget();

private slots:
    void on_btnFindLineWindow_clicked();
    void on_btnCaliperCircleWindow_clicked();
    void on_btnVoidInspWindow_clicked();
	void on_btnAlignmentWindow_clicked();
    
private:
    Ui::InspWindowSelectWidget ui;

    InspWindowWidget *m_pParent;
};
