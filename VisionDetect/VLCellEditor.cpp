#include "VLCellEditor.h"
#include <QFileDialog>

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMath.h>
#include  <QMouseEvent>
#include  <QWheelEvent>
#include <QThread>
#include <QMessageBox>
#include <QBitmap>
#include "../lib/VisionLibrary/include/VisionAPI.h"

#include "../DataModule/QDetectObj.h"

#include "VLMaskEditor.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define ToInt(value)                (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))
using namespace AOI;

QVLCellEditor::QVLCellEditor(DataTypeEnum emType, QWidget *parent)
	: m_dataTypeEnum(emType), QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::Dialog);
	setWindowModality(Qt::ApplicationModal);

	m_pView = new QCellView(this);
	ui.verticalLayout->addWidget(m_pView);

	m_pVLMaskEditor = new QVLMaskEditor();
	m_curObj = NULL;
	m_nObjIndex = 0;

	ui.tableView_ItemDetAndBase->setModel(&m_model);

	m_BitmapScene = new QGraphicsScene();
	ui.graphicsView_cellBitmap->setScene(m_BitmapScene);
	ui.graphicsView_cellBitmap->setFixedSize(80, 60);
	ui.graphicsView_cellBitmap->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_cellBitmap->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QStringList ls;
	ls << QStringLiteral("标准组件") << QStringLiteral("管脚器件") << QStringLiteral("IC芯片") << QStringLiteral("LED元件");
	ui.comboBox_cellType->addItems(ls);
	ui.comboBox_cellType->setCurrentIndex(0);

	connect(ui.comboBox_cellType, SIGNAL(currentIndexChanged(int)), SLOT(onCellTypeChanged(int)));
	connect(ui.comboBox_cellIndex, SIGNAL(currentIndexChanged(int)), SLOT(onCellIndexChanged(int)));
	connect(ui.pushButton_addCell, SIGNAL(clicked()), SLOT(onAddCell()));
	connect(ui.pushButton_deleteCell, SIGNAL(clicked()), SLOT(onDeleteCell()));
	connect(ui.pushButton_selectCellBitmap, SIGNAL(clicked()), SLOT(onSelectCellBitmap()));

	connect(ui.pushButton_editCellROI, SIGNAL(clicked()), SLOT(onEditCellROI()));
	connect(ui.pushButton_editCellFrame, SIGNAL(clicked()), SLOT(onEditCellFrame()));
	connect(ui.pushButton_editCellLocFrame, SIGNAL(clicked()), SLOT(onEditCellLocFrame()));
	connect(ui.pushButton_editCellLocLearn, SIGNAL(clicked()), SLOT(onEditCellLocLearn()));
	connect(ui.pushButton_editCellBase, SIGNAL(clicked()), SLOT(onEditCellBaseFrame()));
	connect(ui.pushButton_editCellDetect, SIGNAL(clicked()), SLOT(onEditCellDetectFrame()));
	connect(ui.pushButton_searchCell, SIGNAL(clicked()), SLOT(onSearchCell()));

	connect(ui.comboBox_cellDetectIndex, SIGNAL(currentIndexChanged(int)), SLOT(onCellDetectIndexChanged(int)));
	connect(ui.comboBox_cellBaseIndex, SIGNAL(currentIndexChanged(int)), SLOT(onCellBaseIndexChanged(int)));
	connect(ui.pushButton_AddRelation, SIGNAL(clicked()), SLOT(onAddCellRelation()));
	connect(ui.pushButton_DeleteRelation, SIGNAL(clicked()), SLOT(onDeleteCellRelation()));

	connect(ui.pushButton_loadConfigFile, SIGNAL(clicked()), SLOT(onLoadConfigFile()));
	connect(ui.pushButton_saveConfigFile, SIGNAL(clicked()), SLOT(onSaveConfigFile()));

	connect(ui.comboBox_alignIndex, SIGNAL(currentIndexChanged(int)), SLOT(onAlignmentIndexChanged(int)));
	connect(ui.pushButton_addBoardAlign, SIGNAL(clicked()), SLOT(onAddBoardAlign()));
	connect(ui.pushButton_deleteBoardAlign, SIGNAL(clicked()), SLOT(onDeleteBoardAlign()));

	initValue();

	//loadMapData();
	loadDataBase();

	setMouseTracking(true);
	ui.checkBox_showNumber->setChecked(true);
}

QVLCellEditor::~QVLCellEditor()
{
	if (m_pView)
	{
		delete m_pView;
		m_pView = NULL;
	}

	if (m_pVLMaskEditor)
	{
		delete m_pVLMaskEditor;
		m_pVLMaskEditor = NULL;
	}	

	if (m_BitmapScene)
	{
		delete m_BitmapScene;
		m_BitmapScene = NULL;
	}
}

void QVLCellEditor::closeEvent(QCloseEvent *e){
	//qDebug() << "关闭事件";
	//e->ignore();

	this->hide();
}

