#include "VLProfileEditor.h"
#include <QFileDialog>

#include "VisionCtrl.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IData.h"

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

#include "../DataModule/QProfileObj.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "qcustomplot.h"
#include "QProfileView.h"

#define ToInt(value)                (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))
using namespace AOI;

QVLProfileEditor::QVLProfileEditor(VisionCtrl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QMainWindow(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::Dialog);
	setWindowModality(Qt::ApplicationModal);

	ui.tableView_profileList->setModel(&m_modelProf);
	ui.tableView_measureList->setModel(&m_modelMeasure);

	m_curObj = NULL;

	m_imgGrayScene = new QGraphicsScene();
	ui.graphicsView_imgGray->setScene(m_imgGrayScene);
	ui.graphicsView_imgGray->setFixedSize(400, 300);
	ui.graphicsView_imgGray->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_imgGray->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_img3DScene = new QGraphicsScene();
	ui.graphicsView_img3D->setScene(m_img3DScene);
	ui.graphicsView_img3D->setFixedSize(400, 300);
	ui.graphicsView_img3D->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_img3D->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	ui.dockWidget_profileData->setFixedSize(800, 400);

	m_pPlotProfile = new QCustomPlot(this);
	setupLineChartDemo(m_pPlotProfile);
	ui.dockWidget_profileData->setWidget(m_pPlotProfile);

	m_pProfView = new QProfileView();

	init();	

	connect(ui.comboBox_selectProfile, SIGNAL(currentIndexChanged(int)), SLOT(onProfileIndexChanged(int)));
	connect(ui.pushButton_addProfile, SIGNAL(clicked()), SLOT(onAddProfile()));
	connect(ui.pushButton_deleteProfile, SIGNAL(clicked()), SLOT(onDeleteProfile()));

	connect(ui.comboBox_selectMeasure, SIGNAL(currentIndexChanged(int)), SLOT(onMeasureIndexChanged(int)));
	connect(ui.pushButton_addMeasure, SIGNAL(clicked()), SLOT(onAddMeasure()));
	connect(ui.pushButton_deleteMeasure, SIGNAL(clicked()), SLOT(onDeleteMeasure()));

	connect(ui.pushButton_loadConfigFile, SIGNAL(clicked()), SLOT(loadFile()));
	connect(ui.pushButton_saveConfigFile, SIGNAL(clicked()), SLOT(saveFile()));

	loadDataBase();
}

QVLProfileEditor::~QVLProfileEditor()
{	
	if (m_imgGrayScene)
	{
		delete m_imgGrayScene;
		m_imgGrayScene = NULL;
	}

	if (m_img3DScene)
	{
		delete m_img3DScene;
		m_img3DScene = NULL;
	}

	if (m_pPlotProfile)
	{
		delete m_pPlotProfile;
		m_pPlotProfile = NULL;
	}

	if (m_pProfView)
	{
		delete m_pProfView;
		m_pProfView = NULL;
	}
}

void QVLProfileEditor::closeEvent(QCloseEvent *e){
	//qDebug() << "关闭事件";
	//e->ignore();

	this->hide();
}

void QVLProfileEditor::init()
{
	createActions();
	createToolBars();
	createStatusBar();

	setUnifiedTitleAndToolBarOnMac(true);
}

void QVLProfileEditor::createActions()
{
	loadAct = new QAction(QIcon("image/openFile.png"), QStringLiteral("导入数据..."), this);
	loadAct->setShortcuts(QKeySequence::Open);
	loadAct->setStatusTip(tr("Open data..."));
	connect(loadAct, SIGNAL(triggered()), this, SLOT(loadFile()));

	saveAct = new QAction(QIcon("image/saveAsFile.png"), QStringLiteral("保存数据..."), this);
	saveAct->setShortcuts(QKeySequence::SaveAs);
	saveAct->setStatusTip(tr("Save data..."));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

	line2lineAct = new QAction(QIcon("image/Profile/ln2ln.png"), QStringLiteral("线到线距离..."), this);
	line2lineAct->setShortcuts(QKeySequence::MoveToEndOfLine);
	line2lineAct->setStatusTip(tr("Profile Measure"));
	connect(line2lineAct, SIGNAL(triggered()), this, SLOT(onLn2LnDist()));

	point2lineAct = new QAction(QIcon("image/Profile/ln2pt.png"), QStringLiteral("点到线距离..."), this);
	point2lineAct->setShortcuts(QKeySequence::MoveToStartOfBlock);
	point2lineAct->setStatusTip(tr("Profile Measure"));
	connect(point2lineAct, SIGNAL(triggered()), this, SLOT(onPt2LnDist()));

	point2pointAct = new QAction(QIcon("image/Profile/pt2pt.png"), QStringLiteral("点到点距离..."), this);
	point2pointAct->setShortcuts(QKeySequence::MoveToEndOfBlock);
	point2pointAct->setStatusTip(tr("Profile Measure"));
	connect(point2pointAct, SIGNAL(triggered()), this, SLOT(onPt2PtDist()));

	pointHeightAct = new QAction(QIcon("image/Profile/ptHeight.png"), QStringLiteral("点的高度..."), this);
	pointHeightAct->setShortcuts(QKeySequence::MoveToStartOfDocument);
	pointHeightAct->setStatusTip(tr("Profile Measure"));
	connect(pointHeightAct, SIGNAL(triggered()), this, SLOT(onPtHeight()));

	circle2circleAct = new QAction(QIcon("image/Profile/crl2crl.png"), QStringLiteral("圆到圆距离..."), this);
	circle2circleAct->setShortcuts(QKeySequence::MoveToEndOfDocument);
	circle2circleAct->setStatusTip(tr("Profile Measure"));
	connect(circle2circleAct, SIGNAL(triggered()), this, SLOT(onCir2CirDist()));

	circle2lineAct = new QAction(QIcon("image/Profile/crl2ln.png"), QStringLiteral("圆到线距离..."), this);
	circle2lineAct->setShortcuts(QKeySequence::SelectNextChar);
	circle2lineAct->setStatusTip(tr("Profile Measure"));
	connect(circle2lineAct, SIGNAL(triggered()), this, SLOT(onCir2LnDist()));

	circle2pointAct = new QAction(QIcon("image/Profile/crl2pt.png"), QStringLiteral("圆到点距离..."), this);
	circle2pointAct->setShortcuts(QKeySequence::SelectPreviousChar);
	circle2pointAct->setStatusTip(tr("Profile Measure"));
	connect(circle2pointAct, SIGNAL(triggered()), this, SLOT(onCir2PtDist()));

	arcRadiusAct = new QAction(QIcon("image/Profile/arcRadius.png"), QStringLiteral("圆弧半径..."), this);
	arcRadiusAct->setShortcuts(QKeySequence::SelectNextWord);
	arcRadiusAct->setStatusTip(tr("Profile Measure"));
	connect(arcRadiusAct, SIGNAL(triggered()), this, SLOT(onCrRadious()));

	line2lineAngleAct = new QAction(QIcon("image/Profile/ln2lnAngle.png"), QStringLiteral("线与线夹角..."), this);
	line2lineAngleAct->setShortcuts(QKeySequence::SelectPreviousWord);
	line2lineAngleAct->setStatusTip(tr("Profile Measure"));
	connect(line2lineAngleAct, SIGNAL(triggered()), this, SLOT(onLn2LnAngle()));

	squareAreaAct = new QAction(QIcon("image/Profile/profArea.png"), QStringLiteral("区域面积..."), this);
	squareAreaAct->setShortcuts(QKeySequence::SelectNextLine);
	squareAreaAct->setStatusTip(tr("Profile Measure"));
	connect(squareAreaAct, SIGNAL(triggered()), this, SLOT(onProfArea()));

	profileLengthAct = new QAction(QIcon("image/Profile/profLength.png"), QStringLiteral("线段长度..."), this);
	profileLengthAct->setShortcuts(QKeySequence::SelectPreviousLine);
	profileLengthAct->setStatusTip(tr("Profile Measure"));
	connect(profileLengthAct, SIGNAL(triggered()), this, SLOT(onProfLength()));
}

