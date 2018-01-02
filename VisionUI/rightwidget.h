#pragma once

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "ui_rightwidget.h"

class QRightWidget : public QWidget
{
	Q_OBJECT

public:
	QRightWidget(QWidget *parent = Q_NULLPTR);
	~QRightWidget();

protected:
	void paintEvent(QPaintEvent *event);

public slots:
	void onRunState(const QVariantList &data);

private:
	Ui::rightwidget ui;

protected:
	QWidget * m_widgetRun;
	QWidget * m_widgetEdit;

	QHBoxLayout * m_subVLayout;
	QVBoxLayout * m_mainLayout;
};
