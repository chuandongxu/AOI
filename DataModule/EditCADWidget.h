#ifndef EDITCAD_H
#define EDITCAD_H

#include <QWidget>
#include "ui_EditCADWidget.h"
#include "DataCtrl.h"

class EditCADWidget : public QWidget
{
    Q_OBJECT

public:
    EditCADWidget(DataCtrl *pDataCtrl, QWidget *parent = 0);
    ~EditCADWidget();

private slots:
    void on_btnAddDevice_clicked();
    void on_btnEditDevice_clicked();
    void on_btnDeleteDevice_clicked();

private:
    Ui::EditCAD ui;
    DataCtrl                   *m_pDataCtrl;
};

#endif // EDITCAD_H
