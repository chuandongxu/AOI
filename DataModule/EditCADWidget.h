#ifndef EDITCAD_H
#define EDITCAD_H

#include <QWidget>
#include "ui_EditCADWidget.h"

class EditCADWidget : public QWidget
{
    Q_OBJECT

public:
    EditCADWidget(QWidget *parent = 0);
    ~EditCADWidget();

private slots:
    void on_btnAddDevice_clicked();
    void on_btnEditDevice_clicked();
    void on_btnDeleteDevice_clicked();

private:
    Ui::EditCAD ui;
};

#endif // EDITCAD_H