void QVLCellEditor::loadConfigData(int nIndex)
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	initValue();

	//loadMapData();
	for (int i = 0; i < pData->getObjNum(m_dataTypeEnum); i++)
	{
		QDetectObj* pObj = pData->getObj(i, m_dataTypeEnum);
		if (pObj)
		{
			ui.comboBox_cellIndex->addItem(QString("%1").arg(pObj->getName()));
		}
	}

	ui.comboBox_cellIndex->setCurrentIndex(nIndex);


	ui.comboBox_alignIndex->clear();
	for (int i = 0; i < pData->getBoardObj()->getBoardAlignNum(); i++)
	{
		ui.comboBox_alignIndex->addItem(QString("%1").arg(i + 1));

	}
	ui.comboBox_alignIndex->setCurrentIndex(0);
}

void QVLCellEditor::initValue()
{
	ui.comboBox_cellIndex->clear();
	ui.comboBox_cellIndex->addItem(QString("%1").arg(QStringLiteral("全部显示")));

	refreshDBRelation();

}

void QVLCellEditor::setImage(cv::Mat& matImage)
{
	m_pView->setImage(matImage);
}

void QVLCellEditor::onCellTypeChanged(int iIndex)
{

}

void QVLCellEditor::onCellIndexChanged(int iIndex)
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	if (ui.comboBox_cellIndex->currentIndex() > 0)
	{
		int nIndex = ui.comboBox_cellIndex->currentIndex() - 1;

		QDetectObj* pObj = pData->getObj(nIndex, m_dataTypeEnum);
		m_curObj = pObj;
		ui.lineEdit_CellName->setText(pObj->getName());

		displayObj();
	}
	else
	{
		displayAllObjs();
	}

	m_pView->clearSelect();

	updateDBMenu();
	refreshDBRelation();
}

void QVLCellEditor::onAddCell()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	QString cellName = ui.lineEdit_CellName->text();

	if (cellName.isEmpty())
	{
		cellName = QString("Obj %1").arg(++m_nObjIndex);
	}
	else
	{
		for (int i = 0; i < pData->getObjNum(m_dataTypeEnum); i++)
		{
			if (cellName == pData->getObj(i, m_dataTypeEnum)->getName())
			{
				if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
					QStringLiteral("名称已经存在，是否重新命名？"), QMessageBox::Ok, QMessageBox::Cancel))
				{
					return;
				}
				else
				{
					cellName = QString("Obj %1").arg(++m_nObjIndex);
				}
			}
		}
	}	

	QDetectObj* pObj = new QDetectObj(-1, cellName);
	int nTypeIndex = ui.comboBox_cellType->currentIndex();
	pObj->setType(QString("%1").arg(nTypeIndex));
	pData->pushObj(pObj, m_dataTypeEnum);

	ui.comboBox_cellIndex->addItem(QString("%1").arg(cellName));

	ui.comboBox_cellIndex->setCurrentIndex(ui.comboBox_cellIndex->count() - 1);
}

void QVLCellEditor::onDeleteCell()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	if (ui.comboBox_cellIndex->currentIndex() > 0)
	{
		int nIndex = ui.comboBox_cellIndex->currentIndex() - 1;

		pData->deleteObj(nIndex, m_dataTypeEnum);

		m_curObj = NULL;
		ui.comboBox_cellIndex->removeItem(nIndex + 1);		
	}		
}

void QVLCellEditor::onSelectCellBitmap()
{
	if (!m_curObj)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Device"));
		return;
	}

	QString path = QApplication::applicationDirPath();
	path += "/image/DataImage/";

	QString picFilter = "Image(*.tif *.tiff *.gif *.bmp *.jpg *.jpeg *.jp2 *.png *.pcx *.pgm *.ppm *.pbm *.xwd *.ima *.ent)";
	QString strFileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开图片"), path, picFilter);

	if (!strFileName.isEmpty())
	{
		cv::Mat matImage = imread(strFileName.toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);

		cv::Mat mat;
		double fScaleW = ui.graphicsView_cellBitmap->width()*1.0 / matImage.size().width;
		double fScaleH = ui.graphicsView_cellBitmap->height()*1.0 / matImage.size().height;
		cv::resize(matImage, mat, cv::Size(matImage.size().width*fScaleW, matImage.size().height*fScaleH), (0, 0), (0, 0), 3);

		QImage image = QImage((uchar*)mat.data, mat.cols, mat.rows, ToInt(mat.step), QImage::Format_RGB888);
		m_BitmapScene->addPixmap(QPixmap::fromImage(image));

		if (strFileName.contains("/image/DataImage/"))
		{
			QString szSavePath = strFileName.right(strFileName.size() - strFileName.indexOf("/image/DataImage/"));
			m_curObj->setBitmap(szSavePath);
		}
		else
		{
			m_curObj->setBitmap(QString(""));
		}		
	}
}