void QVLProfileEditor::createToolBars()
{
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(loadAct);
	fileToolBar->addAction(saveAct);

	editToolBar = addToolBar(tr("Edit"));
	editToolBar->addAction(line2lineAct);
	editToolBar->addAction(point2lineAct);
	editToolBar->addAction(point2pointAct);
	editToolBar->addAction(pointHeightAct);
	editToolBar->addAction(circle2circleAct);
	editToolBar->addAction(circle2lineAct);
	editToolBar->addAction(circle2pointAct);
	editToolBar->addAction(arcRadiusAct);
	editToolBar->addAction(line2lineAngleAct);
	editToolBar->addAction(squareAreaAct);
	editToolBar->addAction(profileLengthAct);
}

void QVLProfileEditor::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void QVLProfileEditor::loadFile()
{
	if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
		QStringLiteral("是否导入系统存档数据？"), QMessageBox::Ok, QMessageBox::Cancel))
	{
		loadDataBase();
	}
}

void QVLProfileEditor::saveFile()
{
	if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
		QStringLiteral("是否保存当前数据并覆盖系统存档？"), QMessageBox::Ok, QMessageBox::Cancel))
	{
		saveDataBase();
	}
}

void QVLProfileEditor::onLn2LnDist()
{
	if (m_curObj)
	{
		m_pProfView->clearMeasure();

		cv::Mat imgMat;
		captureProfDataImage(imgMat);

		m_pProfView->setImage(imgMat);
		m_pProfView->show();

		while (!m_pProfView->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		QVector<cv::Rect> rectRanges = m_pProfView->getMeasureRange();

		if (rectRanges.size() != 2)
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择两条线段区域！"));
			return;
		}

		int nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(0).x);
		int nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(0).x + rectRanges.at(0).width);

		Vision::PR_Line2f stLine1;
		if (!m_pCtrl->fitLine(m_profDatas, nProfDataIndex1, nProfDataIndex2, stLine1))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("第一条线拟合失败！"));
			return;
		}

		int nProfDataIndex3 = convetToProfileDataIndex(rectRanges.at(1).x);
		int nProfDataIndex4 = convetToProfileDataIndex(rectRanges.at(1).x + rectRanges.at(1).width);

		Vision::PR_Line2f stLine2;
		if (!m_pCtrl->fitLine(m_profDatas, nProfDataIndex3, nProfDataIndex4, stLine2))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("第二条线拟合失败！"));
			return;
		}

		QProfMeasLnToLnDist* pObj = new QProfMeasLnToLnDist();
		if (!m_pCtrl->calcTwoLineDist(stLine1, stLine2, pObj))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("计算两条线的距离失败！"));
			delete pObj;
			return;
		}

		cv::Point ptLine1, ptLine2;
		ptLine1.x = nProfDataIndex1;
		ptLine1.y = nProfDataIndex2;
		ptLine2.x = nProfDataIndex3;
		ptLine2.y = nProfDataIndex4;

		pObj->addRange(ptLine1, MEASURE_DATA_TYPE_LINE);
		pObj->addRange(ptLine2, MEASURE_DATA_TYPE_LINE);

		m_curObj->addMeasObj(pObj);

		updateMeasure(ui.comboBox_selectMeasure->count());
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Profile"));
	}
}

void QVLProfileEditor::onPt2LnDist()
{
	if (m_curObj)
	{
		m_pProfView->clearMeasure();

		cv::Mat imgMat;
		captureProfDataImage(imgMat);

		m_pProfView->setImage(imgMat);
		m_pProfView->show();

		while (!m_pProfView->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		QVector<cv::Rect> rectRanges = m_pProfView->getMeasureRange();

		if (rectRanges.size() != 2)
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择1条线段区域和一条线交点！"));
			return;
		}

		int nProfDataIndex1 = 0;
		int nProfDataIndex2 = 0;
		int nProfDataIndex3 = 0;
		int nProfDataIndex4 = 0;

		bool bReversedFit = false;  double fSlope = 0, fIntercept = 0;
		if (rectRanges.at(0).width == 1)
		{
			nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(0).x);
			nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(0).x);

			nProfDataIndex3 = convetToProfileDataIndex(rectRanges.at(1).x);
			nProfDataIndex4 = convetToProfileDataIndex(rectRanges.at(1).x + rectRanges.at(1).width);

			if (!m_pCtrl->fitLine(m_profDatas, nProfDataIndex3, nProfDataIndex4, bReversedFit, fSlope, fIntercept))
			{
				QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("第一条线拟合失败！"));
				return;
			}
		}
		else
		{
			nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(1).x);
			nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(1).x);

			nProfDataIndex3 = convetToProfileDataIndex(rectRanges.at(0).x);
			nProfDataIndex4 = convetToProfileDataIndex(rectRanges.at(0).x + rectRanges.at(0).width);

			if (!m_pCtrl->fitLine(m_profDatas, nProfDataIndex3, nProfDataIndex4, bReversedFit, fSlope, fIntercept))
			{
				QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("第一条线拟合失败！"));
				return;
			}
		}

		QProfMeasPtToLnDist* pObj = new QProfMeasPtToLnDist();
		cv::Point2d ptOrg = m_profDatas[nProfDataIndex1];
		if (!m_pCtrl->calcPtToLineDist(ptOrg, bReversedFit, fSlope, fIntercept, pObj))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("计算两条线的距离失败！"));
			delete pObj;
			return;
		}

		cv::Point ptLine1, ptLine2;
		ptLine1.x = nProfDataIndex1;
		ptLine1.y = nProfDataIndex2;
		ptLine2.x = nProfDataIndex3;
		ptLine2.y = nProfDataIndex4;

		pObj->addRange(ptLine1, MEASURE_DATA_TYPE_POINT);
		pObj->addRange(ptLine2, MEASURE_DATA_TYPE_LINE);

		m_curObj->addMeasObj(pObj);

		updateMeasure(ui.comboBox_selectMeasure->count());
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Profile"));
	}
}

