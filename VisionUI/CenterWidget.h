#ifndef CENTERWIDGET_H
#define CENTERWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "imagewidget.h"
#include "statewidget.h"
//#include "workflowWidget.h"
#include "rightwidget.h"

class QCenterWidget : public QFrame
{
    Q_OBJECT
public:
    explicit QCenterWidget(QWidget *parent = 0);

	void setCenterWidget(QWidget * w);
protected:
	void paintEvent(QPaintEvent *event);

protected:
	QWidget * m_widget;

	QHBoxLayout * m_subVLayout;
    QVBoxLayout * m_mainLayout;

	QRightWidget * m_rightWidget;
	QHBoxLayout * m_rightLayout;
};

#endif // CENTERWIDGET_H