void QVLCellEditor::displayObj()
{
	if (m_curObj)
	{
		QVector<QDetectObj*> cellObjs;
		cellObjs.push_back(m_curObj);
		m_pView->displayObjs(cellObjs, ui.checkBox_showNumber->isChecked());

		
		QString path = QApplication::applicationDirPath();
		QString filePath = path + m_curObj->getBitmap();
		QFile file(filePath);
		
		if (!m_curObj->getBitmap().isEmpty() && file.exists(filePath))
		{
			cv::Mat matImage = imread(filePath.toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);

			cv::Mat mat;
			double fScaleW = ui.graphicsView_cellBitmap->width()*1.0 / matImage.size().width;
			double fScaleH = ui.graphicsView_cellBitmap->height()*1.0 / matImage.size().height;
			cv::resize(matImage, mat, cv::Size(matImage.size().width*fScaleW, matImage.size().height*fScaleH), (0, 0), (0, 0), 3);

			QImage image = QImage((uchar*)mat.data, mat.cols, mat.rows, ToInt(mat.step), QImage::Format_RGB888);
			m_BitmapScene->addPixmap(QPixmap::fromImage(image));
		}
		else
		{
			m_BitmapScene->clear();
		}
	
	}
}

void QVLCellEditor::displayAllObjs()
{	
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	QVector<QDetectObj*> cellObjs;
	for (int i = 0; i < pData->getObjNum(m_dataTypeEnum); i++)
	{
		cellObjs.push_back(pData->getObj(i, m_dataTypeEnum));
	}
	m_pView->displayObjs(cellObjs, ui.checkBox_showNumber->isChecked());
}

void QVLCellEditor::onEditCellROI()
{
	if (m_pView->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (!m_curObj)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择对象"));
		return;
	}	

	m_pView->setSelect();
	while (!m_pView->isSelect())
	{
		QThread::msleep(100);
		QApplication::processEvents();
	}

	QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("检测区域设置完成"));
}

void QVLCellEditor::onEditCellFrame()
{
	if (!m_pView || m_pView->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (!m_curObj)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择对象"));
		return;
	}	

	m_pVLMaskEditor->setImage(m_pView->getSelectImage(), true);
	if (m_curObj->isFrameCreated())
	{
		Rect2f rtScale = m_pView->getSelectScale();

		cv::RotatedRect frameRect = m_curObj->getFrame();
		frameRect.center.x -= rtScale.x;
		frameRect.center.y -= rtScale.y;

		m_pVLMaskEditor->addSelect(frameRect);
	}
	m_pVLMaskEditor->repaintAll();
	m_pVLMaskEditor->setEditorView(true, false);
	m_pVLMaskEditor->setSelectRtRect();
	m_pVLMaskEditor->show();

	while (!m_pVLMaskEditor->isHidden())
	{
		QThread::msleep(100);
		QApplication::processEvents();
	}

	if (m_pVLMaskEditor->getSelectNum() > 0)
	{
		RotatedRect cellFrameRect = m_pVLMaskEditor->getSelect();

		Rect2f rtScale = m_pView->getSelectScale();

		cellFrameRect.center.x += rtScale.x;
		cellFrameRect.center.y += rtScale.y;		

		m_curObj->setFrame(cellFrameRect);	

		displayObj();
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("元件框必须存在，请重新设置"));
	}	
}

void QVLCellEditor::onEditCellLocFrame()
{
	if (m_pView->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (!m_curObj)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择对象"));
		return;
	}

	if (!m_curObj->isFrameCreated())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建元件框"));
		return;
	}	

	m_pVLMaskEditor->setImage(m_pView->getSelectImage(), true);
	if (m_curObj->isLocCreated())
	{
		Rect2f rtScale = m_pView->getSelectScale();

		cv::RotatedRect locRect = m_curObj->getLoc();
		locRect.center.x -= rtScale.x;
		locRect.center.y -= rtScale.y;

		m_pVLMaskEditor->addSelect(locRect);
	}
	m_pVLMaskEditor->repaintAll();
	m_pVLMaskEditor->setEditorView(true, false);
	m_pVLMaskEditor->setSelectRtRect();
	m_pVLMaskEditor->show();

	while (!m_pVLMaskEditor->isHidden())
	{
		QThread::msleep(100);
		QApplication::processEvents();
	}

	m_curObj->clearLoc();
	if (m_pVLMaskEditor->getSelectNum() > 0)
	{
		RotatedRect cellLocRect = m_pVLMaskEditor->getSelect();

		Rect2f rtScale = m_pView->getSelectScale();

		cellLocRect.center.x += rtScale.x;
		cellLocRect.center.y += rtScale.y;

		m_curObj->setLoc(cellLocRect);		
	}	

	displayObj();
}