void QVLProfileEditor::onPt2PtDist()
{
	if (m_curObj)
	{
		m_pProfView->clearMeasure();

		cv::Mat imgMat;
		captureProfDataImage(imgMat);

		m_pProfView->setImage(imgMat);
		m_pProfView->show();

		while (!m_pProfView->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		QVector<cv::Rect> rectRanges = m_pProfView->getMeasureRange();

		if (rectRanges.size() != 2)
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择2条线交点！"));
			return;
		}

		int nProfDataIndex1 = 0;
		int nProfDataIndex2 = 0;
		int nProfDataIndex3 = 0;
		int nProfDataIndex4 = 0;

		nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(0).x);
		nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(0).x);

		nProfDataIndex3 = convetToProfileDataIndex(rectRanges.at(1).x);
		nProfDataIndex4 = convetToProfileDataIndex(rectRanges.at(1).x);

		QProfMeasPtToPtDist* pObj = new QProfMeasPtToPtDist();
		cv::Point2d pt1 = m_profDatas[nProfDataIndex1];
		cv::Point2d pt2 = m_profDatas[nProfDataIndex3];
		if (!m_pCtrl->calcPtToPtDist(pt1, pt2, pObj))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("计算两点的距离失败！"));
			delete pObj;
			return;
		}

		cv::Point ptLine1, ptLine2;
		ptLine1.x = nProfDataIndex1;
		ptLine1.y = nProfDataIndex2;
		ptLine2.x = nProfDataIndex3;
		ptLine2.y = nProfDataIndex4;

		pObj->addRange(ptLine1, MEASURE_DATA_TYPE_POINT);
		pObj->addRange(ptLine2, MEASURE_DATA_TYPE_POINT);

		m_curObj->addMeasObj(pObj);

		updateMeasure(ui.comboBox_selectMeasure->count());
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Profile"));
	}
}

void QVLProfileEditor::onPtHeight()
{
	if (m_curObj)
	{
		m_pProfView->clearMeasure();

		cv::Mat imgMat;
		captureProfDataImage(imgMat);

		m_pProfView->setImage(imgMat);
		m_pProfView->show();

		while (!m_pProfView->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		QVector<cv::Rect> rectRanges = m_pProfView->getMeasureRange();

		if (rectRanges.size() != 1)
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择1条线交点！"));
			return;
		}

		int nProfDataIndex1 = 0;
		int nProfDataIndex2 = 0;
		int nProfDataIndex3 = 0;
		int nProfDataIndex4 = 0;

		nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(0).x);
		nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(0).x);	

		QProfMeasPtHeight* pObj = new QProfMeasPtHeight();
		cv::Point2d pt1 = m_profDatas[nProfDataIndex1];	
		pObj->_dHeight = pt1.y;

		cv::Point ptLine1, ptLine2;
		ptLine1.x = nProfDataIndex1;
		ptLine1.y = nProfDataIndex2;
		ptLine2.x = nProfDataIndex3;
		ptLine2.y = nProfDataIndex4;

		pObj->addRange(ptLine1, MEASURE_DATA_TYPE_POINT);
		//pObj->addRange(ptLine2);

		m_curObj->addMeasObj(pObj);

		updateMeasure(ui.comboBox_selectMeasure->count());
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Profile"));
	}
}

void QVLProfileEditor::onCir2CirDist()
{
	if (m_curObj)
	{
		m_pProfView->clearMeasure();

		cv::Mat imgMat;
		captureProfDataImage(imgMat);

		m_pProfView->setImage(imgMat);
		m_pProfView->show();

		while (!m_pProfView->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		QVector<cv::Rect> rectRanges = m_pProfView->getMeasureRange();

		if (rectRanges.size() != 2)
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择2个圆弧区域！"));
			return;
		}

		int nProfDataIndex1 = 0;
		int nProfDataIndex2 = 0;
		int nProfDataIndex3 = 0;
		int nProfDataIndex4 = 0;

		nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(0).x);
		nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(0).x + rectRanges.at(0).width);
		nProfDataIndex3 = convetToProfileDataIndex(rectRanges.at(1).x);
		nProfDataIndex4 = convetToProfileDataIndex(rectRanges.at(1).x + rectRanges.at(1).width);

		QProfMeasCirToCir* pObj = new QProfMeasCirToCir();

		cv::Point2f ptCirCenter1; double dRadius1 = 0;
		if (!m_pCtrl->fitCircle(m_profDatas, nProfDataIndex1, nProfDataIndex2, ptCirCenter1, dRadius1))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("拟合圆弧失败！"));
			delete pObj;
			return;
		}
		pObj->_ptCirCenter1 = ptCirCenter1;
		pObj->_dRadius1 = dRadius1;

		cv::Point2f ptCirCenter2; double dRadius2 = 0;
		if (!m_pCtrl->fitCircle(m_profDatas, nProfDataIndex3, nProfDataIndex4, ptCirCenter2, dRadius2))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("拟合圆弧失败！"));
			delete pObj;
			return;
		}		
		pObj->_ptCirCenter2 = ptCirCenter2;
		pObj->_dRadius2 = dRadius2;

		cv::Point2d pt1 = ptCirCenter1;
		cv::Point2d pt2 = ptCirCenter2;
		if (!m_pCtrl->calcPtToPtDist(pt1, pt2, pObj))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("计算两点的距离失败！"));
			delete pObj;
			return;
		}

		cv::Point ptLine1, ptLine2;
		ptLine1.x = nProfDataIndex1;
		ptLine1.y = nProfDataIndex2;
		ptLine2.x = nProfDataIndex3;
		ptLine2.y = nProfDataIndex4;

		pObj->addRange(ptLine1, MEASURE_DATA_TYPE_ARC, ptCirCenter1);
		pObj->addRange(ptLine2, MEASURE_DATA_TYPE_ARC, ptCirCenter2);

		m_curObj->addMeasObj(pObj);

		updateMeasure(ui.comboBox_selectMeasure->count());
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Profile"));
	}
}

