#pragma once

#include <QWidget>
#include "ui_ImportCAD.h"
#include <vector>
#include "DataUtils.h"

class ImportCADWidget : public QWidget
{
    Q_OBJECT

public:
    ImportCADWidget(QWidget *parent = Q_NULLPTR);
    ~ImportCADWidget();

private slots:
    void on_btnSelectFile_clicked();
    void on_btnSelectColumn_clicked();
    void on_btnMoveUpSelection_clicked();
    void on_btnMoveDownSelection_clicked();
    void on_btnDeleteSelection_clicked();
    void on_btnImportCAD_clicked();
private:
    Ui::ImportCAD ui;
    std::vector<CAD_DATA_COLUMNS> _vecCadDataColumns;
    QString _strCADFilePath;
};