void QVLCellEditor::onEditCellLocLearn()
{
	if (m_pView->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (!m_curObj)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择对象"));
		return;
	}

	if (!m_curObj->isFrameCreated())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建元件框"));
		return;
	}

	if (!m_curObj->isLocCreated())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建定位框"));
		return;
	}

	bool bAutoClearRecord = System->getParam("vision_record_auto_clear").toBool();
	if (bAutoClearRecord)
	{
		if (m_curObj->getRecordID() > 0)
		{
			Vision::VisionStatus retStatus = Vision::PR_FreeRecord(m_curObj->getRecordID());
			if (retStatus == Vision::VisionStatus::OK)
			{
				System->setTrackInfo(QString("Free previous record %1").arg(m_curObj->getRecordID()));
			}
		}
	}

	Vision::PR_LRN_TEMPLATE_CMD  stCmd;
	stCmd.matInputImg = m_pView->getImage();
	stCmd.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
	stCmd.rectROI = m_curObj->getLoc().boundingRect();

	Vision::PR_LRN_TEMPLATE_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_LrnTmpl(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		m_curObj->setRecordID(stRpy.nRecordId);
	}
	else
	{
		m_pView->addImageText(QString("Error at Learn Template Match, error code = %1").arg((int)retStatus));
	}

	cv::RotatedRect rtRect = m_curObj->getLoc();
	rtRect.size.width *= 2;
	rtRect.size.height *= 2;
	
	cv::Rect searchRect = rtRect.boundingRect();
	int nImgWidth = m_pView->getImage().cols;
	int nImgHeight = m_pView->getImage().rows;
	if (searchRect.x < 0) searchRect.x = 0;
	if (searchRect.y < 0) searchRect.y = 0;
	if (searchRect.x + searchRect.width > nImgWidth) searchRect.width = nImgWidth - searchRect.x;
	if (searchRect.y + searchRect.height > nImgHeight) searchRect.height = nImgHeight - searchRect.y;

	Vision::PR_MATCH_TEMPLATE_CMD  stCmdSrh;
	stCmdSrh.matInputImg = m_pView->getImage();
	stCmdSrh.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
	stCmdSrh.nRecordId = m_curObj->getRecordID();
	stCmdSrh.rectSrchWindow = searchRect;

	int nTmpMatchObjMotion = System->getParam("srh_tmp_obj_motion").toInt();
	stCmdSrh.enMotion = (Vision::PR_OBJECT_MOTION)nTmpMatchObjMotion;

	Vision::PR_MATCH_TEMPLATE_RPY stRpySrh;
	Vision::VisionStatus retStatusSrh = Vision::PR_MatchTmpl(&stCmdSrh, &stRpySrh);
	if (retStatusSrh == Vision::VisionStatus::OK)
	{
		m_curObj->setObjPos(stRpySrh.ptObjPos.x, stRpySrh.ptObjPos.y);
	}
	else
	{
		m_pView->addImageText(QString("Error at Template Match, error code = %1").arg((int)retStatusSrh));
	}
}

void QVLCellEditor::onEditCellBaseFrame()
{
	if (m_pView->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (!m_curObj)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择对象"));
		return;
	}

	if (!m_curObj->isFrameCreated())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建元件框"));
		return;
	}

	m_pVLMaskEditor->setImage(m_pView->getSelectImage(), true);
	if (m_curObj->isHgtBaseCreated())
	{
		Rect2f rtScale = m_pView->getSelectScale();

		for (int i = 0; i < m_curObj->getHeightBaseNum(); i++)
		{
			cv::RotatedRect rect = m_curObj->getHeightBase(i);
			rect.center.x -= rtScale.x;
			rect.center.y -= rtScale.y;

			m_pVLMaskEditor->addSelect(rect);
		}		
	}
	m_pVLMaskEditor->repaintAll();
	m_pVLMaskEditor->setEditorView(true, false);
	m_pVLMaskEditor->setSelectRtRect();
	m_pVLMaskEditor->show();

	while (!m_pVLMaskEditor->isHidden())
	{
		QThread::msleep(100);
		QApplication::processEvents();
	}

	if (m_curObj->getHeightBaseNum() != m_pVLMaskEditor->getSelectNum())
	{
		m_curObj->clearDBRelation();
	}

	m_curObj->clearHeightBase();
	if (m_pVLMaskEditor->getSelectNum() > 0)
	{		
		Rect2f rtScale = m_pView->getSelectScale();

		for (int i = 0; i < m_pVLMaskEditor->getSelectNum(); i++)
		{
			RotatedRect rect = m_pVLMaskEditor->getSelect(i);			

			rect.center.x += rtScale.x;
			rect.center.y += rtScale.y;

			m_curObj->addHeightBase(rect);
		}			
	}
	displayObj();

	updateDBMenu();
}

