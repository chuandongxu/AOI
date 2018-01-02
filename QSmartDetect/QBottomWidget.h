#pragma once

#include <QWidget>
#include "ui_QBottomWidget.h"
#include <qstandarditemmodel>

class QBottomWidget : public QWidget
{
	Q_OBJECT

public:
	QBottomWidget(QWidget *parent = Q_NULLPTR);
	~QBottomWidget();

protected slots:
	void onDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles);

private:
	Ui::QBottomWidget ui;
};
