#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <QWidget>
#include "ui_TimeWidget.h"

class QTimeWidget : public QWidget
{
    Q_OBJECT

public:
    QTimeWidget(QWidget *parent = 0);
    ~QTimeWidget();

private:
    Ui::QTimeWidget ui;
};

#endif // TIMEWIDGET_H