void QVLCellEditor::onEditCellDetectFrame()
{
	if (m_pView->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (!m_curObj)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择对象"));
		return;
	}

	if (!m_curObj->isFrameCreated())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建元件框"));
		return;
	}

	m_pVLMaskEditor->setImage(m_pView->getSelectImage(), true);
	if (m_curObj->isHgtDetectCreated())
	{
		Rect2f rtScale = m_pView->getSelectScale();

		for (int i = 0; i < m_curObj->getHeightDetectNum(); i++)
		{
			cv::RotatedRect rect = m_curObj->getHeightDetect(i);
			rect.center.x -= rtScale.x;
			rect.center.y -= rtScale.y;

			m_pVLMaskEditor->addSelect(rect);
		}
	}
	m_pVLMaskEditor->repaintAll();
	m_pVLMaskEditor->setEditorView(true, false);
	m_pVLMaskEditor->setSelectRtRect();
	m_pVLMaskEditor->show();

	while (!m_pVLMaskEditor->isHidden())
	{
		QThread::msleep(100);
		QApplication::processEvents();
	}

	if (m_curObj->getHeightDetectNum() != m_pVLMaskEditor->getSelectNum())
	{
		m_curObj->clearDBRelation();
	}

	m_curObj->clearHeightDetect();
	if (m_pVLMaskEditor->getSelectNum() > 0)
	{
		Rect2f rtScale = m_pView->getSelectScale();

		for (int i = 0; i < m_pVLMaskEditor->getSelectNum(); i++)
		{
			RotatedRect rect = m_pVLMaskEditor->getSelect(i);			

			rect.center.x += rtScale.x;
			rect.center.y += rtScale.y;

			m_curObj->addHeightDetect(rect);
		}
	}
	displayObj();

	updateDBMenu();
}

void QVLCellEditor::onSearchCell()
{
	if (m_pView->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (!m_curObj)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择对象"));
		return;
	}

	if (!m_curObj->isFrameCreated())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建元件框"));
		return;
	}

	if (!m_curObj->isLocCreated())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建定位框"));
		return;
	}

	cv::RotatedRect rtRect = m_curObj->getLoc();
	rtRect.size.width *= 2;
	rtRect.size.height *= 2;

	cv::Rect searchRect = rtRect.boundingRect();
	int nImgWidth = m_pView->getImage().cols;
	int nImgHeight = m_pView->getImage().rows;
	if (searchRect.x < 0) searchRect.x = 0;
	if (searchRect.y < 0) searchRect.y = 0;
	if (searchRect.x + searchRect.width > nImgWidth) searchRect.width = nImgWidth - searchRect.x;
	if (searchRect.y + searchRect.height > nImgHeight) searchRect.height = nImgHeight - searchRect.y;

	Vision::PR_MATCH_TEMPLATE_CMD  stCmdSrh;
	stCmdSrh.matInputImg = m_pView->getImage();
	stCmdSrh.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
	stCmdSrh.nRecordId = m_curObj->getRecordID();
	stCmdSrh.rectSrchWindow = searchRect;

	int nTmpMatchObjMotion = System->getParam("srh_tmp_obj_motion").toInt();
	stCmdSrh.enMotion = (Vision::PR_OBJECT_MOTION)nTmpMatchObjMotion;

	Vision::PR_MATCH_TEMPLATE_RPY stRpySrh;
	Vision::VisionStatus retStatusSrh = Vision::PR_MatchTmpl(&stCmdSrh, &stRpySrh);
	if (retStatusSrh == Vision::VisionStatus::OK)
	{	
		//RotatedRect rtFrame = m_curObj->getFrame();
		//rtFrame.center.x += stRpySrh.ptObjPos.x - m_curObj->getX();
		//rtFrame.center.y += stRpySrh.ptObjPos.y - m_curObj->getY();
		//m_curObj->setFrame(rtFrame);

		m_curObj->setObjPos(stRpySrh.ptObjPos.x, stRpySrh.ptObjPos.y);
	}
	else
	{
		m_pView->addImageText(QString("Error at Template Match, error code = %1").arg((int)retStatusSrh));
	}

	displayObj();
}

void QVLCellEditor::onCellDetectIndexChanged(int iIndex)
{
	int nIndex = ui.comboBox_cellDetectIndex->currentIndex();

	refreshDBRelation();

	//displayObj();
}

void QVLCellEditor::onCellBaseIndexChanged(int iIndex)
{
	int nIndex = ui.comboBox_cellBaseIndex->currentIndex();

	refreshDBRelation();

	//displayObj();
}

void QVLCellEditor::onAddCellRelation()
{
	int nDetectIndex = ui.comboBox_cellDetectIndex->currentIndex();
	int nBaseIndex = ui.comboBox_cellBaseIndex->currentIndex();

	if (m_curObj)
	{
		m_curObj->addDBRelation(nDetectIndex, nBaseIndex);
	}

	refreshDBRelation();
}

void QVLCellEditor::onDeleteCellRelation()
{
	int nDetectIndex = ui.comboBox_cellDetectIndex->currentIndex();
	int nBaseIndex = ui.comboBox_cellBaseIndex->currentIndex();

	if (m_curObj)
	{
		m_curObj->removeDBRelation(nDetectIndex, nBaseIndex);
	}

	refreshDBRelation();
}