void QVLProfileEditor::onCir2LnDist()
{
	if (m_curObj)
	{
		m_pProfView->clearMeasure();

		cv::Mat imgMat;
		captureProfDataImage(imgMat);

		m_pProfView->setImage(imgMat);
		m_pProfView->show();

		while (!m_pProfView->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		QVector<cv::Rect> rectRanges = m_pProfView->getMeasureRange();

		if (rectRanges.size() != 2)
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择2个圆弧区域！"));
			return;
		}

		int nProfDataIndex1 = 0;
		int nProfDataIndex2 = 0;
		int nProfDataIndex3 = 0;
		int nProfDataIndex4 = 0;

		nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(0).x);
		nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(0).x + rectRanges.at(0).width);
		nProfDataIndex3 = convetToProfileDataIndex(rectRanges.at(1).x);
		nProfDataIndex4 = convetToProfileDataIndex(rectRanges.at(1).x + rectRanges.at(1).width);

		QProfMeasCirToLn* pObj = new QProfMeasCirToLn();

		cv::Point2f ptCirCenter1; double dRadius1 = 0;
		if (!m_pCtrl->fitCircle(m_profDatas, nProfDataIndex1, nProfDataIndex2, ptCirCenter1, dRadius1))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("拟合圆弧失败！"));
			delete pObj;
			return;
		}
		pObj->_ptCirCenter = ptCirCenter1;
		pObj->_dRadius = dRadius1;

		bool bReversedFit = false;  double fSlope = 0, fIntercept = 0;
		if (!m_pCtrl->fitLine(m_profDatas, nProfDataIndex3, nProfDataIndex4, bReversedFit, fSlope, fIntercept))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("第一条线拟合失败！"));
			return;
		}

		cv::Point2d ptOrg = ptCirCenter1;
		if (!m_pCtrl->calcPtToLineDist(ptOrg, bReversedFit, fSlope, fIntercept, pObj))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("计算点到线的距离失败！"));
			delete pObj;
			return;
		}

		cv::Point ptLine1, ptLine2;
		ptLine1.x = nProfDataIndex1;
		ptLine1.y = nProfDataIndex2;
		ptLine2.x = nProfDataIndex3;
		ptLine2.y = nProfDataIndex4;

		pObj->addRange(ptLine1, MEASURE_DATA_TYPE_ARC, ptCirCenter1);
		pObj->addRange(ptLine2, MEASURE_DATA_TYPE_LINE);

		m_curObj->addMeasObj(pObj);

		updateMeasure(ui.comboBox_selectMeasure->count());
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Profile"));
	}
}

void QVLProfileEditor::onCir2PtDist()
{
	if (m_curObj)
	{
		m_pProfView->clearMeasure();

		cv::Mat imgMat;
		captureProfDataImage(imgMat);

		m_pProfView->setImage(imgMat);
		m_pProfView->show();

		while (!m_pProfView->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		QVector<cv::Rect> rectRanges = m_pProfView->getMeasureRange();

		if (rectRanges.size() != 2)
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择1个圆弧区域和一条交线！"));
			return;
		}

		int nProfDataIndex1 = 0;
		int nProfDataIndex2 = 0;
		int nProfDataIndex3 = 0;
		int nProfDataIndex4 = 0;

		nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(0).x);
		nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(0).x + rectRanges.at(0).width);
		nProfDataIndex3 = convetToProfileDataIndex(rectRanges.at(1).x);
		nProfDataIndex4 = convetToProfileDataIndex(rectRanges.at(1).x);

		QProfMeasCirToPt* pObj = new QProfMeasCirToPt();

		cv::Point2f ptCirCenter1; double dRadius1 = 0;
		if (!m_pCtrl->fitCircle(m_profDatas, nProfDataIndex1, nProfDataIndex2, ptCirCenter1, dRadius1))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("拟合圆弧失败！"));
			delete pObj;
			return;
		}
		pObj->_ptCirCenter = ptCirCenter1;
		pObj->_dRadius = dRadius1;	

		cv::Point2d pt1 = ptCirCenter1;
		cv::Point2d pt2 = m_profDatas[nProfDataIndex3];
		if (!m_pCtrl->calcPtToPtDist(pt1, pt2, pObj))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("计算两点的距离失败！"));
			delete pObj;
			return;
		}

		cv::Point ptLine1, ptLine2;
		ptLine1.x = nProfDataIndex1;
		ptLine1.y = nProfDataIndex2;
		ptLine2.x = nProfDataIndex3;
		ptLine2.y = nProfDataIndex4;

		pObj->addRange(ptLine1, MEASURE_DATA_TYPE_ARC, ptCirCenter1);
		pObj->addRange(ptLine2, MEASURE_DATA_TYPE_POINT);

		m_curObj->addMeasObj(pObj);

		updateMeasure(ui.comboBox_selectMeasure->count());
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Profile"));
	}
}

void QVLProfileEditor::onCrRadious()
{
	if (m_curObj)
	{
		m_pProfView->clearMeasure();

		cv::Mat imgMat;
		captureProfDataImage(imgMat);

		m_pProfView->setImage(imgMat);
		m_pProfView->show();

		while (!m_pProfView->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		QVector<cv::Rect> rectRanges = m_pProfView->getMeasureRange();

		if (rectRanges.size() != 1)
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择1个圆弧区域！"));
			return;
		}

		int nProfDataIndex1 = 0;
		int nProfDataIndex2 = 0;
		int nProfDataIndex3 = 0;
		int nProfDataIndex4 = 0;

		nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(0).x);
		nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(0).x + rectRanges.at(0).width);

		QProfMeasCirRadius* pObj = new QProfMeasCirRadius();

		cv::Point2f ptCirCenter; double dRadius = 0;
		if (!m_pCtrl->fitCircle(m_profDatas, nProfDataIndex1, nProfDataIndex2, ptCirCenter, dRadius))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("拟合圆弧失败！"));
			delete pObj;
			return;
		}	
		pObj->_dRadius = dRadius;
		pObj->_ptCirCenter = ptCirCenter;

		cv::Point ptLine1, ptLine2;
		ptLine1.x = nProfDataIndex1;
		ptLine1.y = nProfDataIndex2;
		ptLine2.x = nProfDataIndex3;
		ptLine2.y = nProfDataIndex4;

		pObj->addRange(ptLine1, MEASURE_DATA_TYPE_ARC, ptCirCenter);
		//pObj->addRange(ptLine2);

		m_curObj->addMeasObj(pObj);

		updateMeasure(ui.comboBox_selectMeasure->count());
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Profile"));
	}
}

void QVLProfileEditor::onProfArea()
{
	if (m_curObj)
	{
		m_pProfView->clearMeasure();

		cv::Mat imgMat;
		captureProfDataImage(imgMat);

		m_pProfView->setImage(imgMat);
		m_pProfView->show();

		while (!m_pProfView->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		QVector<cv::Rect> rectRanges = m_pProfView->getMeasureRange();

		if (rectRanges.size() != 1)
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择1个区域！"));
			return;
		}

		int nProfDataIndex1 = 0;
		int nProfDataIndex2 = 0;
		int nProfDataIndex3 = 0;
		int nProfDataIndex4 = 0;

		nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(0).x);
		nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(0).x + rectRanges.at(0).width);

		QProfMeasProfArea* pObj = new QProfMeasProfArea();

		double dArea = 0;
		if (!m_pCtrl->CrossSectionArea(m_profDatas, nProfDataIndex1, nProfDataIndex2, dArea))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("求区域面积失败！"));
			delete pObj;
			return;
		}
		pObj->setValue(dArea);
	
		cv::Point ptLine1, ptLine2;
		ptLine1.x = nProfDataIndex1;
		ptLine1.y = nProfDataIndex2;
		ptLine2.x = nProfDataIndex3;
		ptLine2.y = nProfDataIndex4;

		pObj->addRange(ptLine1, MEASURE_DATA_TYPE_LINE);
		//pObj->addRange(ptLine2);

		m_curObj->addMeasObj(pObj);

		updateMeasure(ui.comboBox_selectMeasure->count());
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Profile"));
	}
}

