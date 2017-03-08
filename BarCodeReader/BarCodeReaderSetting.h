#ifndef BARCODEREADERSETTING_H
#define BARCODEREADERSETTING_H

#include <QWidget>
#include <qstandarditemmodel.h>
#include "ui_BarCodeReaderSetting.h"
#include "BarCodeDevMgr.h"
#include <QItemDelegate>


class QBarSettingDelegate : public QItemDelegate
{
public:
	virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
	virtual void	setEditorData(QWidget * editor, const QModelIndex & index) const;
	virtual void	setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;
};

class QBarCodeReaderSetting : public QWidget
{
	Q_OBJECT

public:
	QBarCodeReaderSetting(QBarCodeDevMgr * pDevMgr,QWidget *parent = 0);
	~QBarCodeReaderSetting();

protected slots:
	void addDevice();
	void removeDevice();

	void openDevice();
	void closeDevice();

	void readBarCode();
	void save();
	void onChangeLange();

protected:
	void loadData();
	//QString tr(const QString & str);

private:
	Ui::QBarCodeReaderSetting ui;
	QBarCodeDevMgr * m_pBarCodeDevMgr;
	QStandardItemModel m_model;
	QBarSettingDelegate m_delegate;
};

#endif // BARCODEREADERSETTING_H
