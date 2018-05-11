#pragma once

#include <QWidget>
#include "ui_AutoRunPanel.h"

#include <qstandarditemmodel>

class AutoRunPanel : public QWidget
{
    Q_OBJECT

public:
    AutoRunPanel(QWidget *parent = Q_NULLPTR);
    ~AutoRunPanel();

private:
    void updateResultList();

protected slots:
    void onStateEvent(const QVariantList &data);
    void onResultEvent(const QVariantList &data);

    void onStateChangeAutoRun(int iState);
    void onRunOnce();

    void onStartAutoRun();
    void onStopAutoRun();

private:
    Ui::AutoRunPanel ui;

    QStandardItemModel m_model;

    bool m_bAutoRunning;
};
