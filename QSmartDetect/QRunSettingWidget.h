#pragma once

#include <QWidget>
#include "ui_QRunSettingWidget.h"
#include "DataCtrl.h"

#include <qstandarditemmodel>

#include "opencv/cv.h"

using namespace cv;

class QRunSettingWidget : public QWidget
{
	Q_OBJECT

public:
	QRunSettingWidget(DataCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~QRunSettingWidget();

private:
	void initObjList();

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void mousePressEvent(QMouseEvent *event);

private slots:
	void slotRowDoubleClicked(const QModelIndex &);
	void onObjEvent(const QVariantList &data);

	void onRunTypeIndexChanged(int iIndex);
	void onRunModeIndexChanged(int iIndex);

	void on3DDetectOpen();
	void onCellEditor();
	void onProfileEditor();

private:
	void loadTmps();
	void loadObjs();

	void loadTemplate(QWidget* pWidget, int nType);

private:
	bool convertToGrayImage(QString& szFilePath, cv::Mat &matGray);

private:
	Ui::QRunSettingWidget ui;
	DataCtrl* m_pCtrl;
	QStandardItemModel m_model;
};
