#ifndef CENTERWIDGET_H
#define CENTERWIDGET_H

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "imagewidget.h"
#include "statewidget.h"
//#include "workflowWidget.h"

class QCenterWidget : public QFrame
{
    Q_OBJECT
public:
    explicit QCenterWidget(QWidget *parent = 0);

	void setCenterWidget(QWidget * w);

protected:
	QWidget * m_widget;

	QHBoxLayout * m_subVLayout;
    QVBoxLayout * m_mainLayout;
};

#endif // CENTERWIDGET_H
