#pragma once

#include <QWidget>
#include "ui_DataEditor.h"

#include <memory>

class DataEditor : public QWidget
{
    Q_OBJECT

public:
    DataEditor(QWidget *parent = Q_NULLPTR);
    ~DataEditor();

    bool displayRecord(int recordID);

private:
    void initUI();

private:
    Ui::DataEditor ui;

    QGraphicsScene * m_recordImgScene;
};