void QVLCellEditor::updateDBMenu()
{
	ui.comboBox_cellDetectIndex->clear();
	ui.comboBox_cellBaseIndex->clear();
	if (m_curObj)
	{
		for (int i = 0; i < m_curObj->getHeightDetectNum(); i++)
		{
			ui.comboBox_cellDetectIndex->addItem(QString("%1").arg(i+1));
		}	

		for (int i = 0; i < m_curObj->getHeightBaseNum(); i++)
		{
			ui.comboBox_cellBaseIndex->addItem(QString("%1").arg(i + 1));
		}
	}
}

void QVLCellEditor::refreshDBRelation()
{
	m_model.clear();

	QStringList ls;
	ls << QStringLiteral("序号") << QStringLiteral("测量位置") << QStringLiteral("Base位置");
	m_model.setHorizontalHeaderLabels(ls);

	ui.tableView_ItemDetAndBase->setColumnWidth(0, 50);
	ui.tableView_ItemDetAndBase->setColumnWidth(1, 70);
	ui.tableView_ItemDetAndBase->setColumnWidth(2, 70);

	if (m_curObj)
	{
		QList<int> detectIndexs = m_curObj->getDBRelationDetect();
		int count = 0;
		for (int i = 0; i < detectIndexs.size(); i++)
		{
			QList<int> baseIndexs = m_curObj->getDBRelationBase(detectIndexs[i]);

			for (int j = 0; j < baseIndexs.size(); j++)
			{
				int nr = m_model.rowCount();
				m_model.insertRow(nr);

				m_model.setData(m_model.index(nr, 0), QStringLiteral("%1").arg(++count));
				m_model.setData(m_model.index(nr, 1), QStringLiteral("%1").arg(detectIndexs[i] + 1));
				m_model.setData(m_model.index(nr, 2), QStringLiteral("%1").arg(baseIndexs[j] + 1));
			}
		}
	}	
}

void QVLCellEditor::onLoadConfigFile()
{
	loadDataBase();
	//loadConfigData();
}

void QVLCellEditor::onSaveConfigFile()
{
	//saveMapData();
	if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
		QStringLiteral("是否保存当前数据并覆盖系统存档？"), QMessageBox::Ok, QMessageBox::Cancel))
	{
		saveDataBase();
	}
}

void QVLCellEditor::onAlignmentIndexChanged(int iIndex)
{	
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	if (ui.comboBox_alignIndex->currentIndex() >= 0)
	{
		int nIndex = ui.comboBox_alignIndex->currentIndex();		
	}
}

void QVLCellEditor::onAddBoardAlign()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	QBoardObj* pBoardObj = pData->getBoardObj();
	if (!pBoardObj) return;

	if (m_pView->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	m_pVLMaskEditor->setImage(m_pView->getImage(), true);
	if (pBoardObj->isBoardCreated())
	{
		for (int i = 0; i < pBoardObj->getBoardAlignNum(); i++)
		{
			cv::RotatedRect rect = pBoardObj->getBoardAlign(i);	

			m_pVLMaskEditor->addSelect(rect);
		}
	}
	m_pVLMaskEditor->repaintAll();
	m_pVLMaskEditor->setEditorView(true, false);
	m_pVLMaskEditor->setSelectRtRect();
	m_pVLMaskEditor->show();

	while (!m_pVLMaskEditor->isHidden())
	{
		QThread::msleep(100);
		QApplication::processEvents();
	}

	bool bAutoClearRecord = System->getParam("vision_record_auto_clear").toBool();
	if (bAutoClearRecord)
	{
		for (int i = 0; i < pBoardObj->getBoardAlignNum(); i++)
		{
			Vision::VisionStatus retStatus = Vision::PR_FreeRecord(pBoardObj->getRecordID(i));
			if (retStatus == Vision::VisionStatus::OK)
			{
				System->setTrackInfo(QString("Free previous record %1").arg(pBoardObj->getRecordID(i)));
			}
		}
	}

	pBoardObj->clearBoardAlignments();
	ui.comboBox_alignIndex->clear();

	if (m_pVLMaskEditor->getSelectNum() > 0)
	{
		for (int i = 0; i < m_pVLMaskEditor->getSelectNum(); i++)
		{
			RotatedRect rect = m_pVLMaskEditor->getSelect(i);
			int nRecordID = 0;

			Vision::PR_LRN_TEMPLATE_CMD  stCmd;
			stCmd.matInputImg = m_pView->getImage();
			stCmd.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
			stCmd.rectROI = rect.boundingRect();

			Vision::PR_LRN_TEMPLATE_RPY stRpy;
			Vision::VisionStatus retStatus = Vision::PR_LrnTmpl(&stCmd, &stRpy);
			if (retStatus == Vision::VisionStatus::OK)
			{
				nRecordID = stRpy.nRecordId;
			}
			else
			{
				m_pView->addImageText(QString("Error at Learn Template Match, error code = %1").arg((int)retStatus));
			}			

			pBoardObj->addBoardAlign(rect, nRecordID);

			ui.comboBox_alignIndex->addItem(QString("%1").arg(ui.comboBox_alignIndex->count()));
			ui.comboBox_alignIndex->setCurrentIndex(ui.comboBox_alignIndex->count() - 1);
		}
	}
}

