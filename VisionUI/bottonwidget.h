#ifndef BOTTONWIDGET_H
#define BOTTONWIDGET_H

#include <QWidget>
#include "ui_bottonwidget.h"
#include <qstandarditemmodel>
#include <QResizeEvent>

/*
class QBottomModel : public QStandardItemModel
{
	Q_OBJECT
public:
	QBottomModel(){};
	
public slots:
	void onErrorInfo(const QString &data,const QString &msg,unsigned int level);
};
*/

class QBottonWidget : public QWidget
{
	Q_OBJECT

public:
	QBottonWidget(QWidget *parent = NULL);
	~QBottonWidget();

	//static void initBottomModel();

protected slots:
	void onResoultEvent(const QVariantList &data);
	void onChangeModuleType(const QVariantList &data);
	void onDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles);

protected:
	void checkInitState();
	void resizeEvent(QResizeEvent * event);
private:
	Ui::bottonwidget ui;
	QStandardItemModel m_checkModel;

private:
	//static QBottomModel m_model;
};

#endif // BOTTONWIDGET_H