void QVLProfileEditor::onProfLength()
{
	if (m_curObj)
	{
		m_pProfView->clearMeasure();

		cv::Mat imgMat;
		captureProfDataImage(imgMat);

		m_pProfView->setImage(imgMat);
		m_pProfView->show();

		while (!m_pProfView->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		QVector<cv::Rect> rectRanges = m_pProfView->getMeasureRange();

		if (rectRanges.size() != 1)
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择1个区域！"));
			return;
		}

		int nProfDataIndex1 = 0;
		int nProfDataIndex2 = 0;
		int nProfDataIndex3 = 0;
		int nProfDataIndex4 = 0;

		nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(0).x);
		nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(0).x + rectRanges.at(0).width);

		QProfMeasProfLength* pObj = new QProfMeasProfLength();

		double dLength = 0;
		if (!m_pCtrl->CrossSectionProfLength(m_profDatas, nProfDataIndex1, nProfDataIndex2, dLength))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("求曲线长度失败！"));
			delete pObj;
			return;
		}
		pObj->setValue(dLength);

		cv::Point ptLine1, ptLine2;
		ptLine1.x = nProfDataIndex1;
		ptLine1.y = nProfDataIndex2;
		ptLine2.x = nProfDataIndex3;
		ptLine2.y = nProfDataIndex4;

		pObj->addRange(ptLine1, MEASURE_DATA_TYPE_LINE);
		//pObj->addRange(ptLine2);

		m_curObj->addMeasObj(pObj);

		updateMeasure(ui.comboBox_selectMeasure->count());
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Profile"));
	}
}

void QVLProfileEditor::onLn2LnAngle()
{
	if (m_curObj)
	{
		m_pProfView->clearMeasure();

		cv::Mat imgMat;
		captureProfDataImage(imgMat);

		m_pProfView->setImage(imgMat);
		m_pProfView->show();

		while (!m_pProfView->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		QVector<cv::Rect> rectRanges = m_pProfView->getMeasureRange();
		
		if (rectRanges.size() != 2)
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择两条线段区域！"));
			return;
		}

		int nProfDataIndex1 = convetToProfileDataIndex(rectRanges.at(0).x);
		int nProfDataIndex2 = convetToProfileDataIndex(rectRanges.at(0).x + rectRanges.at(0).width);

		Vision::PR_Line2f stLine1;
		if (!m_pCtrl->fitLine(m_profDatas, nProfDataIndex1, nProfDataIndex2, stLine1))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("第一条线拟合失败！"));
			return;
		}

		int nProfDataIndex3 = convetToProfileDataIndex(rectRanges.at(1).x);
		int nProfDataIndex4 = convetToProfileDataIndex(rectRanges.at(1).x + rectRanges.at(1).width);

		Vision::PR_Line2f stLine2;
		if (!m_pCtrl->fitLine(m_profDatas, nProfDataIndex3, nProfDataIndex4, stLine2))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("第二条线拟合失败！"));
			return;
		}

		QProfMeasLnToLnAngle* pObj = new QProfMeasLnToLnAngle();
		if (!m_pCtrl->calcTwoLineAngle(stLine1, stLine2, pObj))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("计算两条线的角度失败！"));
			delete pObj;
			return;
		}

		cv::Point ptLine1, ptLine2;
		ptLine1.x = nProfDataIndex1;
		ptLine1.y = nProfDataIndex2;
		ptLine2.x = nProfDataIndex3;
		ptLine2.y = nProfDataIndex4;

		pObj->addRange(ptLine1, MEASURE_DATA_TYPE_LINE);
		pObj->addRange(ptLine2, MEASURE_DATA_TYPE_LINE);

		m_curObj->addMeasObj(pObj);

		updateMeasure(ui.comboBox_selectMeasure->count());
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择Profile"));
	}	
}

void QVLProfileEditor::onProfileIndexChanged(int iIndex)
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	if (ui.comboBox_selectProfile->count() > 0)
	{
		int nIndex = ui.comboBox_selectProfile->currentIndex();

		if (nIndex >= 0)
		{
			QProfileObj* pObj = pData->getProfObj(nIndex);
			m_curObj = pObj;
			ui.lineEdit_profileName->setText(pObj->getName());			
		}
		else
		{
			m_curObj = NULL;
		}	

		displayObj();
		updateMeasure(0);
	}
}

void QVLProfileEditor::onAddProfile()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	if (m_profDatas.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("没有Profile数据，请重新提取轮廓线！"));
		return;
	}

	QString cellName = ui.lineEdit_profileName->text();

	if (cellName.isEmpty())
	{
		cellName = QString("Prof %1").arg(pData->getProfObjIndex() + 1);
	}
	else
	{
		for (int i = 0; i < pData->getProfObjNum(); i++)
		{
			if (cellName == pData->getProfObj(i)->getName())
			{
				if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
					QStringLiteral("名称已经存在，是否采用默认名称？"), QMessageBox::Ok, QMessageBox::Cancel))
				{
					cellName = QString("Prof %1").arg(pData->getProfObjIndex() + 1);
				}
				else
				{
					return;
				}
			}
		}
	}

	QProfileObj* pObj = new QProfileObj(pData->increaseProfObjIndex(), cellName);

	pObj->setCutingPosn(m_startPt.x, m_startPt.y, m_endPt.x, m_endPt.y);

	pObj->set3DImage(m_img3D);
	//QImage imageGray = QImage((uchar*)m_imgGray.data, m_imgGray.cols, m_imgGray.rows, ToInt(m_imgGray.step), QImage::Format_RGB888);
	pObj->setGrayImage(m_imgGrayCutting);

	pObj->setProfData(m_profDatas);

	pData->pushProfObj(pObj);

	ui.comboBox_selectProfile->addItem(QString("%1").arg(cellName));
	ui.comboBox_selectProfile->setCurrentIndex(ui.comboBox_selectProfile->count() - 1);
	updateProfileList();

	updateMeasure(0);
}

void QVLProfileEditor::onDeleteProfile()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	if (ui.comboBox_selectProfile->currentIndex() >= 0)
	{
		int nIndex = ui.comboBox_selectProfile->currentIndex();

		pData->deleteProfObj(nIndex);

		m_curObj = NULL;
		loadProfileIndex(0);
	}

	updateProfileList();

	updateMeasure(0);
}

