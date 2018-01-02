#ifndef ATTCHWIDGET_H
#define ATTCHWIDGET_H

#include <QWidget>
#include <QHBoxLayout>

class QAttchWidget : public QWidget
{
	Q_OBJECT

public:
	QAttchWidget(QWidget * w,QWidget *parent = NULL);
	~QAttchWidget();

private:
	QHBoxLayout * m_pLayout;
	QWidget * m_attWidget;
};

#endif // ATTCHWIDGET_H