void QVLCellEditor::onDeleteBoardAlign()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	QBoardObj* pBoardObj = pData->getBoardObj();

	if (pBoardObj)
	{
		if (ui.comboBox_alignIndex->currentIndex() >= 0)
		{
			int nIndex = ui.comboBox_alignIndex->currentIndex();

			pBoardObj->removeBoardAlign(nIndex);

			ui.comboBox_alignIndex->removeItem(nIndex);

			ui.comboBox_alignIndex->setCurrentIndex(0);
		}
	}
}

void QVLCellEditor::saveMapData()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	QString path = QApplication::applicationDirPath();
	path += "/3D/data/";

	std::string strResultMatPath = QString(path + "mapData.yml").toStdString();
	cv::FileStorage fs(strResultMatPath, cv::FileStorage::WRITE);
	if (!fs.isOpened())
		return;
	
	write(fs, QString("obj_num").toStdString(), pData->getObjNum(m_dataTypeEnum));
	for (int i = 0; i < pData->getObjNum(m_dataTypeEnum); i++)
	{
		QDetectObj* pObj = pData->getObj(i, m_dataTypeEnum);
		if (pObj)
		{	
			write(fs, QString("obj_name_%1").arg(i + 1).toStdString(), pObj->getName().toStdString());			

			//write(fs, QString("pos_x_%1").arg(i + 1).toStdString(), pObj->getX());
			//write(fs, QString("pos_y_%1").arg(i + 1).toStdString(), pObj->getY());

			RotatedRect rtFrame = pObj->getFrame();
			write(fs, QString("frame_center_%1").arg(i + 1).toStdString(), rtFrame.center);
			write(fs, QString("frame_size_%1").arg(i + 1).toStdString(), rtFrame.size);
			write(fs, QString("frame_angle_%1").arg(i + 1).toStdString(), rtFrame.angle);

			RotatedRect rtLoc = pObj->getLoc();
			write(fs, QString("loc_center_%1").arg(i + 1).toStdString(), rtLoc.center);
			write(fs, QString("loc_size_%1").arg(i + 1).toStdString(), rtLoc.size);
			write(fs, QString("loc_angle_%1").arg(i + 1).toStdString(), rtLoc.angle);
			write(fs, QString("loc_record_%1").arg(i + 1).toStdString(), pObj->getRecordID());

			write(fs, QString("detect_num_%1").arg(i + 1).toStdString(), pObj->getHeightDetectNum());
			for (int j = 0; j < pObj->getHeightDetectNum(); j++)
			{
				RotatedRect rtDetect = pObj->getHeightDetect(j);
				write(fs, QString("detect_center_%1_%2").arg(i + 1).arg(j+1).toStdString(), rtDetect.center);
				write(fs, QString("detect_size_%1_%2").arg(i + 1).arg(j+1).toStdString(), rtDetect.size);
				write(fs, QString("detect_angle_%1_%2").arg(i + 1).arg(j+1).toStdString(), rtDetect.angle);
			}

			write(fs, QString("base_num_%1").arg(i + 1).toStdString(), pObj->getHeightBaseNum());
			for (int j = 0; j < pObj->getHeightBaseNum(); j++)
			{
				RotatedRect rtBase = pObj->getHeightBase(j);
				write(fs, QString("base_center_%1_%2").arg(i + 1).arg(j + 1).toStdString(), rtBase.center);
				write(fs, QString("base_size_%1_%2").arg(i + 1).arg(j + 1).toStdString(), rtBase.size);
				write(fs, QString("base_angle_%1_%2").arg(i + 1).arg(j + 1).toStdString(), rtBase.angle);
			}


			QList<int> detectList = pObj->getDBRelationDetect();
			write(fs, QString("relation_detect_num_%1").arg(i + 1).toStdString(), detectList.size());
			for (int j = 0; j < detectList.size(); j++)
			{				
				write(fs, QString("relation_detect_%1_%2").arg(i + 1).arg(j + 1).toStdString(), detectList[j]);

				QList<int> baseList = pObj->getDBRelationBase(detectList[j]);
				write(fs, QString("relation_base_num_%1_%2").arg(i + 1).arg(j + 1).toStdString(), baseList.size());
				for (int m = 0; m < baseList.size(); m++)
				{
					write(fs, QString("relation_base_%1_%2_%3").arg(i + 1).arg(j + 1).arg(m + 1).toStdString(), baseList[m]);
				}
			}
		}		
	}	

	fs.release();
}

