#pragma once

#include <QWidget>
#include "ui_AutoStartUp.h"
#include <qstandarditemmodel>

class AutoStartUp : public QWidget
{
    Q_OBJECT

public:
    AutoStartUp(QWidget *parent = Q_NULLPTR);
    ~AutoStartUp();

    void showProgress();

protected slots:
    void onDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles);    

private:
    void insertMsg(const QString& msg, bool bOK);
    void initStartUp(int nStepCount);

private:
    Ui::AutoStartUp ui;
    QStandardItemModel m_dataModel;
    int m_nStepCount;
};