void QVLProfileEditor::onMeasureIndexChanged(int iIndex)
{

}

void QVLProfileEditor::onAddMeasure()
{

}

void QVLProfileEditor::onDeleteMeasure()
{
	if (m_curObj)
	{
		if (ui.comboBox_selectMeasure->currentIndex() >= 0)
		{
			int nIndex = ui.comboBox_selectMeasure->currentIndex();
			m_curObj->removeMeasObj(nIndex);

			updateMeasure(0);
		}		
	}	
}

//bool  QVLProfileEditor::fitLine(int nProfDataIndex1, int nProfDataIndex2, Vision::PR_Line2f& stLine)
//{
//	Vision::PR_FIT_LINE_BY_POINT_CMD  stCmd;
//	for (int i = 0 + nProfDataIndex1; (i < nProfDataIndex2) && (i < m_profDatas.size()); i++)
//	{
//		stCmd.vecPoints.push_back(m_profDatas.at(i));
//	}
//	stCmd.enMethod = Vision::PR_FIT_METHOD::LEAST_SQUARE_REFINE;
//	stCmd.bPreprocessed = false;
//	stCmd.nThreshold = 20;
//	stCmd.enAttribute = Vision::PR_OBJECT_ATTRIBUTE::DARK;
//	stCmd.enRmNoiseMethod = Vision::PR_RM_FIT_NOISE_METHOD::ABSOLUTE_ERR;
//	stCmd.fErrTol = 10;
//
//	Vision::PR_FIT_LINE_BY_POINT_RPY stRpy;
//	Vision::VisionStatus retStatus = Vision::PR_FitLineByPoint(&stCmd, &stRpy);
//	if (retStatus == Vision::VisionStatus::OK)
//	{
//		stLine = stRpy.stLine;
//		return true;
//	}
//	return false;
//}

void QVLProfileEditor::captureProfDataImage(cv::Mat& matImg)
{
	//QImage img = m_pPlotProfile->grab(QRect(0, 0, 1070, 360)).toImage();
	//QImage img = m_pPlotProfile->toPixmap().toImage();
	//cv::Mat imgMat;
	//imgMat = cv::Mat(img.height(), img.width(), CV_8UC3, (void*)img.constBits(), img.bytesPerLine());
	//cv::cvtColor(imgMat, imgMat, CV_BGR2RGB);

	QString path = QApplication::applicationDirPath();
	path += "/3D/data/prof/";

	//m_plotSelects.clear();
	//setProfileData();
	//QApplication::processEvents();
	m_pPlotProfile->saveBmp(path + "profImgTemp.bmp");

	cv::Mat imgMat = cv::imread((path + "profImgTemp.bmp").toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);
	matImg = imgMat.clone();
}

void QVLProfileEditor::updateMeasure(int iIndex)
{
	loadProfileMeasIndex(iIndex);
	updateProfMeasList();
	displayObjMeasure();
}

void QVLProfileEditor::loadProfileIndex(int nIndex)
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	ui.comboBox_selectProfile->clear();

	//loadMapData();
	for (int i = 0; i < pData->getProfObjNum(); i++)
	{
		QProfileObj* pObj = pData->getProfObj(i);
		if (pObj)
		{
			ui.comboBox_selectProfile->addItem(QString("%1").arg(pObj->getName()));
		}
	}

	ui.comboBox_selectProfile->setCurrentIndex(nIndex);

	updateMeasure(0);
}

void QVLProfileEditor::loadProfileMeasIndex(int iIndex)
{
	ui.comboBox_selectMeasure->clear();

	if (m_curObj)
	{
		//loadMapData();
		for (int i = 0; i < m_curObj->getMeasObjNum(); i++)
		{
			QProfMeasureObj* pObj = m_curObj->getMeasObj(i);
			if (pObj)
			{
				ui.comboBox_selectMeasure->addItem(QString("%1").arg(pObj->getName()));
			}
		}

		ui.comboBox_selectMeasure->setCurrentIndex(iIndex);
	}
}

void QVLProfileEditor::displayObj()
{
	if (m_curObj)
	{
		QVector<cv::Point2d> profDatas = m_curObj->getProfData();
		setProfData(profDatas);

		QImage img3D = m_curObj->get3dImage();
		setProf3DImg(img3D);

		cv::Mat imgGray = m_curObj->getGrayImage();
		if (!imgGray.empty())
		{
			m_imgGrayCutting = imgGray.clone();
			//cv::Mat grayMat;
			//grayMat = cv::Mat(imgGray.height(), imgGray.width(), CV_8UC3, (void*)imgGray.constBits(), imgGray.bytesPerLine());
			//cv::cvtColor(grayMat, grayMat, CV_BGR2RGB);
			setImagGray(m_imgGrayCutting);
		}
		

		double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
		m_curObj->getCutingPosn(x1, y1, x2, y2);
		m_startPt.x = x1;
		m_startPt.y = y1;
		m_endPt.x = x2;
		m_endPt.y = y2;
	}
}

void QVLProfileEditor::displayObjMeasure()
{
	m_plotSelects.clear();
	m_plotSelectTypes.clear();
	m_plotSelectValues.clear();

	if (m_curObj)
	{
		for (int i = 0; i < m_curObj->getMeasObjNum(); i++)
		{
			QProfMeasureObj* pObj = m_curObj->getMeasObj(i);
			if (pObj)
			{
				for (int j = 0; j < pObj->getRangeNum(); j++)
				{
					m_plotSelects.push_back(pObj->getRange(j));
					m_plotSelectTypes.push_back(pObj->getRangeType(j));					
					m_plotSelectValues.push_back(pObj->getRangeValue(j));					
				}				
			}
		}

		setProfileData();
	}
}

void QVLProfileEditor::setHeightData(const cv::Mat& matHeight)
{
	m_3DMatHeight = matHeight;
}

void QVLProfileEditor::setProfGrayImg(cv::Mat& imgGray)
{
	m_imgGray = imgGray;

	if (m_imgGray.type() == CV_8UC3)
	{
		cvtColor(m_imgGray, m_imgGray, CV_BGR2RGB);
	}
	else if (m_imgGray.type() == CV_8UC1)
	{
		cvtColor(m_imgGray, m_imgGray, CV_GRAY2RGB);
	}
	
	setImagGray(m_imgGray);
}

void QVLProfileEditor::setProf3DImg(QImage& img3D)
{
	m_img3D = img3D;
	setImag3D(img3D);
}

void QVLProfileEditor::setProfData(QVector<cv::Point2d>& profDatas)
{
	m_profDatas = profDatas;

	setProfileData();
}

void QVLProfileEditor::setProfRange(double x1, double y1, double x2, double y2)
{
	double fScaleW = m_imgGray.size().width * 1.0 / m_img3D.width();
	double fScaleH = m_imgGray.size().height * 1.0 / m_img3D.height();

	m_startPt.x = x1 * fScaleW;
	m_startPt.y = y1 * fScaleH;
	m_endPt.x = x2 * fScaleW;
	m_endPt.y = y2 * fScaleH;

	setCutLine();
}