void QVLCellEditor::loadMapData()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	pData->clearObjs(m_dataTypeEnum);

	QString path = QApplication::applicationDirPath();
	path += "/3D/data/";

	std::string strResultMatPath = QString(path + "mapData.yml").toStdString();
	cv::FileStorage fs(strResultMatPath, cv::FileStorage::READ);
	if (!fs.isOpened())
		return;

	int  nObjNum = 0;

	cv::FileNode fileNode = fs["obj_num"];
	cv::read(fileNode, nObjNum, 0);

	for (int i = 0; i < nObjNum; i++)
	{
		cv::String objName = "";
		fileNode = fs[QString("obj_name_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, objName, "");

		QDetectObj *pObj = new QDetectObj(-1, objName.c_str());

		//double posX = 0, posY = 0;
		//fileNode = fs[QString("pos_x_%1").arg(i + 1).toStdString()];
		//cv::read(fileNode, posX, 0);
		//fileNode = fs[QString("pos_y_%1").arg(i + 1).toStdString()];
		//cv::read(fileNode, posY, 0);
		//pObj->setObjPos(posX, posY);

		RotatedRect rtFrame;
		fileNode = fs[QString("frame_center_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, rtFrame.center, cv::Point2f());
		fileNode = fs[QString("frame_size_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, rtFrame.size, cv::Size2f());
		fileNode = fs[QString("frame_angle_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, rtFrame.angle, 0);
		pObj->setFrame(rtFrame);

		RotatedRect rtLoc;
		fileNode = fs[QString("loc_center_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, rtLoc.center, cv::Point2f());
		fileNode = fs[QString("loc_size_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, rtLoc.size, cv::Size2f());
		fileNode = fs[QString("loc_angle_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, rtLoc.angle, 0);
		pObj->setLoc(rtLoc);

		int nRecordID = 0;
		fileNode = fs[QString("loc_record_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, nRecordID, 0);
		pObj->setRecordID(nRecordID);


		int nDetectNum = 0;
		fileNode = fs[QString("detect_num_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, nDetectNum, 0);		
		for (int j = 0; j < nDetectNum; j++)
		{
			RotatedRect rtDetect;
			fileNode = fs[QString("detect_center_%1_%2").arg(i + 1).arg(j + 1).toStdString()];
			cv::read(fileNode, rtDetect.center, cv::Point2f());
			fileNode = fs[QString("detect_size_%1_%2").arg(i + 1).arg(j + 1).toStdString()];
			cv::read(fileNode, rtDetect.size, cv::Size2f());
			fileNode = fs[QString("detect_angle_%1_%2").arg(i + 1).arg(j + 1).toStdString()];
			cv::read(fileNode, rtDetect.angle, 0);
			pObj->addHeightDetect(rtDetect);		
		}

		int nBaseNum = 0;
		fileNode = fs[QString("base_num_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, nBaseNum, 0);
		for (int j = 0; j < nBaseNum; j++)
		{
			RotatedRect rtBase;
			fileNode = fs[QString("base_center_%1_%2").arg(i + 1).arg(j + 1).toStdString()];
			cv::read(fileNode, rtBase.center, cv::Point2f());
			fileNode = fs[QString("base_size_%1_%2").arg(i + 1).arg(j + 1).toStdString()];
			cv::read(fileNode, rtBase.size, cv::Size2f());
			fileNode = fs[QString("base_angle_%1_%2").arg(i + 1).arg(j + 1).toStdString()];
			cv::read(fileNode, rtBase.angle, 0);
			pObj->addHeightBase(rtBase);
		}

		int nRelationDetectNum = 0;
		fileNode = fs[QString("relation_detect_num_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, nRelationDetectNum, 0);
		for (int j = 0; j < nRelationDetectNum; j++)
		{
			int nRelationDetectIndex = 0;
			fileNode = fs[QString("relation_detect_%1_%2").arg(i + 1).arg(j + 1).toStdString()];
			cv::read(fileNode, nRelationDetectIndex, 0);

			int nRelationBaseNum = 0;
			fileNode = fs[QString("relation_base_num_%1_%2").arg(i + 1).arg(j + 1).toStdString()];
			cv::read(fileNode, nRelationBaseNum, 0);
			for (int m = 0; m < nRelationBaseNum; m++)
			{
				int nRelationBaseIndex = 0;
				fileNode = fs[QString("relation_base_%1_%2_%3").arg(i + 1).arg(j + 1).arg(m + 1).toStdString()];
				cv::read(fileNode, nRelationBaseIndex, 0);

				pObj->addDBRelation(nRelationDetectIndex, nRelationBaseIndex);
			}
		}

		pData->pushObj(pObj, m_dataTypeEnum);

		ui.comboBox_cellIndex->addItem(QString("%1").arg(objName.c_str()));		
	}	

	ui.comboBox_cellIndex->setCurrentIndex(0);

	fs.release();
}

void QVLCellEditor::saveDataBase()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	pData->saveDataBase(QString(""), m_dataTypeEnum);
}

void QVLCellEditor::loadDataBase()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	pData->clearObjs(m_dataTypeEnum);
	m_curObj = NULL;

	pData->loadDataBase(QString(""), m_dataTypeEnum);

	loadConfigData();
}