#pragma once

#include <QWidget>
#include "ui_VLCellEditor.h"

#include "../include/IData.h"

#include "opencv/cv.h"
#include <qstandarditemmodel>

using namespace cv;

class QColorWeight;
class IVisionUI;
class QVLMaskEditor;
class QVLCellEditor : public QWidget
{
	Q_OBJECT

public:
	QVLCellEditor(DataTypeEnum emType, QWidget *parent = Q_NULLPTR);
	~QVLCellEditor();

private:
	IVisionUI* getVisionUI();

protected:
	void closeEvent(QCloseEvent *e);

public:
	void setImage(cv::Mat& matImage);

protected slots:
	void onCellTypeChanged(int iIndex);
	void onCellIndexChanged(int iIndex);
	void onAddCell();
	void onDeleteCell();
	void onSelectCellBitmap();

	void onEditCellROI();
	void onEditCellFrame();
	void onEditCellLocFrame();
	void onEditCellLocLearn();

	void onEditCellBaseFrame();
	void onEditCellDetectFrame();

	void onSearchCell();

	void onCellDetectIndexChanged(int iIndex);
	void onCellBaseIndexChanged(int iIndex);

	void onAddCellRelation();
	void onDeleteCellRelation();

	void onLoadConfigFile();
	void onSaveConfigFile();

	void onAlignmentIndexChanged(int iIndex);
	void onAddBoardAlign();
	void onDeleteBoardAlign();

public:
	void loadConfigData(int nIndex = 0);

private:
	void initValue();
	void displayObj();
	void displayAllObjs();

	void updateDBMenu();
	void refreshDBRelation();

	void saveMapData();
	void loadMapData();

	void saveDataBase();
	void loadDataBase();

private:
	Ui::QVLCellEditor ui;
	QGraphicsScene * m_BitmapScene;

	QStandardItemModel m_model;

	QVLMaskEditor *m_pVLMaskEditor;

	QDetectObj* m_curObj;
	int m_nObjIndex;

	DataTypeEnum m_dataTypeEnum;
};