void QVLProfileEditor::prepareNewProf()
{
	//ui.comboBox_selectProfile->setCurrentIndex(-1);

	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	QString cellName = ui.lineEdit_profileName->text();

	if (cellName.isEmpty())
	{
		cellName = QString("Prof %1").arg(pData->getProfObjIndex() + 1);
	}
	else
	{
		for (int i = 0; i < pData->getProfObjNum(); i++)
		{
			if (cellName == pData->getProfObj(i)->getName())
			{				
				cellName = QString("Prof %1").arg(pData->getProfObjIndex() + 1);
				break;
			}
		}
	}

	QProfileObj* pObj = new QProfileObj(pData->increaseProfObjIndex(), cellName);

	pObj->setCutingPosn(m_startPt.x, m_startPt.y, m_endPt.x, m_endPt.y);

	pObj->set3DImage(m_img3D);
	//QImage imageGray = QImage((uchar*)m_imgGray.data, m_imgGray.cols, m_imgGray.rows, ToInt(m_imgGray.step), QImage::Format_RGB888);
	pObj->setGrayImage(m_imgGrayCutting);

	pObj->setProfData(m_profDatas);

	pData->pushProfObj(pObj);

	ui.comboBox_selectProfile->addItem(QString("%1").arg(cellName));
	ui.comboBox_selectProfile->setCurrentIndex(ui.comboBox_selectProfile->count() - 1);
	updateProfileList();

	updateMeasure(0);
}

int QVLProfileEditor::convetToProfileDataIndex(int mousePos)
{
	int nProfileDataSize = m_profDatas.size();

	int nWindowWidth = m_pProfView->getWindowWidth();

	int nLeftGap = 25;
	int nRightGap = 20;

	int nProfileWidth = nWindowWidth - nLeftGap - nRightGap;
	int nProfilePos = mousePos - nLeftGap;
	if (nProfilePos < 0)
	{
		nProfilePos = 0;
	}
	else if (nProfilePos >= nProfileWidth)
	{
		nProfilePos = nProfileWidth - 1;
	}
	double dProfilePosScale = (double)nProfilePos / nProfileWidth;

	return nProfileDataSize * dProfilePosScale;
}

void QVLProfileEditor::setImagGray(cv::Mat& imgMat)
{
	if (!imgMat.empty())
	{
		double fScaleW = ui.graphicsView_imgGray->width()*1.0 / imgMat.size().width;
		double fScaleH = ui.graphicsView_imgGray->height()*1.0 / imgMat.size().height;

		cv::Mat mat;
		cv::resize(imgMat, mat, cv::Size(imgMat.size().width*fScaleW, imgMat.size().height*fScaleH), (0, 0), (0, 0), 3);

		QImage image = QImage((uchar*)mat.data, mat.cols, mat.rows, ToInt(mat.step), QImage::Format_RGB888);
		m_imgGrayScene->addPixmap(QPixmap::fromImage(image));
	}	
}

void QVLProfileEditor::setImag3D(QImage& image)
{
	//double fScaleW = ui.graphicsView_img3D->width()*1.0 / image.width();
	//double fScaleH = ui.graphicsView_img3D->height()*1.0 / image.height();
	//int x1 = m_startPt.x;
	//int y1 = m_startPt.y;
	//int x2 = m_endPt.x;
	//int y2 = m_endPt.y;

	//int x = x1 < x2 ? x1 : x2;
	//int y = y1 < y2 ? y1 : y2;
	//int width = abs(x1 - x2);
	//int height = abs(y1 - y2);

	//qDebug() << "tt " << x << " " << y << " " << width << " " << height;

	//int x = 50;
	//int y = 150;
	//int width = image.width() - 50*2;
	//int height = image.height();
	//QImage img = image.copy(x, y, width, height);

	if (!image.isNull())
	{
		QImage img = image.scaled(QSize(ui.graphicsView_img3D->width(), ui.graphicsView_img3D->height()), Qt::IgnoreAspectRatio);
		m_img3DScene->addPixmap(QPixmap::fromImage(img));
	}		
}

void QVLProfileEditor::setCutLine()
{
	//double fScaleW = m_imgGray.size().width * 1.0 / m_img3D.width();
	//double fScaleH = m_imgGray.size().height * 1.0 / m_img3D.height();

	m_imgGrayCutting = m_imgGray.clone();

	int thickness = 3;
	int lineType = 8;

	cv::Point pt1, pt2;

	pt1.x = m_startPt.x;
	pt1.y = m_startPt.y;

	pt2.x = m_endPt.x;
	pt2.y = m_endPt.y;

	line(m_imgGrayCutting, pt1, pt2, cv::Scalar(255, 0, 0), thickness, lineType);

	setImagGray(m_imgGrayCutting);
}

void QVLProfileEditor::setProfileData()
{
	if (m_pPlotProfile)
	{
		delete m_pPlotProfile;
		m_pPlotProfile = new QCustomPlot(this);
		setupSincScatterDemo(m_pPlotProfile);
		ui.dockWidget_profileData->setWidget(m_pPlotProfile);
	}
}

void QVLProfileEditor::setupLineChartDemo(QCustomPlot *customPlot)
{
	//demoName = "Line Style Demo";
	customPlot->legend->setVisible(true);
	customPlot->legend->setFont(QFont("Helvetica", 9));
	QPen pen;
	QStringList lineNames;
	lineNames << "Profile";
	// add graphs with different line styles:
	for (int i = 0; i < 1; ++i)
	{
		customPlot->addGraph();
		pen.setColor(QColor(qSin(i * 1 + 1.2) * 80 + 80, qSin(i*0.3 + 0) * 80 + 80, qSin(i*0.3 + 1.5) * 80 + 80));
		customPlot->graph()->setPen(pen);
		customPlot->graph()->setName(lineNames.at(i));
		customPlot->graph()->setLineStyle(QCPGraph::LineStyle::lsLine);
		customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

		// generate data:
		int nDataCount = m_profDatas.size();		
		QVector<double> x(nDataCount), y(nDataCount);
		for (int j = 0; j < nDataCount; ++j)
		{
			x[j] = m_profDatas[j].x;
			y[j] = m_profDatas[j].y;
		}
		customPlot->graph()->setData(x, y);

		customPlot->graph()->rescaleAxes(true);
	}
	// zoom out a bit:
	customPlot->yAxis->scaleRange(1.1, customPlot->yAxis->range().center());
	customPlot->xAxis->scaleRange(1.1, customPlot->xAxis->range().center());
	// set blank axis lines:
	customPlot->xAxis->setTicks(false);
	customPlot->yAxis->setTicks(true);
	customPlot->xAxis->setTickLabels(false);
	customPlot->yAxis->setTickLabels(true);
	// make top right axes clones of bottom left axes:
	customPlot->axisRect()->setupFullAxesBox();
}

void QVLProfileEditor::setupSincScatterDemo(QCustomPlot *customPlot)
{
	customPlot->legend->setVisible(true);
	customPlot->legend->setFont(QFont("Helvetica", 9));
	// set locale to english, so we get english decimal separator:
	customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));

	// add theory curve graph:
	customPlot->addGraph();
	QPen pen;
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(0);
	pen.setColor(Qt::darkBlue);
	customPlot->graph(0)->setPen(pen);
	customPlot->graph(0)->setName("Profile");	

	// generate data:
	int nDataCount = m_profDatas.size();
	QVector<double> x(nDataCount), y(nDataCount);
	for (int j = 0; j < nDataCount; ++j)
	{
		x[j] = m_profDatas[j].x;
		y[j] = m_profDatas[j].y;
	}
	
	customPlot->graph(0)->setData(x, y);
	customPlot->graph(0)->rescaleAxes();
	customPlot->graph(0)->rescaleAxes(true);

	for (int i = 0; i < m_plotSelects.size(); i++)
	{
		// add theory curve graph:
		customPlot->addGraph();
		//pen.setColor(QColor(qSin(i * 1 + 1.2) * 80 + 80, qSin(i*0.3 + 0) * 80 + 80, qSin(i*0.3 + 1.5) * 80 + 80));
		pen.setColor(Qt::red);
		customPlot->graph()->setPen(pen);
		//customPlot->graph()->setName("Profile");
		customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 5));

		switch (m_plotSelectTypes[i])
		{
		case  0:
			customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 5));
			break;
		case  1:
			customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 8));
			break;
		case  2:
			customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 5));
			break;
		default:
			break;
		}

		// generate data:
		cv::Point ptLine = m_plotSelects[i];

		int nDataCount = ptLine.y - ptLine.x;
		if (nDataCount == 0)
		{
			nDataCount = 1;		
		}
		
		QVector<double> x(nDataCount), y(nDataCount);
		for (int j = 0; j < nDataCount; ++j)
		{
			int nIndex = ptLine.x + j;
			if (nIndex >= m_profDatas.size())
			{
				if (m_profDatas.size() > 0)
				{
					x[j] = m_profDatas[m_profDatas.size() - 1].x;
					y[j] = m_profDatas[m_profDatas.size() - 1].y;
				}
				else
				{
					x[j] = 0;
					y[j] = 0;
				}				
				continue;
			}

			x[j] = m_profDatas[nIndex].x;
			y[j] = m_profDatas[nIndex].y;
		}

		customPlot->graph()->setData(x, y);
		customPlot->graph()->rescaleAxes(true);	
	}
	if (m_plotSelects.size() > 0) customPlot->legend->setVisible(false);

	for (int i = 0; i < m_plotSelectValues.size(); i++)
	{
		if (2 == m_plotSelectTypes[i])
		{
			// add theory curve graph:
			customPlot->addGraph();
			//pen.setColor(QColor(qSin(i * 1 + 1.2) * 80 + 80, qSin(i*0.3 + 0) * 80 + 80, qSin(i*0.3 + 1.5) * 80 + 80));
			pen.setColor(Qt::darkGreen);
			customPlot->graph()->setPen(pen);
			//customPlot->graph()->setName("Profile");
			customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssPlusCircle, 8));

			// generate data:
			cv::Point ptValue = m_plotSelectValues[i];

			int nDataCount = 1;
			QVector<double> x(nDataCount), y(nDataCount);
			for (int j = 0; j < nDataCount; ++j)
			{
				x[j] = ptValue.x;
				y[j] = ptValue.y;
			}

			customPlot->graph()->setData(x, y);
			customPlot->graph()->rescaleAxes(true);
		}	
	}

	// zoom out a bit:
	customPlot->yAxis->scaleRange(1.5, customPlot->yAxis->range().center());
	customPlot->xAxis->scaleRange(1.0, customPlot->xAxis->range().center());
	// setup look of bottom tick labels:
	//customPlot->xAxis->setTickLabelRotation(30);
	//customPlot->xAxis->setAutoTickCount(9);
	//customPlot->xAxis->setNumberFormat("ebc");
	customPlot->xAxis->setNumberPrecision(1);
	//customPlot->xAxis->moveRange(-10);

	// make top right axes clones of bottom left axes. Looks prettier:
	customPlot->axisRect()->setupFullAxesBox();
}

void QVLProfileEditor::saveDataBase()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	pData->saveProfDataBase(QString(""));
}

void QVLProfileEditor::loadDataBase()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	m_curObj = NULL;

	pData->loadProfDataBase(QString(""));

	loadProfileIndex(0);

	updateProfileList();
}

void QVLProfileEditor::updateProfileList()
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	m_modelProf.clear();

	QStringList ls;
	ls << QStringLiteral("测量名称") << QStringLiteral("测量位置");
	m_modelProf.setHorizontalHeaderLabels(ls);

	ui.tableView_profileList->setColumnWidth(0, 70);
	ui.tableView_profileList->setColumnWidth(1, 200);

	for (int j = 0; j < pData->getProfObjNum(); j++)
	{
		QProfileObj* pObj = pData->getProfObj(j);
		if (pObj)
		{
			int nr = m_modelProf.rowCount();
			m_modelProf.insertRow(nr);
		
			m_modelProf.setData(m_modelProf.index(nr, 0), QStringLiteral("%1").arg(pObj->getName()));

			double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
			pObj->getCutingPosn(x1, y1, x2, y2);
			m_modelProf.setData(m_modelProf.index(nr, 1), QStringLiteral("Pt1=(%1,%2), Pt2=(%3,%4)").arg(x1).arg(y1).arg(x2).arg(y2));
		}		
	}
}

void QVLProfileEditor::updateProfMeasList()
{
	m_modelMeasure.clear();

	if (m_curObj)
	{
		QStringList ls;
		ls << QStringLiteral("测量类型") << QStringLiteral("测量数据");
		m_modelMeasure.setHorizontalHeaderLabels(ls);

		ui.tableView_measureList->setColumnWidth(0, 135);
		ui.tableView_measureList->setColumnWidth(1, 135);

		for (int j = 0; j < m_curObj->getMeasObjNum(); j++)
		{
			QProfMeasureObj* pObj = m_curObj->getMeasObj(j);
			if (pObj)
			{
				int nr = m_modelMeasure.rowCount();
				m_modelMeasure.insertRow(nr);

				m_modelMeasure.setData(m_modelMeasure.index(nr, 0), QStringLiteral("%1").arg(pObj->getName()));
				m_modelMeasure.setData(m_modelMeasure.index(nr, 1), QStringLiteral("Value = %1").arg(pObj->getValue()));
			}
		}
	}	
}

