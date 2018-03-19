#include "VisionDetectRunView.h"

#include <QFileDialog>

#include "visiondetect_global.h"

#include "../Common/SystemData.h"
#include "../include/IdDefine.h"
#include "../include/IVisionUI.h"
#include "../include/IMotion.h"
#include "../Common/eos.h"
#include <QMessageBox>
#include <QtMath>
#include <QThread>
#include <QDebug>
#include <QThreadPool>
#include <QDateTime>

#include "QDlpMTFRsltDisplay.h"

#include "../Common/ModuleMgr.h"
#include "../include/ICamera.h"
#include "../include/IDlp.h"

#include "CryptLib.h"

#include "../lib/VisionLibrary/include/VisionAPI.h"
#define ToInt(value)                (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace AOI;

//#define  SMART_DETECT_APP 

VisionDetectRunView::VisionDetectRunView(VisionCtrl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	QEos::Attach(EVENT_OBJ_STATE, this, SLOT(onObjEvent(const QVariantList &)));

	initUI();
	initLimits();	

	m_nImageRow = 0;
	m_nImageCol = 0;

	m_nMTFLocRecord1 = 0;
	m_nMTFLocRecord2 = 0;

	m_pDlpMTFDisplay = new QDlpMTFRsltDisplay();

	int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
	for (int i = 0; i < nStationNum; i++)
	{
		m_3DMatHeights.push_back(cv::Mat());
	}

	loadMTFData();

#ifdef SMART_DETECT_APP
	ui.tabWidget->removeTab(0);
	//ui.tabWidget->removeTab(1);
	ui.page_7->setVisible(false);
	ui.page_8->setVisible(false);
	ui.toolBox_3->removeItem(1);
	ui.toolBox_3->removeItem(1);
#endif

	QString user;
	int level = 0;
	System->getUser(user, level);
	m_nLevel = level;
	if (USER_LEVEL_MANAGER > level)
	{
		ui.tabWidget->removeTab(1);
	}
}

VisionDetectRunView::~VisionDetectRunView()
{
	/*if (m_pView)
	{
	delete m_pView;
	m_pView = NULL;
	}*/

	if (m_pVLMaskEditor)
	{
		delete m_pVLMaskEditor;
		m_pVLMaskEditor = NULL;
	}

	if (m_TmpMatchScene)
	{
		delete m_TmpMatchScene;
		m_TmpMatchScene = NULL;
	}

	if (m_TmpMatchMTFScene1)
	{
		delete m_TmpMatchMTFScene1;
		m_TmpMatchMTFScene1 = NULL;
	}

	if (m_TmpMatchMTFScene2)
	{
		delete m_TmpMatchMTFScene2;
		m_TmpMatchMTFScene2 = NULL;
	}

	if (m_pDlpMTFDisplay)
	{
		delete m_pDlpMTFDisplay;
		m_pDlpMTFDisplay = NULL;
	}

	if (inputIntRangePos)
	{
		delete inputIntRangePos;
		inputIntRangePos = NULL;
	}

	if (inputIntRangeAll)
	{
		delete inputIntRangeAll;
		inputIntRangeAll = NULL;
	}

	if (inputDoubleRangePos)
	{
		delete inputDoubleRangePos;
		inputDoubleRangePos = NULL;
	}

	if (inputDoubleRangeAll)
	{
		delete inputDoubleRangeAll;
		inputDoubleRangeAll = NULL;
	}
}

QWidget* VisionDetectRunView::getCellEditorView()
{
	return m_pVLCellObjEditor;
}

void VisionDetectRunView::initUI()
{
	m_pVLMaskEditor = new QVLMaskEditor();
	m_pVLCellObjEditor = m_pCtrl->getCellObjEditor();
	m_pVLProflieEditor = m_pCtrl->getProfileEditor();

	//线性检测
	connect(ui.comboBox_SrhLineDir, SIGNAL(currentIndexChanged(int)), SLOT(onSrhLineDirIndexChanged(int)));
	QStringList ls;
	ls << QStringLiteral("由暗边到亮边") << QStringLiteral("由亮边到暗边");
	ui.comboBox_SrhLineDir->addItems(ls);
	int nSrhLineDir = System->getParam("srh_line_dir").toInt();
	ui.comboBox_SrhLineDir->setCurrentIndex(nSrhLineDir);

	bool bSrhLineLinerity = System->getParam("srh_line_linerity_enable").toBool();
	double dSrhLinePointMaxOft = System->getParam("srh_line_point_max_oft").toDouble();
	double dSrhLineMinLinerity = System->getParam("srh_line_min_linerity").toDouble();
	ui.checkBox_SrhLineLinerity->setChecked(bSrhLineLinerity);
	ui.lineEdit_SrhLinePointMaxOft->setText(QString("%1").arg(dSrhLinePointMaxOft));
	ui.lineEdit_SrhLineMinLinerity->setText(QString("%1").arg(dSrhLineMinLinerity));

	bool bSrhLineAngle = System->getParam("srh_line_angle_enable").toBool();
	double dSrhLineExpectAngle = System->getParam("srh_line_expect_angle").toDouble();
	double dSrhLineAngleDiffTol = System->getParam("srh_line_angle_diff_tol").toDouble();
	ui.checkBox_SrhLineAngle->setChecked(bSrhLineAngle);
	ui.lineEdit_SrhLineExpectAngle->setText(QString("%1").arg(dSrhLineExpectAngle));
	ui.lineEdit_SrhLineAngleDiffTol->setText(QString("%1").arg(dSrhLineAngleDiffTol));

	connect(ui.pushButton_SrhLineROI, SIGNAL(clicked()), SLOT(onSrhLineROI()));
	connect(ui.pushButton_SrhLineDetect, SIGNAL(clicked()), SLOT(onSrhLineDetect()));
	connect(ui.pushButton_SaveDetectParam, SIGNAL(clicked()), SLOT(onSaveDetectParam()));

	//区域检测
	connect(ui.comboBox_TmpMatchObjMotion, SIGNAL(currentIndexChanged(int)), SLOT(onTmpMatchObjMotionIndexChanged(int)));
	ls.clear();
	ls << QStringLiteral("TRANSLATION") << QStringLiteral("EUCLIDEAN") << QStringLiteral("AFFINE") << QStringLiteral("HOMOGRAPHY");
	ui.comboBox_TmpMatchObjMotion->addItems(ls);
	int nTmpMatchObjMotion = System->getParam("srh_tmp_obj_motion").toInt();
	ui.comboBox_TmpMatchObjMotion->setCurrentIndex(nTmpMatchObjMotion);

	connect(ui.pushButton_TmpMatchSave, SIGNAL(clicked()), SLOT(onSaveTmpMatchParam()));

	connect(ui.pushButton_TmpMatchOpen, SIGNAL(clicked()), SLOT(onTmpMatchOpen()));
	connect(ui.pushButton_TmpMatchSelectROI, SIGNAL(clicked()), SLOT(onTmpMatchSelectROI()));
	connect(ui.pushButton_TmpMatchDeleteROI, SIGNAL(clicked()), SLOT(onTmpMatchDeleteROI()));
	connect(ui.pushButton_TmpMatchSelectDetect, SIGNAL(clicked()), SLOT(onTmpMatchSelectDetect()));
	connect(ui.pushButton_TmpMatchDetect, SIGNAL(clicked()), SLOT(onTmpMatchDetect()));

	m_TmpMatchScene = new QGraphicsScene();
	ui.graphicsView_TmpMatchImg->setScene(m_TmpMatchScene);
	ui.graphicsView_TmpMatchImg->setFixedSize(150, 150);
	ui.graphicsView_TmpMatchImg->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_TmpMatchImg->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	bool bTmpMatchSubPixel = System->getParam("srh_tmp_sub_pixel").toBool();
	ui.checkBox_TmpMatchSubPixel->setChecked(bTmpMatchSubPixel);
	connect(ui.checkBox_TmpMatchSubPixel, SIGNAL(stateChanged(int)), SLOT(onComBoxSubPixel(int)));

	//圆弧检测
	int nDetectEdgeThred1 = System->getParam("detect_edge_threshold1").toInt();
	int nDetectEdgeThred2 = System->getParam("detect_edge_threshold2").toInt();
	int nDetectEdgeApeSize = System->getParam("detect_edge_aperture_size").toInt();
	ui.lineEdit_DetectEdgeThreshold1->setText(QString("%1").arg(nDetectEdgeThred1));
	ui.lineEdit_DetectEdgeThreshold2->setText(QString("%1").arg(nDetectEdgeThred2));
	ui.lineEdit_DetectEdgeApeSize->setText(QString("%1").arg(nDetectEdgeApeSize));

	connect(ui.pushButton_SaveDetectEdgeParams, SIGNAL(clicked()), SLOT(onSaveDetectEdgeParams()));
	connect(ui.pushButton_DetectEdgeROI, SIGNAL(clicked()), SLOT(onDetectEdgeROI()));
	connect(ui.pushButton_DetectEdge, SIGNAL(clicked()), SLOT(onDetectEdge()));

	connect(ui.pushButton_SrhCircleROI, SIGNAL(clicked()), SLOT(onSrhCircleROI()));
	connect(ui.pushButton_SrhCirccleDetect, SIGNAL(clicked()), SLOT(onSrhCircleDetect()));

	//灰度检测
	int nDetectGrayScaleRow = System->getParam("detect_gray_scale_row").toInt();
	int nDetectGrayScaleCol = System->getParam("detect_gray_scale_col").toInt();
	ui.lineEdit_DetectGraySacleRow->setText(QString("%1").arg(nDetectGrayScaleRow));
	ui.lineEdit_DetectGrayScaleCol->setText(QString("%1").arg(nDetectGrayScaleCol));

	connect(ui.pushButton_DetectGrayScaleOpen, SIGNAL(clicked()), SLOT(onDetectGrayScaleOpen()));
	connect(ui.pushButton_SaveDetectGrayScaleParams, SIGNAL(clicked()), SLOT(onSaveDetectGrayScaleParams()));
	connect(ui.pushButton_DetectGrayScale, SIGNAL(clicked()), SLOT(onDetectGrayScale()));
	

	//3D检测测量
	connect(ui.comboBox_selectDLP, SIGNAL(currentIndexChanged(int)), SLOT(onDLPIndexChanged(int)));
	int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
	for (int i = 0; i < nStationNum; i++)
	{
		ui.comboBox_selectDLP->addItem(QString("%1").arg(QStringLiteral("DLP%1").arg(i + 1)));
	}
	ui.comboBox_selectDLP->setCurrentIndex(0);

	bool b3DDetectGaussionFilter = System->getParam("3d_detect_gaussion_filter").toBool();
	bool b3DDetectReverseSeq = System->getParam("3d_detect_reverse_seq").toBool();
	ui.checkBox_3DDetectGaussionFilter->setChecked(b3DDetectGaussionFilter);
	ui.checkBox_3DDetectReverseSeq->setChecked(b3DDetectReverseSeq);
	double d3DDetectMinIntDiff = System->getParam("3d_detect_min_intensity_diff").toDouble();
	ui.lineEdit_3DDetectMinIntDiff->setText(QString("%1").arg(d3DDetectMinIntDiff));

	double d3DDetectPhaseShift = System->getParam("3d_detect_phase_shift").toDouble();
	ui.doubleSpinBox_phaseShift->setValue(d3DDetectPhaseShift);
	updatePhaseShift(d3DDetectPhaseShift);
	connect(ui.doubleSpinBox_phaseShift, SIGNAL(valueChanged(double)), SLOT(onPhaseShiftValueChanged(double)));

	connect(ui.pushButton_3DDetectOpen, SIGNAL(clicked()), SLOT(on3DDetectOpen()));
	connect(ui.pushButton_3DDetect, SIGNAL(clicked()), SLOT(on3DDetect()));	
	connect(ui.pushButton_Save3DDetectParams, SIGNAL(clicked()), SLOT(onSave3DDetectParams()));

	double d3DDetectHeightDiffThd = System->getParam("3d_detect_height_diff_threshold").toDouble();
	ui.lineEdit_3DDetectHeightDiffThreshold->setText(QString("%1").arg(d3DDetectHeightDiffThd));
	double d3DDetectHeightNoiseThd = System->getParam("3d_detect_height_noise_threshold").toDouble();
	ui.lineEdit_3DDetectHeightNoiseThreshold->setText(QString("%1").arg(d3DDetectHeightNoiseThd));
	connect(ui.pushButton_3DDetectMerge, SIGNAL(clicked()), SLOT(on3DDetectMerge()));

	
	//3D 检测数据处理
	int nDataBaseMin = System->getParam("3d_detect_data_base_min").toInt();
	int nDataBaseMax = System->getParam("3d_detect_data_base_max").toInt();
	int nDataDetectMin = System->getParam("3d_detect_data_detect_min").toInt();
	int nDataDetectMax = System->getParam("3d_detect_data_detect_max").toInt();
	ui.lineEdit_DataBaseMin->setText(QString("%1").arg(nDataBaseMin));
	ui.lineEdit_DataBaseMax->setText(QString("%1").arg(nDataBaseMax));
	ui.lineEdit_DataDetectMin->setText(QString("%1").arg(nDataDetectMin));
	ui.lineEdit_DataDetectMax->setText(QString("%1").arg(nDataDetectMax));
	connect(ui.pushButton_SaveDataParams, SIGNAL(clicked()), SLOT(onSaveDataParams()));
	connect(ui.pushButton_3DHeightDetect, SIGNAL(clicked()), SLOT(on3DHeightDetect()));

	connect(ui.pushButton_3DHeightCellObjEdit, SIGNAL(clicked()), SLOT(on3DHeightCellObjEdit()));
	connect(ui.pushButton_3DProfileEdit, SIGNAL(clicked()), SLOT(on3DProfileEdit()));
	

	// MTF 检测
	connect(ui.pushButton_editMTFLoc1, SIGNAL(clicked()), SLOT(onEditMTFLoc1()));
	connect(ui.pushButton_editMTFLocLearn1, SIGNAL(clicked()), SLOT(onEditMTFLocLearn1()));
	connect(ui.pushButton_searchMTFLoc1, SIGNAL(clicked()), SLOT(onSearchMTFLoc1()));
	connect(ui.pushButton_editMTFDetect1, SIGNAL(clicked()), SLOT(onEditMTFDetect1()));
	connect(ui.pushButton_editMTFLoc2, SIGNAL(clicked()), SLOT(onEditMTFLoc2()));
	connect(ui.pushButton_editMTFLocLearn2, SIGNAL(clicked()), SLOT(onEditMTFLocLearn2()));
	connect(ui.pushButton_searchMTFLoc2, SIGNAL(clicked()), SLOT(onSearchMTFLoc2()));
	connect(ui.pushButton_editMTFDetect2, SIGNAL(clicked()), SLOT(onEditMTFDetect2()));
	connect(ui.pushButton_calcMTF, SIGNAL(clicked()), SLOT(onCalcMTF()));
	connect(ui.pushButton_loadMTFFile, SIGNAL(clicked()), SLOT(onLoadMTFFile()));
	connect(ui.pushButton_saveMTFFile, SIGNAL(clicked()), SLOT(onSaveMTFFile()));

	m_TmpMatchMTFScene1 = new QGraphicsScene();
	ui.graphicsView_TmpMatchImg_MTF1->setScene(m_TmpMatchMTFScene1);
	ui.graphicsView_TmpMatchImg_MTF1->setFixedSize(120, 120);
	ui.graphicsView_TmpMatchImg_MTF1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_TmpMatchImg_MTF1->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_TmpMatchMTFScene2 = new QGraphicsScene();
	ui.graphicsView_TmpMatchImg_MTF2->setScene(m_TmpMatchMTFScene2);
	ui.graphicsView_TmpMatchImg_MTF2->setFixedSize(120, 120);
	ui.graphicsView_TmpMatchImg_MTF2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_TmpMatchImg_MTF2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	ui.lineEdit_BigPatAbsMtfV->setText(QString::number(0.0, 'f', 5));
	ui.lineEdit_BigPatAbsMtfH->setText(QString::number(0.0, 'f', 5));
	ui.lineEdit_SmallPatAbsMtfV->setText(QString::number(0.0, 'f', 5));
	ui.lineEdit_SmallPatAbsMtfH->setText(QString::number(0.0, 'f', 5));
	ui.lineEdit_SmallPatRelMtfV->setText(QString::number(0.0, 'f', 5));
	ui.lineEdit_SmallPatRelMtfH->setText(QString::number(0.0, 'f', 5));

	// DLP MTF 检测	
	double dMagnitudeOfDLP = System->getParam("dlp_mtf_magnitude").toDouble();
	ui.lineEdit_dlpMTFMagnitudeOfDLP->setText(QString("%1").arg(dMagnitudeOfDLP));


	connect(ui.pushButton_dlpMTFOpen, SIGNAL(clicked()), SLOT(onDlpMTFOpen()));
	connect(ui.pushButton_SaveDlpMTFParams, SIGNAL(clicked()), SLOT(onSaveDlpMTFParams()));
	connect(ui.pushButton_DetectDlpMTF, SIGNAL(clicked()), SLOT(onDetectDlpMTF()));

	// DLP PD 检测
	bool bDlpPDReverseSeq = System->getParam("dlp_pd_reverse_seq").toBool();
	int nDlpPDSizeX = System->getParam("dlp_pd_pattern_size_x").toInt();
	int nDlpPDSizeY = System->getParam("dlp_pd_pattern_size_y").toInt();
	int nDlpPDGaussianSize = System->getParam("dlp_pd_gaussian_size").toInt();
	double dMagnitudeOfDlpPD = System->getParam("dlp_pd_magnitude").toDouble();
	double dDlpPDPixelCycle = System->getParam("dlp_pd_pixel_cycle").toDouble();
	double dDlpPDGaussianSigma = System->getParam("dlp_pd_gaussian_sigma").toDouble();

	ui.checkBox_dlpPDReverseSeq->setChecked(bDlpPDReverseSeq);
	ui.lineEdit_dlpPDPatternSizeX->setText(QString("%1").arg(nDlpPDSizeX));
	ui.lineEdit_dlpPDPatternSizeY->setText(QString("%1").arg(nDlpPDSizeY));
	ui.lineEdit_dlpPDGaussianSize->setText(QString("%1").arg(nDlpPDGaussianSize));
	ui.lineEdit_dlpPDMagnitudeOfDLP->setText(QString("%1").arg(dMagnitudeOfDlpPD));
	ui.lineEdit_dlpPDPixelCycle->setText(QString("%1").arg(dDlpPDPixelCycle));
	ui.lineEdit_dlpPDGaussianSigma->setText(QString("%1").arg(dDlpPDGaussianSigma));


	connect(ui.pushButton_dlpPDOpen, SIGNAL(clicked()), SLOT(onDlpPDOpen()));
	connect(ui.pushButton_DetectDlpPD, SIGNAL(clicked()), SLOT(onDetectDlpPD()));
}

void VisionDetectRunView::initLimits()
{
	//输入限制
	inputIntRangePos = new QIntValidator(0, 1000, this);
	inputIntRangeAll = new QIntValidator(-1000, 1000, this);
	inputDoubleRangePos = new QDoubleValidator(0, 1000, 10, this);
	inputDoubleRangeAll = new QDoubleValidator(-1000, 1000, 10, this);

	ui.lineEdit_SrhLinePointMaxOft->setValidator(inputDoubleRangeAll);
	ui.lineEdit_SrhLineMinLinerity->setValidator(inputDoubleRangeAll);
	ui.lineEdit_SrhLineExpectAngle->setValidator(inputDoubleRangeAll);
	ui.lineEdit_SrhLineAngleDiffTol->setValidator(inputDoubleRangeAll);

	ui.lineEdit_DetectEdgeThreshold1->setValidator(inputIntRangePos);
	ui.lineEdit_DetectEdgeThreshold2->setValidator(inputIntRangePos);
	ui.lineEdit_DetectEdgeApeSize->setValidator(inputIntRangePos);

	ui.lineEdit_DetectGraySacleRow->setValidator(inputIntRangePos);
	ui.lineEdit_DetectGrayScaleCol->setValidator(inputIntRangePos);

	ui.lineEdit_dlpMTFMagnitudeOfDLP->setValidator(inputDoubleRangeAll);

	ui.lineEdit_dlpPDMagnitudeOfDLP->setValidator(inputDoubleRangeAll);
	ui.lineEdit_dlpPDPixelCycle->setValidator(inputDoubleRangeAll);
	ui.lineEdit_dlpPDPatternSizeX->setValidator(inputIntRangePos);
	ui.lineEdit_dlpPDPatternSizeY->setValidator(inputIntRangePos);
	ui.lineEdit_dlpPDGaussianSize->setValidator(inputIntRangePos);
	ui.lineEdit_dlpPDGaussianSigma->setValidator(inputDoubleRangeAll);

	ui.lineEdit_3DDetectMinIntDiff->setValidator(inputDoubleRangeAll);
	ui.lineEdit_3DDetectHeightDiffThreshold->setValidator(inputDoubleRangeAll);
	ui.lineEdit_3DDetectHeightNoiseThreshold->setValidator(inputDoubleRangeAll);

	ui.lineEdit_DataBaseMin->setValidator(inputIntRangePos);
	ui.lineEdit_DataBaseMax->setValidator(inputIntRangePos);
	ui.lineEdit_DataDetectMin->setValidator(inputIntRangePos);
	ui.lineEdit_DataDetectMax->setValidator(inputIntRangePos);
}

IVisionUI* VisionDetectRunView::getVisionUI()
{
	return getModule<IVisionUI>(UI_MODEL);
}

void VisionDetectRunView::onObjEvent(const QVariantList &data)
{
	int iBoard = data[0].toInt();
	int iEvent = data[1].toInt();
	if (iEvent != RUN_OBJ_EDITOR && iEvent != RUN_OBJ_PROFILE && iEvent != RUN_OBJ_PROFILE_EDIT)return;
	int nRowIndex = data[2].toInt();

	if (RUN_OBJ_PROFILE == iEvent)
	{
		m_pVLProflieEditor->show();
	}
	else if (RUN_OBJ_PROFILE_EDIT == iEvent)
	{
		m_pVLProflieEditor->show();
	}
}

void VisionDetectRunView::onSrhLineDirIndexChanged(int iIndex)
{
	int nSrhLineDir = ui.comboBox_SrhLineDir->currentIndex();
}

void VisionDetectRunView::onSrhLineROI()
{
	cv::Mat matImage = getVisionUI()->getImage();

	if (!matImage.empty())
	{
		bool bClearAll = false;
		if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
			QStringLiteral("清除选择框？"), QMessageBox::Ok, QMessageBox::Cancel))
		{
			bClearAll = true;
		}
		m_pVLMaskEditor->setImage(matImage, bClearAll);
		m_pVLMaskEditor->repaintAll();
		m_pVLMaskEditor->setEditorView();
		m_pVLMaskEditor->setSelectRtRect();
		m_pVLMaskEditor->show();

	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
	}
}

void VisionDetectRunView::onSrhLineDetect()
{
	cv::Mat matImage = getVisionUI()->getImage().clone();

	if (!matImage.empty())
	{
		int nSrhLineDir = ui.comboBox_SrhLineDir->currentIndex();

		//m_pView->clearImage();

		//Mat gray = matImage.clone();
		//cv::cvtColor(gray, gray, CV_RGB2GRAY);//灰度化
		//Mat hole(gray.size(), CV_8U, Scalar(0)); //遮罩图层
		////cv::drawContours(hole, contours1, -1, Scalar(255), CV_FILLED); //在遮罩图层上，用白色像素填充轮廓 
		//m_mainView->setImage(hole);

		//PR_Caliper is replaced by PR_FindLine.
		//Vision::PR_CALIPER_CMD  stCmd;
		//stCmd.matInputImg = matImage;
		//stCmd.rectRotatedROI = m_pVLMaskEditor->getSelect();
		//stCmd.matMask = m_pVLMaskEditor->getMaskImage();
		//stCmd.enDetectDir = nSrhLineDir ? Vision::PR_CALIPER_DIR::DARK_TO_BRIGHT : Vision::PR_CALIPER_DIR::BRIGHT_TO_DARK;
		//stCmd.enAlgorithm = Vision::PR_CALIPER_ALGORITHM::PROJECTION;

		//bool bSrhLineLinerity = ui.checkBox_SrhLineLinerity->isChecked();
		//if (bSrhLineLinerity)
		//{
		//	double dSrhLinePointMaxOft = ui.lineEdit_SrhLinePointMaxOft->text().toDouble();
		//	double dSrhLineMinLinerity = ui.lineEdit_SrhLineMinLinerity->text().toDouble();

		//	stCmd.bCheckLinerity = bSrhLineLinerity;
		//	stCmd.fPointMaxOffset = convertToPixel(dSrhLinePointMaxOft);
		//	stCmd.fMinLinerity = dSrhLineMinLinerity;
		//}


		//bool bSrhLineAngle = ui.checkBox_SrhLineAngle->isChecked();
		//if (bSrhLineAngle)
		//{
		//	double dSrhLineExpectAngle = ui.lineEdit_SrhLineExpectAngle->text().toDouble();
		//	double dSrhLineAngleDiffTol = ui.lineEdit_SrhLineAngleDiffTol->text().toDouble();

		//	stCmd.bCheckAngle = bSrhLineAngle;
		//	stCmd.fExpectedAngle = dSrhLineExpectAngle;
		//	stCmd.fAngleDiffTolerance = dSrhLineAngleDiffTol;
		//}

		//Vision::PR_CALIPER_RPY stRpy;
		//Vision::VisionStatus retStatus = Vision::PR_Caliper(&stCmd, &stRpy);
		//if (retStatus == Vision::VisionStatus::OK)
		//{
		//	m_pView->displayImage(stRpy.matResultImg);

		//	ui.lineEdit_SrhLineSlope->setText(QString("%1").arg(stRpy.fSlope));
		//	ui.lineEdit_SrhLineIntercept->setText(QString("%1").arg(convertToUm(stRpy.fIntercept)));
		//	ui.checkBox_SrhLineLinerityRst->setChecked(stRpy.bLinerityCheckPass);
		//	ui.checkBox_SrhLineAngleRst->setChecked(stRpy.bAngleCheckPass);

		//	QString szMessage = QString("Search Line formula: %1").arg(stRpy.bReversedFit ? QString("x = fSlope * y + fIntercept") : QString("y = fSlope * x + fIntercept"));
		//	System->setTrackInfo(szMessage);

		//	szMessage = QString("Search Line angle: %1").arg(qAtan(stRpy.fSlope) * 180 / M_PI);
		//	System->setTrackInfo(szMessage);
		//}
		//else
		//{
		//	m_pView->addImageText(QString("Error at CALIPER, error code = %1").arg((int)retStatus));

		//	ui.lineEdit_SrhLineSlope->setText(QString("%1").arg(0.0));
		//	ui.lineEdit_SrhLineIntercept->setText(QString("%1").arg(0.0));
		//	ui.checkBox_SrhLineLinerityRst->setChecked(false);
		//	ui.checkBox_SrhLineAngleRst->setChecked(false);

		//}
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
	}
}

void VisionDetectRunView::onSaveDetectParam()
{
	int nSrhLineDir = ui.comboBox_SrhLineDir->currentIndex();
	System->setParam("srh_line_dir", nSrhLineDir);

	bool bSrhLineLinerity = ui.checkBox_SrhLineLinerity->isChecked();
	System->setParam("srh_line_linerity_enable", bSrhLineLinerity);
	double dSrhLinePointMaxOft = ui.lineEdit_SrhLinePointMaxOft->text().toDouble();
	System->setParam("srh_line_point_max_oft", dSrhLinePointMaxOft);
	double dSrhLineMinLinerity = ui.lineEdit_SrhLineMinLinerity->text().toDouble();
	System->setParam("srh_line_min_linerity", dSrhLineMinLinerity);

	bool bSrhLineAngle = ui.checkBox_SrhLineAngle->isChecked();
	System->setParam("srh_line_angle_enable", bSrhLineAngle);
	double dSrhLineExpectAngle = ui.lineEdit_SrhLineExpectAngle->text().toDouble();
	System->setParam("srh_line_expect_angle", dSrhLineExpectAngle);
	double dSrhLineAngleDiffTol = ui.lineEdit_SrhLineAngleDiffTol->text().toDouble();
	System->setParam("srh_line_angle_diff_tol", dSrhLineAngleDiffTol);
}

double VisionDetectRunView::convertToPixel(double umValue)
{
	double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	double dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

	double dResolution = (dResolutionX + dResolutionY) / 2;

	if (qAbs(dResolution) > 0.0001)
	{
		return (umValue / dResolution);
	}
	return 0.0;
}

double VisionDetectRunView::convertToUm(double pixel)
{
	double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	double dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

	double dResolution = (dResolutionX + dResolutionY) / 2;

	return pixel*dResolution;
}

void VisionDetectRunView::onTmpMatchObjMotionIndexChanged(int iIndex)
{
	int nIndex = ui.comboBox_TmpMatchObjMotion->currentIndex();
}

void VisionDetectRunView::onSaveTmpMatchParam()
{
	int nTmpMatchObjMotion = ui.comboBox_TmpMatchObjMotion->currentIndex();
	System->setParam("srh_tmp_obj_motion", nTmpMatchObjMotion);
}

void VisionDetectRunView::onTmpMatchOpen()
{
	QString path = QApplication::applicationDirPath();
	path += "/";

	QString picFilter = "Image(*.tif *.tiff *.gif *.bmp *.jpg *.jpeg *.jp2 *.png *.pcx *.pgm *.ppm *.pbm *.xwd *.ima)";
	QString strFileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开图片"), "/"/*path*/, picFilter);

	if (!strFileName.isEmpty())
	{
		//m_tmpMatchImg = imread(strFileName.toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);
		ui.lineEdit_TmpMatchFile->setText(QString("%1").arg(strFileName));
		m_tmpMatchImg.release();
	}
}

void VisionDetectRunView::onTmpMatchSelectROI()
{
	QString strFileName = ui.lineEdit_TmpMatchFile->text();

	if (!strFileName.isEmpty() && QFile(strFileName).exists())
	{
		m_tmpMatchImg = cv::imread(strFileName.toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);

		bool bClearAll = false;
		if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
			QStringLiteral("清除选择框？"), QMessageBox::Ok, QMessageBox::Cancel))
		{
			bClearAll = true;
		}

		m_pVLMaskEditor->setImage(m_tmpMatchImg, bClearAll);
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
			cv::Rect rectROILearn;
			RotatedRect rectRt = m_pVLMaskEditor->getSelect();
			if (rectRt.angle != 0)
			{
				Mat matAffi = getRotationMatrix2D(rectRt.center, rectRt.angle, 1.0);

				RotatedRect rectROI = rectRt;
				rectROI.angle = 0;

				cv::Mat matSrcImage = m_tmpMatchImg.clone();
				cv::Mat matDstImage;
				warpAffine(matSrcImage, matDstImage, matAffi, matSrcImage.size());

				//m_pView->displayImage(matDstImage);

				m_tmpMatchImg = matDstImage;
				m_tmpMatchROI = matDstImage(rectROI.boundingRect());
				rectROILearn = rectROI.boundingRect();
			}
			else
			{
				m_tmpMatchROI = m_tmpMatchImg(rectRt.boundingRect());
				rectROILearn = rectRt.boundingRect();

			}

			Vision::PR_LRN_TEMPLATE_CMD  stCmd;
			stCmd.matInputImg = m_tmpMatchImg;
			stCmd.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::HIERARCHICAL_EDGE;
			stCmd.rectROI = rectROILearn;

			Vision::PR_LRN_TEMPLATE_RPY stRpy;
			Vision::VisionStatus retStatus = Vision::PR_LrnTmpl(&stCmd, &stRpy);
			if (retStatus == Vision::VisionStatus::OK)
			{
				m_tmpRecordID = stRpy.nRecordId;
			}
			else
			{
				getVisionUI()->addImageText(QString("Error at Learn Template Match, error code = %1").arg((int)retStatus));
			}

			double fScaleW = ui.graphicsView_TmpMatchImg->width()*1.0 / m_tmpMatchROI.size().width;
			double fScaleH = ui.graphicsView_TmpMatchImg->height()*1.0 / m_tmpMatchROI.size().height;

			cv::Mat mat;
			cv::resize(m_tmpMatchROI, mat, cv::Size(m_tmpMatchROI.size().width*fScaleW, m_tmpMatchROI.size().height*fScaleH), (0, 0), (0, 0), 3);

			QImage image = QImage((uchar*)mat.data, mat.cols, mat.rows, ToInt(mat.step), QImage::Format_RGB888);
			m_TmpMatchScene->addPixmap(QPixmap::fromImage(image));
			//ui.graphicsView_TmpMatchImg->resize(image.width(), image.height());
			//ui.graphicsView_TmpMatchImg->show();			
		}
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
	}
}

void VisionDetectRunView::onTmpMatchDeleteROI()
{
	m_tmpMatchROI.release();
	m_TmpMatchScene->clear();
	//ui.graphicsView_TmpMatchImg->show();
}

void VisionDetectRunView::onTmpMatchSelectDetect()
{
	if (m_tmpMatchROI.empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择模板ROI区域"));
		return;
	}

	cv::Mat matImage = getVisionUI()->getImage();
	if (!matImage.empty())
	{
		bool bClearAll = false;
		if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
			QStringLiteral("清除选择框？"), QMessageBox::Ok, QMessageBox::Cancel))
		{
			bClearAll = true;
		}

		m_pVLMaskEditor->setImage(matImage, bClearAll);
		m_pVLMaskEditor->repaintAll();
		m_pVLMaskEditor->setEditorView(true, false);
		m_pVLMaskEditor->setSelectRtRect(true);
		m_pVLMaskEditor->show();

		while (!m_pVLMaskEditor->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		if (m_pVLMaskEditor->getSelectNum() > 0)
		{
			m_tmpMatchRect = m_pVLMaskEditor->getSelectRect();
		}
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
	}
}

void VisionDetectRunView::onTmpMatchDetect()
{
	if (m_tmpMatchROI.empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择模板ROI区域"));
		return;
	}

	cv::Mat matImage = getVisionUI()->getImage().clone();
	if (!matImage.empty())
	{
		bool bTmpMatchSubPixel = System->getParam("srh_tmp_sub_pixel").toBool();

		Vision::PR_MATCH_TEMPLATE_CMD  stCmd;
		stCmd.matInputImg = matImage;
		stCmd.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::HIERARCHICAL_EDGE;
		stCmd.nRecordId = m_tmpRecordID;

		if (m_tmpMatchRect.width == 0 || m_tmpMatchRect.height == 0)
		{
			m_tmpMatchRect.x = 0;
			m_tmpMatchRect.y = 0;
			m_tmpMatchRect.width = matImage.size().width;
			m_tmpMatchRect.height = matImage.size().height;
		}

		stCmd.rectSrchWindow = m_tmpMatchRect;
		stCmd.bSubPixelRefine = bTmpMatchSubPixel;

		int nTmpMatchObjMotion = ui.comboBox_TmpMatchObjMotion->currentIndex();
		stCmd.enMotion = (Vision::PR_OBJECT_MOTION)nTmpMatchObjMotion;

		Vision::PR_MATCH_TEMPLATE_RPY stRpy;
		Vision::VisionStatus retStatus = Vision::PR_MatchTmpl(&stCmd, &stRpy);

		QString path = QApplication::applicationDirPath();
		path += "/Vision/";
		Vision::PR_DumpTimeLog(path.toStdString() + "timelog.log");

		if (retStatus == Vision::VisionStatus::OK)
		{
			RotatedRect rectROI;
			rectROI.center = stRpy.ptObjPos;
			rectROI.size = m_tmpMatchROI.size();
			rectROI.angle = -stRpy.fRotation;

			Point2f vertices[4];
			rectROI.points(vertices);

			for (int i = 0; i < 4; i++)
			{
				line(stRpy.matResultImg, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0), 1);
			}

			cv::Point pt1, pt2;
			pt1.x = m_tmpMatchRect.x;
			pt1.y = m_tmpMatchRect.y;
			pt2.x = pt1.x + m_tmpMatchRect.width;
			pt2.y = pt1.y + m_tmpMatchRect.height;
			cv::rectangle(stRpy.matResultImg, pt1, pt2, Scalar(255, 0, 0), 1);

			getVisionUI()->displayImage(stRpy.matResultImg);

			ui.lineEdit_TmpMatchPosX->setText(QString("%1").arg(convertToUm(stRpy.ptObjPos.x)));
			ui.lineEdit_TmpMatchPosY->setText(QString("%1").arg(convertToUm(stRpy.ptObjPos.y)));
			ui.lineEdit_TmpMatchRotated->setText(QString("%1").arg(-stRpy.fRotation));
			ui.lineEdit_TmpMatchScore->setText(QString("%1").arg(stRpy.fMatchScore));
		}
		else
		{
			getVisionUI()->addImageText(QString("Error at Template Match, error code = %1").arg((int)retStatus));

			ui.lineEdit_TmpMatchPosX->setText(QString("%1").arg(0.0));
			ui.lineEdit_TmpMatchPosY->setText(QString("%1").arg(0.0));
			ui.lineEdit_TmpMatchRotated->setText(QString("%1").arg(0.0));
			ui.lineEdit_TmpMatchScore->setText(QString("%1").arg(0.0));
		}
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
	}
}

void VisionDetectRunView::onComBoxSubPixel(int iState)
{
	if (iState == Qt::Checked)
	{
		System->setParam("srh_tmp_sub_pixel", true);
	}
	else
	{
		System->setParam("srh_tmp_sub_pixel", false);
	}
}


void VisionDetectRunView::onSaveDetectEdgeParams()
{
	int nDetectEdgeThred1 = ui.lineEdit_DetectEdgeThreshold1->text().toInt();
	int nDetectEdgeThred2 = ui.lineEdit_DetectEdgeThreshold2->text().toInt();
	int nDetectEdgeApeSize = ui.lineEdit_DetectEdgeApeSize->text().toInt();

	System->setParam("detect_edge_threshold1", nDetectEdgeThred1);
	System->setParam("detect_edge_threshold2", nDetectEdgeThred2);
	System->setParam("detect_edge_aperture_size", nDetectEdgeApeSize);
}

void VisionDetectRunView::onDetectEdgeROI()
{
	cv::Mat matImage = getVisionUI()->getImage();

	if (!matImage.empty())
	{
		bool bClearAll = false;
		if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
			QStringLiteral("清除选择框？"), QMessageBox::Ok, QMessageBox::Cancel))
		{
			bClearAll = true;
		}
		m_pVLMaskEditor->setImage(matImage, bClearAll);
		m_pVLMaskEditor->repaintAll();
		m_pVLMaskEditor->setEditorView(true, false);
		m_pVLMaskEditor->setSelectRtRect(true);
		m_pVLMaskEditor->show();

		while (!m_pVLMaskEditor->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		if (m_pVLMaskEditor->getSelectNum() > 0)
		{
			m_detectEdgeROI = m_pVLMaskEditor->getSelectRect();
			m_srhCircleROI = m_detectEdgeROI;
		}
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
	}
}

void VisionDetectRunView::onDetectEdge()
{
	cv::Mat matImage = getVisionUI()->getImage().clone();
	if (!matImage.empty())
	{
		Vision::PR_DETECT_EDGE_CMD  stCmd;
		stCmd.matInputImg = matImage;

		if (m_detectEdgeROI.width == 0 || m_detectEdgeROI.height == 0)
		{
			m_detectEdgeROI.x = 0;
			m_detectEdgeROI.y = 0;
			m_detectEdgeROI.width = matImage.size().width;
			m_detectEdgeROI.height = matImage.size().height;
		}
		stCmd.rectROI = m_detectEdgeROI;

		int nDetectEdgeThred1 = ui.lineEdit_DetectEdgeThreshold1->text().toInt();
		int nDetectEdgeThred2 = ui.lineEdit_DetectEdgeThreshold2->text().toInt();
		int nDetectEdgeApeSize = ui.lineEdit_DetectEdgeApeSize->text().toInt();

		stCmd.nThreshold1 = nDetectEdgeThred1;
		stCmd.nThreshold2 = nDetectEdgeThred2;
		stCmd.nApertureSize = convertToPixel(nDetectEdgeApeSize);

		Vision::PR_DETECT_EDGE_RPY stRpy;
		Vision::VisionStatus retStatus = Vision::PR_DetectEdge(&stCmd, &stRpy);
		if (retStatus == Vision::VisionStatus::OK)
		{
			m_detectEdgeImg = stRpy.matResultImg;
			getVisionUI()->displayImage(stRpy.matResultImg);
		}
		else
		{
			getVisionUI()->addImageText(QString("Error at Template Match, error code = %1").arg((int)retStatus));
		}
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
	}
}

void VisionDetectRunView::onSrhCircleROI()
{
	cv::Mat matImage = getVisionUI()->getImage();

	if (!matImage.empty())
	{
		bool bClearAll = false;
		if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
			QStringLiteral("清除选择框？"), QMessageBox::Ok, QMessageBox::Cancel))
		{
			bClearAll = true;
		}
		m_pVLMaskEditor->setImage(matImage, bClearAll);
		m_pVLMaskEditor->repaintAll();
		m_pVLMaskEditor->setEditorView(true, false);
		m_pVLMaskEditor->setSelectRtRect(true);
		m_pVLMaskEditor->show();

		while (!m_pVLMaskEditor->isHidden())
		{
			QThread::msleep(100);
			QApplication::processEvents();
		}

		if (m_pVLMaskEditor->getSelectNum() > 0)
		{
			m_srhCircleROI = m_pVLMaskEditor->getSelectRect();
		}
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
	}
}

void VisionDetectRunView::onSrhCircleDetect()
{
	if (m_detectEdgeImg.empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先设定检测边界"));
		return;
	}

	//cv::Mat matImage = m_pView->getImage().clone();
	//if (!matImage.empty())
	{
		Vision::PR_INSP_CIRCLE_CMD  stCmd;
		stCmd.matInputImg = m_detectEdgeImg;
		//stCmd.matMask = m_detectEdgeImg;

		if (m_srhCircleROI.width == 0 || m_srhCircleROI.height == 0)
		{
			m_srhCircleROI.x = 0;
			m_srhCircleROI.y = 0;
			m_srhCircleROI.width = m_detectEdgeImg.size().width;
			m_srhCircleROI.height = m_detectEdgeImg.size().height;
		}

		stCmd.rectROI = m_srhCircleROI;


		Vision::PR_INSP_CIRCLE_RPY stRpy;
		Vision::VisionStatus retStatus = Vision::PR_InspCircle(&stCmd, &stRpy);
		if (retStatus == Vision::VisionStatus::OK)
		{
			getVisionUI()->displayImage(stRpy.matResultImg);

			ui.lineEdit_SrhCirclePosX->setText(QString("%1").arg(convertToUm(stRpy.ptCircleCtr.x)));
			ui.lineEdit_SrhCirclePosY->setText(QString("%1").arg(convertToUm(stRpy.ptCircleCtr.y)));
			ui.lineEdit_SrhCircleDiameter->setText(QString("%1").arg(convertToUm(stRpy.fDiameter)));
			ui.lineEdit_SrhCircleRound->setText(QString("%1").arg(stRpy.fRoundness));
		}
		else
		{
			getVisionUI()->addImageText(QString("Error at Template Match, error code = %1").arg((int)retStatus));

			ui.lineEdit_SrhCirclePosX->setText(QString("%1").arg(0.0));
			ui.lineEdit_SrhCirclePosY->setText(QString("%1").arg(0.0));
			ui.lineEdit_SrhCircleDiameter->setText(QString("%1").arg(0.0));
			ui.lineEdit_SrhCircleRound->setText(QString("%1").arg(0.0));
		}
	}
	//else
	//{
	//	QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
	//}
}

void VisionDetectRunView::onDetectGrayScaleOpen()
{
	QString path = QApplication::applicationDirPath();
	path += "/capture/";

	QString picFilter = "Image(*.tif *.tiff *.gif *.bmp *.jpg *.jpeg *.jp2 *.png *.pcx *.pgm *.ppm *.pbm *.xwd *.ima)";
	QStringList files = QFileDialog::getOpenFileNames(this, QStringLiteral("打开图片文件夹"), path, picFilter);

	if (files.count())
	{
		m_detectGrayScaleFiles = files;
		ui.lineEdit_DetectGrayScaleFile->setText(QString("%1").arg(files.join(',')));
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
	}
}

void VisionDetectRunView::onSaveDetectGrayScaleParams()
{
	int nDetectGrayScaleRow = ui.lineEdit_DetectGraySacleRow->text().toInt();
	int nDetectGrayScaleCol = ui.lineEdit_DetectGrayScaleCol->text().toInt();

	System->setParam("detect_gray_scale_row", nDetectGrayScaleRow);
	System->setParam("detect_gray_scale_col", nDetectGrayScaleCol);
}

void VisionDetectRunView::onDetectGrayScale()
{
	if (m_detectGrayScaleFiles.count() <= 0)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	int nDetectGrayScaleRow = ui.lineEdit_DetectGraySacleRow->text().toInt();
	int nDetectGrayScaleCol = ui.lineEdit_DetectGrayScaleCol->text().toInt();

	Vision::PR_GRID_AVG_GRAY_SCALE_CMD  stCmd;
	stCmd.nGridRow = nDetectGrayScaleRow;
	stCmd.nGridCol = nDetectGrayScaleCol;

	for (int i = 0; i < m_detectGrayScaleFiles.count(); i++)
	{
		cv::Mat imageMat = imread(m_detectGrayScaleFiles[i].toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);
		stCmd.vecInputImgs.push_back(imageMat);
	}


	Vision::PR_GRID_AVG_GRAY_SCALE_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_GridAvgGrayScale(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		getVisionUI()->displayImage(stRpy.matResultImg);
	}
	else
	{
		getVisionUI()->addImageText(QString("Error at Template Match, error code = %1").arg((int)retStatus));
	}
}

void VisionDetectRunView::onDLPIndexChanged(int iState)
{
	int nDLPIndex = ui.comboBox_selectDLP->currentIndex();
	System->setParam("dlp_image_capture_index", nDLPIndex);
}

void VisionDetectRunView::on3DDetectOpen()
{
	QString path = QApplication::applicationDirPath();
	path += "/capture/";

	QString filePath = QFileDialog::getExistingDirectory(this, QStringLiteral("打开图片文件夹"), path);

	if (!filePath.isEmpty())
	{
		cv::Mat matGray;
		if (convertToGrayImage(filePath, matGray))
		{
			getVisionUI()->setImage(matGray);

			ui.lineEdit_3DDetectFile->setText(QString("%1").arg(filePath + "/"));
		}
		else
		{
			ui.lineEdit_3DDetectFile->setText("");
		}
	}
	else
	{
		ui.lineEdit_3DDetectFile->setText("");
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请选择文件夹"));
	}
}


void VisionDetectRunView::on3DDetect()
{
	QString sz3DDetectFile = ui.lineEdit_3DDetectFile->text();
	if (sz3DDetectFile.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	int nDLPIndex = ui.comboBox_selectDLP->currentIndex();
	QString sz3DCaliRstFile = System->getParam(QString("3d_cali_rst_filename_%1").arg(nDLPIndex + 1)).toString();
	QString path = QApplication::applicationDirPath();
	QDir dir(path);
	sz3DCaliRstFile = dir.absolutePath() + sz3DCaliRstFile;

	if (sz3DCaliRstFile.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择校准输出文件"));
		return;
	}

	const int IMAGE_COUNT = 12;
	std::string strFolder = sz3DDetectFile.toStdString();
	Vision::PR_CALC_3D_HEIGHT_CMD stCmd;
	Vision::PR_CALC_3D_HEIGHT_RPY stRpy;
	if (!readImages(sz3DDetectFile, stCmd.vecInputImgs))
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("读取文件错误"));
		return;
	}

	bool b3DDetectCaliUseThinPattern = ui.checkBox_3DCaliUseThinPattern->isChecked();
	bool b3DDetectGaussionFilter = ui.checkBox_3DDetectGaussionFilter->isChecked();
	bool b3DDetectReverseSeq = ui.checkBox_3DDetectReverseSeq->isChecked();
	double d3DDetectMinIntDiff = ui.lineEdit_3DDetectMinIntDiff->text().toDouble();
	double d3DDetectPhaseShift = ui.doubleSpinBox_phaseShift->value();
	stCmd.bEnableGaussianFilter = b3DDetectGaussionFilter;
	stCmd.bReverseSeq = b3DDetectReverseSeq;
	stCmd.fMinAmplitude = d3DDetectMinIntDiff;
	stCmd.bUseThinnestPattern = b3DDetectCaliUseThinPattern;
	stCmd.fPhaseShift = d3DDetectPhaseShift;

	cv::Mat matBaseSurfaceParam;

	std::string strResultMatPath = sz3DCaliRstFile.toStdString();
	cv::FileStorage fs(strResultMatPath, cv::FileStorage::READ);
	cv::FileNode fileNode = fs["K1"];
	cv::read(fileNode, stCmd.matThickToThinK, cv::Mat());
	fileNode = fs["K2"];
	cv::read(fileNode, stCmd.matThickToThinnestK, cv::Mat());
	fileNode = fs["BaseWrappedAlpha"];
	cv::read(fileNode, stCmd.matBaseWrappedAlpha, cv::Mat());
	fileNode = fs["BaseWrappedBeta"];
	cv::read(fileNode, stCmd.matBaseWrappedBeta, cv::Mat());
	fileNode = fs["BaseWrappedGamma"];
	cv::read(fileNode, stCmd.matBaseWrappedGamma, cv::Mat());
	fs.release();

	path = QApplication::applicationDirPath();
	path += "/3D/config/";

	QString fileName = QString("IntegrateCalibResult") + QString::number(nDLPIndex + 1, 'g', 2) + QString(".yml");

	std::string strIntegratedCalibResultPath = path.toStdString() + fileName.toStdString();
	cv::FileStorage fsIntegrated(strIntegratedCalibResultPath, cv::FileStorage::READ);
	cv::FileNode fileNodeIntegrated = fsIntegrated["IntegratedK"];
	cv::read(fileNodeIntegrated, stCmd.matIntegratedK, cv::Mat());
	fileNodeIntegrated = fsIntegrated["Order3CurveSurface"];
	cv::read(fileNodeIntegrated, stCmd.matOrder3CurveSurface, cv::Mat());
	fsIntegrated.release();

	QDateTime dateTM = QDateTime::currentDateTime();
	Vision::VisionStatus retStatus = PR_Calc3DHeight(&stCmd, &stRpy);
	System->setTrackInfo(QString("PR_Calc3DHeight time %1 ms").arg(dateTM.msecsTo(QDateTime::currentDateTime())));

	path = QApplication::applicationDirPath();
	path += "/Vision/";
	Vision::PR_DumpTimeLog(path.toStdString() + "timelog.log");

	if (retStatus == Vision::VisionStatus::OK)
	{
		int nDLPIndex = ui.comboBox_selectDLP->currentIndex();

		m_3DMatHeights[nDLPIndex] = stRpy.matHeight;
	
		cv::Mat matHeightResultImg = drawHeightGray(stRpy.matHeight);
		getVisionUI()->displayImage(matHeightResultImg);

		cv::Mat matGray;
		const int IMAGE_COUNT = 4;
		std::string strFolder = sz3DDetectFile.toStdString();
		for (int i = 1; i <= IMAGE_COUNT; ++i)
		{
			char chArrFileName[100];
			_snprintf(chArrFileName, sizeof(chArrFileName), "%02d.bmp", i);
			std::string strImageFile = strFolder + chArrFileName;
			cv::Mat mat = cv::imread(strImageFile, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);

			if (matGray.empty())
			{
				matGray = mat.clone();
				matGray.setTo(0);
			}
			matGray += mat / IMAGE_COUNT;
		}

		IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
		if (pUI)
		{
			pUI->setHeightData(m_3DMatHeights[nDLPIndex]);
			pUI->setImage(matGray, false);
		}	
	}
	else
	{
		getVisionUI()->addImageText(QString("Error at 3D Height Calculation, error code = %1").arg((int)retStatus));
	}
}

void VisionDetectRunView::onPhaseShiftValueChanged(double dValue)
{
	updatePhaseShift(dValue);
}

void VisionDetectRunView::on3DDetectMerge()
{
	int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
	for (int i = 0; i < nStationNum; i++)
	{
		if (m_3DMatHeights[i].empty())
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先分步计算3D高度！"));
			return;
		}
	}

	if (2 == nStationNum) // two DLPs
	{
		Vision::PR_MERGE_3D_HEIGHT_CMD stCmd;
		Vision::PR_MERGE_3D_HEIGHT_RPY stRpy;

		for (int i = 0; i < nStationNum; i++)
		{
			stCmd.vecMatHeight.push_back(m_3DMatHeights[i]);
		}
		double d3DDetectHeightDiffThd = ui.lineEdit_3DDetectHeightDiffThreshold->text().toDouble();
		stCmd.fHeightDiffThreshold = d3DDetectHeightDiffThd;
		double d3DDetectHeightNoiseThd = ui.lineEdit_3DDetectHeightNoiseThreshold->text().toDouble();
		stCmd.fRemoveLowerNoiseRatio = d3DDetectHeightNoiseThd;

		QDateTime dateTM = QDateTime::currentDateTime();
		Vision::VisionStatus retStatus = PR_Merge3DHeight(&stCmd, &stRpy);
		System->setTrackInfo(QString("PR_Merge3DHeight time %1 ms").arg(dateTM.msecsTo(QDateTime::currentDateTime())));

		QString path = QApplication::applicationDirPath();
		path += "/Vision/";
		Vision::PR_DumpTimeLog(path.toStdString() + "timelog.log");

		if (retStatus == Vision::VisionStatus::OK)
		{
			m_3DMatHeightMerge = stRpy.matHeight;

			//cv::patchNaNs(m_3DMatHeightMerge, 0);

			cv::Mat matHeightResultImg = drawHeightGray(stRpy.matHeight);
			getVisionUI()->displayImage(matHeightResultImg);


			IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
			if (pUI)
			{
				pUI->setHeightData(m_3DMatHeightMerge);
			}
		}
		else
		{
			getVisionUI()->addImageText(QString("Error at 3D Height Calculation, error code = %1").arg((int)retStatus));
		}
	}
	else // 4 DLPs
	{
		cv::Mat matHeightMerges[2];
		for (int j = 0; j < 2; j++)
		{
			Vision::PR_MERGE_3D_HEIGHT_CMD stCmd;
			Vision::PR_MERGE_3D_HEIGHT_RPY stRpy;

			stCmd.vecMatHeight.push_back(m_3DMatHeights[j + 0]);
			stCmd.vecMatHeight.push_back(m_3DMatHeights[j + 2]);

			double d3DDetectHeightDiffThd = ui.lineEdit_3DDetectHeightDiffThreshold->text().toDouble();
			stCmd.fHeightDiffThreshold = d3DDetectHeightDiffThd;
			double d3DDetectHeightNoiseThd = ui.lineEdit_3DDetectHeightNoiseThreshold->text().toDouble();
			stCmd.fRemoveLowerNoiseRatio = d3DDetectHeightNoiseThd;

			QDateTime dateTM = QDateTime::currentDateTime();
			Vision::VisionStatus retStatus = PR_Merge3DHeight(&stCmd, &stRpy);
			System->setTrackInfo(QString("PR_Merge3DHeight time %1 ms").arg(dateTM.msecsTo(QDateTime::currentDateTime())));

			QString path = QApplication::applicationDirPath();
			path += "/Vision/";
			Vision::PR_DumpTimeLog(path.toStdString() + "timelog.log");

			if (retStatus == Vision::VisionStatus::OK)
			{
				matHeightMerges[j] = stRpy.matHeight;

				cv::Mat matHeightResultImg = drawHeightGray(stRpy.matHeight);
				getVisionUI()->displayImage(matHeightResultImg);
			}
			else
			{
				getVisionUI()->addImageText(QString("Error at 3D Height Calculation, error code = %1").arg((int)retStatus));
			}
		}

		m_3DMatHeightMerge = (matHeightMerges[0] + matHeightMerges[1])/2;
		//cv::patchNaNs(m_3DMatHeightMerge, 0);

		IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
		if (pUI)
		{
			pUI->setHeightData(m_3DMatHeightMerge);
		}
	}
		
}

//void VisionDetectRunView::on3DDetectShow()
//{
//	if (m_3DMatHeightMerge.empty())
//	{
//		int nDLPIndex = ui.comboBox_selectDLP->currentIndex();
//
//		if (!m_3DMatHeights[nDLPIndex].empty())
//		{
//			//auto vecMatNewPhase = matToVector<float>(m_3DMatHeight);
//			int nSizeY = m_3DMatHeights[nDLPIndex].rows;
//			int nSizeX = m_3DMatHeights[nDLPIndex].cols;
//
//			int nDataNum = nSizeX * nSizeY;
//
//			double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
//
//			cv::Mat mat3DHeight = m_3DMatHeights[nDLPIndex].clone();
//			cv::patchNaNs(mat3DHeight, 0);
//
//			QVector<double> xValues, yValues, zValues;
//			for (int i = 0; i < nDataNum; i++)
//			{
//				int col = i%nSizeY + 1;
//				int row = i / nSizeY + 1;
//
//				xValues.push_back(col - nSizeY / 2);
//				yValues.push_back(row - nSizeX / 2);
//
//				zValues.push_back(mat3DHeight.at<float>(col - 1, row - 1) * 1000 / dResolutionX);
//			}
//
//			getVisionUI()->show3DView();
//			getVisionUI()->load3DViewData(nSizeY, nSizeX, xValues, yValues, zValues);
//			getVisionUI()->show3DView();
//		}
//	}
//	else
//	{
//		int nSizeY = m_3DMatHeightMerge.rows;
//		int nSizeX = m_3DMatHeightMerge.cols;
//
//		int nDataNum = nSizeX * nSizeY;
//
//		double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
//
//		QVector<double> xValues, yValues, zValues;
//		for (int i = 0; i < nDataNum; i++)
//		{
//			int col = i%nSizeY + 1;
//			int row = i / nSizeY + 1;
//
//			xValues.push_back(col - nSizeY / 2);
//			yValues.push_back(row - nSizeX / 2);
//
//			zValues.push_back(m_3DMatHeightMerge.at<float>(col - 1, row - 1) * 1000 / dResolutionX);
//		}
//
//		getVisionUI()->show3DView();
//		getVisionUI()->load3DViewData(nSizeY, nSizeX, xValues, yValues, zValues);
//		getVisionUI()->show3DView();
//	}
//}

void VisionDetectRunView::onSave3DDetectParams()
{
	bool b3DDetectGaussionFilter = ui.checkBox_3DDetectGaussionFilter->isChecked();
	bool b3DDetectReverseSeq = ui.checkBox_3DDetectReverseSeq->isChecked();
	double d3DDetectMinIntDiff = ui.lineEdit_3DDetectMinIntDiff->text().toDouble();
	double d3DDetectHeightDiffThd = ui.lineEdit_3DDetectHeightDiffThreshold->text().toDouble();
	double d3DDetectHeightNoiseThd = ui.lineEdit_3DDetectHeightNoiseThreshold->text().toDouble();
	double d3DDetectPhaseShift = ui.doubleSpinBox_phaseShift->value();

	System->setParam("3d_detect_gaussion_filter", b3DDetectGaussionFilter);
	System->setParam("3d_detect_reverse_seq", b3DDetectReverseSeq);
	System->setParam("3d_detect_min_intensity_diff", d3DDetectMinIntDiff);
	System->setParam("3d_detect_height_diff_threshold", d3DDetectHeightDiffThd);
	System->setParam("3d_detect_height_noise_threshold", d3DDetectHeightNoiseThd);
	System->setParam("3d_detect_phase_shift", d3DDetectPhaseShift);
}

void VisionDetectRunView::on3DHeightDetect()
{
}

void VisionDetectRunView::onSaveDataParams()
{
	int nDataBaseMin = ui.lineEdit_DataBaseMin->text().toInt();
	int nDataBaseMax = ui.lineEdit_DataBaseMax->text().toInt();
	int nDataDetectMin = ui.lineEdit_DataDetectMin->text().toInt();
	int nDataDetectMax = ui.lineEdit_DataDetectMax->text().toInt();

	System->setParam("3d_detect_data_base_min", nDataBaseMin);
	System->setParam("3d_detect_data_base_max", nDataBaseMax);
	System->setParam("3d_detect_data_detect_min", nDataDetectMin);
	System->setParam("3d_detect_data_detect_max", nDataDetectMax);
}

void VisionDetectRunView::on3DHeightCellObjEdit()
{
	QEos::Notify(EVENT_UI_STATE, 0, RUN_UI_STATE_TOOLS);
}

void VisionDetectRunView::on3DProfileEdit()
{
	m_pVLProflieEditor->show();
}

void VisionDetectRunView::onEditMTFLoc1()
{
	if (getVisionUI()->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	m_pVLMaskEditor->setImage(getVisionUI()->getImage(), true);
	if (m_MTFLocROI1.width > 0 || m_MTFLocROI1.height > 0)
	{
		cv::RotatedRect rtSelectRect;
		rtSelectRect.size.width = m_MTFLocROI1.width;
		rtSelectRect.size.height = m_MTFLocROI1.height;
		rtSelectRect.angle = 0;
		rtSelectRect.center.x = m_MTFLocROI1.x + m_MTFLocROI1.width / 2;
		rtSelectRect.center.y = m_MTFLocROI1.y + m_MTFLocROI1.height / 2;
		m_pVLMaskEditor->addSelect(rtSelectRect);
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
		m_MTFLocROI1 = m_pVLMaskEditor->getSelect().boundingRect();

		displayMTFRect();
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("元件框必须存在，请重新设置"));
	}
}

void VisionDetectRunView::onEditMTFLocLearn1()
{
	if (getVisionUI()->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (m_MTFLocROI1.width == 0 && m_MTFLocROI1.height == 0)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建定位框"));
		return;
	}

	bool bAutoClearRecord = System->getParam("vision_record_auto_clear").toBool();
	if (bAutoClearRecord)
	{
		if (m_nMTFLocRecord1 > 0)
		{
			Vision::VisionStatus retStatus = Vision::PR_FreeRecord(m_nMTFLocRecord1);
			if (retStatus == Vision::VisionStatus::OK)
			{
				System->setTrackInfo(QString("Free previous record %1").arg(m_nMTFLocRecord1));
			}
		}
	}

	Vision::PR_LRN_TEMPLATE_CMD  stCmd;
	stCmd.matInputImg = getVisionUI()->getImage();
	stCmd.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
	stCmd.rectROI = m_MTFLocROI1;

	Vision::PR_LRN_TEMPLATE_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_LrnTmpl(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		m_nMTFLocRecord1 = stRpy.nRecordId;

		QString path = QApplication::applicationDirPath();
		path += "/capture/";
		cv::imwrite(path.toStdString() + "MTFAlignment1.bmp", stCmd.matInputImg(stCmd.rectROI));
	}
	else
	{
		getVisionUI()->addImageText(QString("Error at Learn Template Match, error code = %1").arg((int)retStatus));
	}

}

void VisionDetectRunView::onSearchMTFLoc1()
{
	//cv::RotatedRect rtRect = m_MTFLocROI1;
	//rtRect.size.width *= 2;
	//rtRect.size.height *= 2;
	cv::Rect rtRect;
	rtRect.x = 0;
	rtRect.y = 0;
	rtRect.width = getVisionUI()->getImage().cols;
	rtRect.height = getVisionUI()->getImage().rows;

	Vision::PR_MATCH_TEMPLATE_CMD  stCmdSrh;
	stCmdSrh.matInputImg = getVisionUI()->getImage();
	stCmdSrh.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
	stCmdSrh.nRecordId = m_nMTFLocRecord1;
	stCmdSrh.rectSrchWindow = rtRect/*rtRect.boundingRect()*/;

	//int nTmpMatchObjMotion = System->getParam("srh_tmp_obj_motion").toInt();
	//stCmdSrh.enMotion = (Vision::PR_OBJECT_MOTION)nTmpMatchObjMotion;

	stCmdSrh.enMotion = Vision::PR_OBJECT_MOTION::EUCLIDEAN;
	stCmdSrh.bSubPixelRefine = true;

	Vision::PR_MATCH_TEMPLATE_RPY stRpySrh;
	Vision::VisionStatus retStatusSrh = Vision::PR_MatchTmpl(&stCmdSrh, &stRpySrh);

	QString path = QApplication::applicationDirPath();
	path += "/Vision/";
	Vision::PR_DumpTimeLog(path.toStdString() + "timelog.log");

	if (retStatusSrh == Vision::VisionStatus::OK)
	{
		System->setTrackInfo(QString("Search obj x = %1, y= %2, angle = %3").arg(stRpySrh.ptObjPos.x).arg(stRpySrh.ptObjPos.y).arg(stRpySrh.fRotation));

		// Rotate Image
		cv::Mat imgSrc = getVisionUI()->getImage();
		cv::Point2f center(imgSrc.cols / 2, imgSrc.rows / 2);

		cv::Mat rot = cv::getRotationMatrix2D(center, stRpySrh.fRotation, 1);
		cv::Rect bbox = cv::RotatedRect(center, imgSrc.size(), stRpySrh.fRotation).boundingRect();

		//cv::Point2f ptDest = rotateByPoint(cv::Point2f(stRpySrh.ptObjPos.x, stRpySrh.ptObjPos.y), center, -stRpySrh.fRotation);
		//m_MTFLocROI1.x = ptDest.x - m_MTFLocROI1.width / 2;
		//m_MTFLocROI1.y = ptDest.y - m_MTFLocROI1.height / 2;

		//rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
		//rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;

		cv::warpAffine(imgSrc, m_MTFRtImage, rot, imgSrc.size());
		getVisionUI()->displayImage(m_MTFRtImage);

		stCmdSrh.matInputImg = m_MTFRtImage.empty() ? getVisionUI()->getImage() : m_MTFRtImage;
		Vision::VisionStatus retStatusSrh = Vision::PR_MatchTmpl(&stCmdSrh, &stRpySrh);
		if (retStatusSrh == Vision::VisionStatus::OK)
		{
			m_MTFLocROI1.x = stRpySrh.ptObjPos.x - m_MTFLocROI1.width / 2;
			m_MTFLocROI1.y = stRpySrh.ptObjPos.y - m_MTFLocROI1.height / 2;
		}
		else
		{
			getVisionUI()->addImageText(QString("Error at Template Match, error code = %1").arg((int)retStatusSrh));
		}
	}
	else
	{
		getVisionUI()->addImageText(QString("Error at Template Match, error code = %1").arg((int)retStatusSrh));
	}
}

void VisionDetectRunView::onEditMTFDetect1()
{
	if (getVisionUI()->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (m_MTFLocROI1.width == 0 && m_MTFLocROI1.height == 0)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建定位框"));
		return;
	}

	cv::Rect selectROI = m_MTFLocROI1;

	m_pVLMaskEditor->setImage(getVisionUI()->getImage()(selectROI), true);
	if (m_MTFDetectROI1.width > 0 || m_MTFDetectROI1.height > 0)
	{
		cv::RotatedRect rtSelectRect;
		rtSelectRect.size.width = m_MTFDetectROI1.width;
		rtSelectRect.size.height = m_MTFDetectROI1.height;
		rtSelectRect.angle = 0;
		rtSelectRect.center.x = m_MTFDetectROI1.x + m_MTFDetectROI1.width / 2;
		rtSelectRect.center.y = m_MTFDetectROI1.y + m_MTFDetectROI1.height / 2;
		m_pVLMaskEditor->addSelect(rtSelectRect);
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
		m_MTFDetectROI1 = m_pVLMaskEditor->getSelect().boundingRect();

		cv::Rect selectROI = m_MTFDetectROI1;

		selectROI.x += m_MTFLocROI1.x;
		selectROI.y += m_MTFLocROI1.y;

		cv::Mat diplayMTF = getVisionUI()->getImage()(selectROI);

		double fScaleW = ui.graphicsView_TmpMatchImg_MTF1->width()*1.0 / diplayMTF.size().width;
		double fScaleH = ui.graphicsView_TmpMatchImg_MTF1->height()*1.0 / diplayMTF.size().height;

		cv::Mat mat;
		cv::resize(diplayMTF, mat, cv::Size(diplayMTF.size().width*fScaleW, diplayMTF.size().height*fScaleH), (0, 0), (0, 0), 3);

		QImage image = QImage((uchar*)mat.data, mat.cols, mat.rows, ToInt(mat.step), QImage::Format_RGB888);
		m_TmpMatchMTFScene1->addPixmap(QPixmap::fromImage(image));


		displayMTFRect();
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("检测框必须存在，请重新设置"));
	}
}

void VisionDetectRunView::onEditMTFLoc2()
{
	if (getVisionUI()->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	cv::Rect selectROI = m_MTFLocROI1;

	m_pVLMaskEditor->setImage(getVisionUI()->getImage()(selectROI), true);
	if (m_MTFLocROI2.width > 0 || m_MTFLocROI2.height > 0)
	{
		cv::RotatedRect rtSelectRect;
		rtSelectRect.size.width = m_MTFLocROI2.width;
		rtSelectRect.size.height = m_MTFLocROI2.height;
		rtSelectRect.angle = 0;
		rtSelectRect.center.x = m_MTFLocROI2.x + m_MTFLocROI2.width / 2;
		rtSelectRect.center.y = m_MTFLocROI2.y + m_MTFLocROI2.height / 2;
		m_pVLMaskEditor->addSelect(rtSelectRect);
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
		m_MTFLocROI2 = m_pVLMaskEditor->getSelect().boundingRect();

		displayMTFRect();
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("元件框必须存在，请重新设置"));
	}
}

void VisionDetectRunView::onEditMTFLocLearn2()
{
	if (getVisionUI()->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (m_MTFLocROI2.width == 0 && m_MTFLocROI2.height == 0)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建定位框"));
		return;
	}

	bool bAutoClearRecord = System->getParam("vision_record_auto_clear").toBool();
	if (bAutoClearRecord)
	{
		if (m_nMTFLocRecord2 > 0)
		{
			Vision::VisionStatus retStatus = Vision::PR_FreeRecord(m_nMTFLocRecord2);
			if (retStatus == Vision::VisionStatus::OK)
			{
				System->setTrackInfo(QString("Free previous record %1").arg(m_nMTFLocRecord2));
			}
		}
	}

	Vision::PR_LRN_TEMPLATE_CMD  stCmd;
	stCmd.matInputImg = getVisionUI()->getImage();
	stCmd.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
	stCmd.rectROI = m_MTFLocROI2;
	stCmd.rectROI.x += m_MTFLocROI1.x;
	stCmd.rectROI.y += m_MTFLocROI1.y;


	Vision::PR_LRN_TEMPLATE_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_LrnTmpl(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		m_nMTFLocRecord2 = stRpy.nRecordId;

		QString path = QApplication::applicationDirPath();
		path += "/capture/";
		cv::imwrite(path.toStdString() + "MTFAlignment2.bmp", stCmd.matInputImg(stCmd.rectROI));
	}
	else
	{
		getVisionUI()->addImageText(QString("Error at Learn Template Match, error code = %1").arg((int)retStatus));
	}
}

void VisionDetectRunView::onSearchMTFLoc2()
{
	cv::Rect rtRect = m_MTFLocROI2;
	rtRect.x += m_MTFLocROI1.x;
	rtRect.y += m_MTFLocROI1.y;
	rtRect.width *= 1.2;
	rtRect.height *= 1.2;

	Vision::PR_MATCH_TEMPLATE_CMD  stCmdSrh;
	stCmdSrh.matInputImg = m_MTFRtImage.empty() ? getVisionUI()->getImage() : m_MTFRtImage;
	stCmdSrh.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
	stCmdSrh.nRecordId = m_nMTFLocRecord2;
	stCmdSrh.rectSrchWindow = rtRect;

	int nTmpMatchObjMotion = System->getParam("srh_tmp_obj_motion").toInt();
	stCmdSrh.enMotion = (Vision::PR_OBJECT_MOTION)nTmpMatchObjMotion;

	Vision::PR_MATCH_TEMPLATE_RPY stRpySrh;
	Vision::VisionStatus retStatusSrh = Vision::PR_MatchTmpl(&stCmdSrh, &stRpySrh);

	QString path = QApplication::applicationDirPath();
	path += "/Vision/";
	Vision::PR_DumpTimeLog(path.toStdString() + "timelog.log");

	if (retStatusSrh == Vision::VisionStatus::OK)
	{
		System->setTrackInfo(QString("%1 - %2").arg(stRpySrh.ptObjPos.x).arg(stRpySrh.ptObjPos.y));

		m_MTFLocROI2.x = stRpySrh.ptObjPos.x - m_MTFLocROI2.width / 2;
		m_MTFLocROI2.y = stRpySrh.ptObjPos.y - m_MTFLocROI2.height / 2;

		m_MTFLocROI2.x -= m_MTFLocROI1.x;
		m_MTFLocROI2.y -= m_MTFLocROI1.y;
	}
	else
	{
		getVisionUI()->addImageText(QString("Error at Template Match, error code = %1").arg((int)retStatusSrh));
	}
}

void VisionDetectRunView::onEditMTFDetect2()
{
	if (getVisionUI()->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (m_MTFLocROI2.width == 0 && m_MTFLocROI2.height == 0)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建定位框"));
		return;
	}

	cv::Rect selectROI = m_MTFLocROI2;
	selectROI.x += m_MTFLocROI1.x;
	selectROI.y += m_MTFLocROI1.y;

	m_pVLMaskEditor->setImage(getVisionUI()->getImage()(selectROI), true);
	if (m_MTFDetectROI2.width > 0 || m_MTFDetectROI2.height > 0)
	{
		cv::RotatedRect rtSelectRect;
		rtSelectRect.size.width = m_MTFDetectROI2.width;
		rtSelectRect.size.height = m_MTFDetectROI2.height;
		rtSelectRect.angle = 0;
		rtSelectRect.center.x = m_MTFDetectROI2.x + m_MTFDetectROI2.width / 2;
		rtSelectRect.center.y = m_MTFDetectROI2.y + m_MTFDetectROI2.height / 2;
		m_pVLMaskEditor->addSelect(rtSelectRect);
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
		m_MTFDetectROI2 = m_pVLMaskEditor->getSelect().boundingRect();

		cv::Rect selectROI = m_MTFDetectROI2;
		selectROI.x += m_MTFLocROI1.x + m_MTFLocROI2.x;
		selectROI.y += m_MTFLocROI1.y + m_MTFLocROI2.y;

		cv::Mat diplayMTF = getVisionUI()->getImage()(selectROI);

		double fScaleW = ui.graphicsView_TmpMatchImg_MTF2->width()*1.0 / diplayMTF.size().width;
		double fScaleH = ui.graphicsView_TmpMatchImg_MTF2->height()*1.0 / diplayMTF.size().height;

		cv::Mat mat;
		cv::resize(diplayMTF, mat, cv::Size(diplayMTF.size().width*fScaleW, diplayMTF.size().height*fScaleH), (0, 0), (0, 0), 3);

		QImage image = QImage((uchar*)mat.data, mat.cols, mat.rows, ToInt(mat.step), QImage::Format_RGB888);
		m_TmpMatchMTFScene2->addPixmap(QPixmap::fromImage(image));


		displayMTFRect();
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("检测框必须存在，请重新设置"));
	}
}

void VisionDetectRunView::onCalcMTF()
{
	if (getVisionUI()->getImage().empty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	if (m_nMTFLocRecord1 == 0 || m_nMTFLocRecord2 == 0)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先创建定位框,检测框"));
		return;
	}

	loadMTFData();

	cv::Mat mtfImage1;
	onSearchMTFLoc1();
	// Show MTF detect 1 image
	{
		cv::Rect selectROI = m_MTFDetectROI1;
		selectROI.x += m_MTFLocROI1.x;
		selectROI.y += m_MTFLocROI1.y;
		mtfImage1 = m_MTFRtImage.empty() ? getVisionUI()->getImage()(selectROI) : m_MTFRtImage(selectROI);


		double fScaleW = ui.graphicsView_TmpMatchImg_MTF1->width()*1.0 / mtfImage1.size().width;
		double fScaleH = ui.graphicsView_TmpMatchImg_MTF1->height()*1.0 / mtfImage1.size().height;
		cv::Mat mat;
		cv::resize(mtfImage1, mat, cv::Size(mtfImage1.size().width*fScaleW, mtfImage1.size().height*fScaleH), (0, 0), (0, 0), 3);
		QImage image = QImage((uchar*)mat.data, mat.cols, mat.rows, ToInt(mat.step), QImage::Format_RGB888);
		m_TmpMatchMTFScene1->addPixmap(QPixmap::fromImage(image));
	}

	cv::Mat mtfImage2;
	onSearchMTFLoc2();
	// Show MTF detect 2 image
	{
		cv::Rect selectROI = m_MTFDetectROI2;
		selectROI.x += m_MTFLocROI1.x + m_MTFLocROI2.x;
		selectROI.y += m_MTFLocROI1.y + m_MTFLocROI2.y;

		mtfImage2 = m_MTFRtImage.empty() ? getVisionUI()->getImage()(selectROI) : m_MTFRtImage(selectROI);

		double fScaleW = ui.graphicsView_TmpMatchImg_MTF2->width()*1.0 / mtfImage2.size().width;
		double fScaleH = ui.graphicsView_TmpMatchImg_MTF2->height()*1.0 / mtfImage2.size().height;

		cv::Mat mat;
		cv::resize(mtfImage2, mat, cv::Size(mtfImage2.size().width*fScaleW, mtfImage2.size().height*fScaleH), (0, 0), (0, 0), 3);

		QImage image = QImage((uchar*)mat.data, mat.cols, mat.rows, ToInt(mat.step), QImage::Format_RGB888);
		m_TmpMatchMTFScene2->addPixmap(QPixmap::fromImage(image));
	}

	displayMTFRect();

	// Calculate MTF value	
	Vision::PR_CALC_CAMERA_MTF_CMD stCmd;
	Vision::PR_CALC_CAMERA_MTF_RPY stRpy;

	cv::Rect displayRect = m_MTFDetectROI1;
	displayRect.x += m_MTFLocROI1.x;
	displayRect.y += m_MTFLocROI1.y;

	stCmd.matInputImg = m_MTFRtImage.empty() ? getVisionUI()->getImage() : m_MTFRtImage;

	stCmd.rectVBigPatternROI = displayRect;
	stCmd.rectVBigPatternROI.width /= 2;
	stCmd.rectHBigPatternROI = displayRect;
	stCmd.rectHBigPatternROI.width /= 2;
	stCmd.rectHBigPatternROI.x += stCmd.rectHBigPatternROI.width;

	//stCmd.rectVBigPatternROI = displayRect;
	//stCmd.rectVBigPatternROI.height /= 2;
	//stCmd.rectHBigPatternROI = displayRect;
	//stCmd.rectHBigPatternROI.height /= 2;
	//stCmd.rectHBigPatternROI.y += stCmd.rectHBigPatternROI.height;

	displayRect = m_MTFDetectROI2;
	displayRect.x += m_MTFLocROI1.x + m_MTFLocROI2.x;
	displayRect.y += m_MTFLocROI1.y + m_MTFLocROI2.y;

	stCmd.rectVSmallPatternROI = displayRect;
	stCmd.rectVSmallPatternROI.width /= 2;
	stCmd.rectHSmallPatternROI = displayRect;
	stCmd.rectHSmallPatternROI.width /= 2;
	stCmd.rectHSmallPatternROI.x += stCmd.rectHSmallPatternROI.width;

	//stCmd.rectVSmallPatternROI = displayRect;
	//stCmd.rectVSmallPatternROI.height /= 2;
	//stCmd.rectHSmallPatternROI = displayRect;
	//stCmd.rectHSmallPatternROI.height /= 2;
	//stCmd.rectHSmallPatternROI.y += stCmd.rectHSmallPatternROI.height;

	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("计算Camera MTF曲线...请耐心等待..."), 0);
	QApplication::processEvents();

	Vision::VisionStatus retStatus = PR_CalcCameraMTF(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		ui.lineEdit_BigPatAbsMtfV->setText(QString::number(stRpy.fBigPatternAbsMtfV, 'f', 5));
		ui.lineEdit_BigPatAbsMtfH->setText(QString::number(stRpy.fBigPatternAbsMtfH, 'f', 5));
		ui.lineEdit_SmallPatAbsMtfV->setText(QString::number(stRpy.fSmallPatternAbsMtfV, 'f', 5));
		ui.lineEdit_SmallPatAbsMtfH->setText(QString::number(stRpy.fSmallPatternAbsMtfH, 'f', 5));
		ui.lineEdit_SmallPatRelMtfV->setText(QString::number(stRpy.fSmallPatternRelMtfV, 'f', 5));
		ui.lineEdit_SmallPatRelMtfH->setText(QString::number(stRpy.fSmallPatternRelMtfH, 'f', 5));
	}
	else
	{
		ui.lineEdit_BigPatAbsMtfV->setText(QString::number(0.0, 'f', 5));
		ui.lineEdit_BigPatAbsMtfH->setText(QString::number(0.0, 'f', 5));
		ui.lineEdit_SmallPatAbsMtfV->setText(QString::number(0.0, 'f', 5));
		ui.lineEdit_SmallPatAbsMtfH->setText(QString::number(0.0, 'f', 5));
		ui.lineEdit_SmallPatRelMtfV->setText(QString::number(0.0, 'f', 5));
		ui.lineEdit_SmallPatRelMtfH->setText(QString::number(0.0, 'f', 5));

		getVisionUI()->addImageText(QString("Error at PR_CalcCameraMTF, error code = %1").arg((int)retStatus));
		System->setTrackInfo(QString("Error at PR_CalcCameraMTF, error code = %1").arg((int)retStatus));
	}

	QSystem::closeMessage();
}

void VisionDetectRunView::onLoadMTFFile()
{
	loadMTFData();
}

void VisionDetectRunView::onSaveMTFFile()
{
	saveMTFData();
}

void VisionDetectRunView::saveMTFData()
{
	QString path = QApplication::applicationDirPath();
	path += "/data/";

	std::string strResultMatPath = QString(path + "mtfData.yml").toStdString();
	cv::FileStorage fs(strResultMatPath, cv::FileStorage::WRITE);
	if (!fs.isOpened())
		return;

	write(fs, QString("mtf_loc_rd_1").toStdString(), m_nMTFLocRecord1);
	write(fs, QString("mtf_loc_1").toStdString(), m_MTFLocROI1);
	write(fs, QString("mtf_detect_1").toStdString(), m_MTFDetectROI1);

	write(fs, QString("mtf_loc_rd_2").toStdString(), m_nMTFLocRecord2);
	write(fs, QString("mtf_loc_2").toStdString(), m_MTFLocROI2);
	write(fs, QString("mtf_detect_2").toStdString(), m_MTFDetectROI2);


	fs.release();
}

void VisionDetectRunView::loadMTFData()
{

	QString path = QApplication::applicationDirPath();
	path += "/data/";

	std::string strResultMatPath = QString(path + "mtfData.yml").toStdString();
	cv::FileStorage fs(strResultMatPath, cv::FileStorage::READ);
	if (!fs.isOpened())
		return;

	cv::FileNode fileNode = fs["mtf_loc_rd_1"];
	cv::read(fileNode, m_nMTFLocRecord1, 0);

	fileNode = fs["mtf_loc_1"];
	cv::read(fileNode, m_MTFLocROI1, cv::Rect());

	fileNode = fs["mtf_detect_1"];
	cv::read(fileNode, m_MTFDetectROI1, cv::Rect());

	fileNode = fs["mtf_loc_rd_2"];
	cv::read(fileNode, m_nMTFLocRecord2, 0);

	fileNode = fs["mtf_loc_2"];
	cv::read(fileNode, m_MTFLocROI2, cv::Rect());

	fileNode = fs["mtf_detect_2"];
	cv::read(fileNode, m_MTFDetectROI2, cv::Rect());

	fs.release();

	m_MTFRtImage.release();
}

cv::Point2f VisionDetectRunView::rotateByPoint(cv::Point2f ptSrc, cv::Point2f ptCenter, double angle)
{
	cv::Point2f ptDest;
	double x0 = (ptSrc.x - ptCenter.x)*qCos(angle*M_PI / 180) - (ptSrc.y - ptCenter.y)*qSin(angle*M_PI / 180) + ptCenter.x;
	double y0 = (ptSrc.x - ptCenter.x)*qSin(angle*M_PI / 180) + (ptSrc.y - ptCenter.y)*qCos(angle*M_PI / 180) + ptCenter.y;
	ptDest.x = x0;
	ptDest.y = y0;
	return ptDest;
}

bool VisionDetectRunView::convertToGrayImage(QString& szFilePath, cv::Mat &matGray)
{
	//判断路径是否存在
	QDir dir(szFilePath);
	if (!dir.exists())
	{
		return false;
	}
	dir.setFilter(QDir::Files | QDir::NoSymLinks);
	QFileInfoList list = dir.entryInfoList();

	int file_count = list.count();
	if (file_count <= 0)
	{
		return false;
	}

	QVector<cv::Mat> imageMats;
	for (int i = 0; i < file_count; i++)
	{
		QFileInfo file_info = list.at(i);
		QString suffix = file_info.suffix();
		if (QString::compare(suffix, QString("ent"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			QString nameDecrypt = file_info.absolutePath() + "/" + file_info.baseName() + ".bmp";
			AOI::Crypt::DecryptFileNfg(absolute_file_path.toStdString(), nameDecrypt.toStdString());

			cv::Mat mat = cv::imread(nameDecrypt.toStdString(), cv::IMREAD_GRAYSCALE);
			imageMats.push_back(mat);

			QFile::remove(nameDecrypt);
		}
		else if (QString::compare(suffix, QString("bmp"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			cv::Mat mat = cv::imread(absolute_file_path.toStdString(), cv::IMREAD_GRAYSCALE);
			imageMats.push_back(mat);
		}
		else
		{
			return false;
		}
	}

	m_pCtrl->generateAverageImage(imageMats, matGray);

	return true;
}

bool VisionDetectRunView::readImages(QString& szFilePath, AOI::Vision::VectorOfMat& matImgs)
{
	const int IMAGE_COUNT = 12;

	//判断路径是否存在
	QDir dir(szFilePath);
	if (!dir.exists())
	{
		return false;
	}
	dir.setFilter(QDir::Files | QDir::NoSymLinks);
	QFileInfoList list = dir.entryInfoList();	

	int file_count = list.count();
	if (file_count < IMAGE_COUNT)
	{
		return false;
	}

	for (int i = 0; i < file_count; i++)
	{
		QFileInfo file_info = list.at(i);
		QString suffix = file_info.suffix();
		if (QString::compare(suffix, QString("ent"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			QString nameDecrypt = file_info.absolutePath() + "/" + file_info.baseName() + ".bmp";
			AOI::Crypt::DecryptFileNfg(absolute_file_path.toStdString(), nameDecrypt.toStdString());

			cv::Mat mat = cv::imread(nameDecrypt.toStdString(), cv::IMREAD_GRAYSCALE);
			matImgs.push_back(mat);

			QFile::remove(nameDecrypt);
		}
		else if (QString::compare(suffix, QString("bmp"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			cv::Mat mat = cv::imread(absolute_file_path.toStdString(), cv::IMREAD_GRAYSCALE);
			matImgs.push_back(mat);
		}
		else
		{
			return false;
		}
	}
	return true;
}

void VisionDetectRunView::updatePhaseShift(double dValue)
{
	double A = 10;
	double B1 = -5;
	double B2 = 5;

	double dShiftMin = A * dValue + B1;
	double dShiftMax = A * dValue + B2;

	ui.label_phaseShiftMin->setText(QString("%1mm").arg(dShiftMin));
	ui.label_phaseShiftMax->setText(QString("%1mm").arg(dShiftMax));
}

void VisionDetectRunView::onDlpMTFOpen()
{
	QString path = QApplication::applicationDirPath();
	path += "/capture/";

	QString filePath = QFileDialog::getExistingDirectory(this, QStringLiteral("打开图片文件夹"), path);

	if (!filePath.isEmpty())
	{
		ui.lineEdit_dlpMTFFile->setText(QString("%1").arg(filePath + "/"));

		std::string strImageFile = QString("%1").arg(filePath + "/" + "01.bmp").toStdString();
		cv::Mat mat = cv::imread(strImageFile, cv::IMREAD_GRAYSCALE);
		getVisionUI()->setImage(mat);
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请选择文件夹"));
	}
}

void VisionDetectRunView::onSaveDlpMTFParams()
{
	double dMagnitudeOfDLP = ui.lineEdit_dlpMTFMagnitudeOfDLP->text().toDouble();
	System->setParam("dlp_mtf_magnitude", dMagnitudeOfDLP);

	bool bDlpPDReverseSeq = ui.checkBox_dlpPDReverseSeq->isChecked();
	int nDlpPDSizeX = ui.lineEdit_dlpPDPatternSizeX->text().toInt();
	int nDlpPDSizeY = ui.lineEdit_dlpPDPatternSizeY->text().toInt();
	int nDlpPDGaussianSize = ui.lineEdit_dlpPDGaussianSize->text().toInt();
	double dMagnitudeOfDlpPD = ui.lineEdit_dlpPDMagnitudeOfDLP->text().toDouble();
	double dDlpPDPixelCycle = ui.lineEdit_dlpPDPixelCycle->text().toDouble();
	double dDlpPDGaussianSigma = ui.lineEdit_dlpPDGaussianSigma->text().toDouble();

	System->setParam("dlp_pd_reverse_seq", bDlpPDReverseSeq);
	System->setParam("dlp_pd_pattern_size_x", nDlpPDSizeX);
	System->setParam("dlp_pd_pattern_size_y", nDlpPDSizeY);
	System->setParam("dlp_pd_gaussian_size", nDlpPDGaussianSize);
	System->setParam("dlp_pd_magnitude", dMagnitudeOfDlpPD);
	System->setParam("dlp_pd_pixel_cycle", dDlpPDPixelCycle);
	System->setParam("dlp_pd_gaussian_sigma", dDlpPDGaussianSigma);
}

void VisionDetectRunView::onDetectDlpMTF()
{
	QString szDLPMTFFile = ui.lineEdit_dlpMTFFile->text();
	if (szDLPMTFFile.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	const int IMAGE_COUNT = 12;

	std::string strFolder = szDLPMTFFile.toStdString();

	Vision::PR_CALC_MTF_CMD stCmd;
	Vision::PR_CALC_MTF_RPY stRpy;
	for (int i = 1; i <= IMAGE_COUNT; ++i)
	{
		char chArrFileName[100];
		_snprintf(chArrFileName, sizeof(chArrFileName), "%02d.bmp", i);
		std::string strImageFile = strFolder + chArrFileName;
		cv::Mat mat = cv::imread(strImageFile, cv::IMREAD_GRAYSCALE);
		stCmd.vecInputImgs.push_back(mat);
	}

	double dMagnitudeOfDLP = ui.lineEdit_dlpMTFMagnitudeOfDLP->text().toDouble();
	stCmd.fMagnitudeOfDLP = dMagnitudeOfDLP;

	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("计算DLP MTF曲线...请耐心等待..."), 0);
	QApplication::processEvents();

	Vision::VisionStatus retStatus = PR_CalcMTF(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		std::vector<QString> plotDataName;
		plotDataName.push_back("Coarse");
		plotDataName.push_back("Fine");
		plotDataName.push_back("Super Fine");

		m_pDlpMTFDisplay->setupPlot1Data(QString("Absolute Attenuation Coefficient(H)"), stRpy.vecVecAbsMtfH, plotDataName);
		m_pDlpMTFDisplay->setupPlot2Data(QString("Relative Attenuation Coefficient(H)"), stRpy.vecVecRelMtfH, plotDataName);
		m_pDlpMTFDisplay->setupPlot3Data(QString("Absolute Attenuation Coefficient(V)"), stRpy.vecVecAbsMtfV, plotDataName);
		m_pDlpMTFDisplay->setupPlot4Data(QString("Relative Attenuation Coefficient(V)"), stRpy.vecVecRelMtfV, plotDataName);

		m_pDlpMTFDisplay->show();
	}
	else
	{
		getVisionUI()->addImageText(QString("Error at PR_CalcMTF, error code = %1").arg((int)retStatus));
		System->setTrackInfo(QString("Error at PR_CalcMTF, error code = %1").arg((int)retStatus));
	}

	QSystem::closeMessage();
}

void VisionDetectRunView::onDlpPDOpen()
{
	QString path = QApplication::applicationDirPath();
	path += "/capture/";

	QString filePath = QFileDialog::getExistingDirectory(this, QStringLiteral("打开图片文件夹"), path);

	if (!filePath.isEmpty())
	{
		ui.lineEdit_dlpPDFile->setText(QString("%1").arg(filePath + "/"));

		std::string strImageFile = QString("%1").arg(filePath + "/" + "01.bmp").toStdString();
		cv::Mat mat = cv::imread(strImageFile, cv::IMREAD_GRAYSCALE);
		getVisionUI()->setImage(mat);
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请选择文件夹"));
	}
}

void VisionDetectRunView::onDetectDlpPD()
{
	QString szDLPPDFile = ui.lineEdit_dlpPDFile->text();
	if (szDLPPDFile.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	const int IMAGE_COUNT = 12;

	std::string strFolder = szDLPPDFile.toStdString();

	Vision::PR_CALC_PD_CMD stCmd;
	Vision::PR_CALC_PD_RPY stRpy;
	for (int i = 1; i <= IMAGE_COUNT; ++i)
	{
		char chArrFileName[100];
		_snprintf(chArrFileName, sizeof(chArrFileName), "%02d.bmp", i);
		std::string strImageFile = strFolder + chArrFileName;
		cv::Mat mat = cv::imread(strImageFile, cv::IMREAD_GRAYSCALE);
		stCmd.vecInputImgs.push_back(mat);
	}

	bool bDlpPDReverseSeq = ui.checkBox_dlpPDReverseSeq->isChecked();
	int nDlpPDSizeX = ui.lineEdit_dlpPDPatternSizeX->text().toInt();
	int nDlpPDSizeY = ui.lineEdit_dlpPDPatternSizeY->text().toInt();
	int nDlpPDGaussianSize = ui.lineEdit_dlpPDGaussianSize->text().toInt();
	double dMagnitudeOfDlpPD = ui.lineEdit_dlpPDMagnitudeOfDLP->text().toDouble();
	double dDlpPDPixelCycle = ui.lineEdit_dlpPDPixelCycle->text().toDouble();
	double dDlpPDGaussianSigma = ui.lineEdit_dlpPDGaussianSigma->text().toDouble();


	stCmd.bReverseSeq = bDlpPDReverseSeq;
	stCmd.szDlpPatternSize = cv::Size(nDlpPDSizeX, nDlpPDSizeY);
	stCmd.nGaussianFilterSize = nDlpPDGaussianSize;
	stCmd.fMagnitudeOfDLP = dMagnitudeOfDlpPD;
	stCmd.fDlpPixelCycle = dDlpPDPixelCycle;
	stCmd.fGaussianFilterSigma = dDlpPDGaussianSigma;

	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("计算DLP PD曲线...请耐心等待..."), 0);
	QApplication::processEvents();

	Vision::VisionStatus retStatus = PR_CalcPD(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		getVisionUI()->setImage(stRpy.matCaptureRegionImg);

		std::vector<QString> plotDataName;
		plotDataName.push_back("PD");

		VectorOfVectorOfFloat vecLeft;
		vecLeft.push_back(stRpy.vecDistortionLeft);
		VectorOfVectorOfFloat vecRight;
		vecRight.push_back(stRpy.vecDistortionRight);
		VectorOfVectorOfFloat vecTop;
		vecTop.push_back(stRpy.vecDistortionTop);
		VectorOfVectorOfFloat vecBottom;
		vecBottom.push_back(stRpy.vecDistortionBottom);

		m_pDlpMTFDisplay->setupPlot1Data(QString("Left"), vecLeft, plotDataName);
		m_pDlpMTFDisplay->setupPlot2Data(QString("Right"), vecRight, plotDataName);
		m_pDlpMTFDisplay->setupPlot3Data(QString("Top"), vecTop, plotDataName);
		m_pDlpMTFDisplay->setupPlot4Data(QString("Bottom"), vecBottom, plotDataName);

		m_pDlpMTFDisplay->show();
	}
	else
	{
		getVisionUI()->addImageText(QString("Error at PR_CalcPD, error code = %1").arg((int)retStatus));
		System->setTrackInfo(QString("Error at PR_CalcPD, error code = %1").arg((int)retStatus));
	}

	QSystem::closeMessage();
}



void VisionDetectRunView::displayMTFRect()
{
	cv::Mat matImage = m_MTFRtImage.empty() ? getVisionUI()->getImage().clone() : m_MTFRtImage.clone();

	if (m_MTFLocROI1.width > 0 || m_MTFLocROI1.height > 0)
	{
		cv::rectangle(matImage, m_MTFLocROI1, Scalar(255, 0, 0), 2, 8, 0);
	}

	if (m_MTFDetectROI1.width > 0 || m_MTFDetectROI1.height > 0)
	{
		cv::Rect displayRect = m_MTFDetectROI1;
		displayRect.x += m_MTFLocROI1.x;
		displayRect.y += m_MTFLocROI1.y;

		cv::rectangle(matImage, displayRect, Scalar(0, 255, 0), 2, 8, 0);
	}

	if (m_MTFLocROI2.width > 0 || m_MTFLocROI2.height > 0)
	{
		cv::Rect displayRect = m_MTFLocROI2;
		displayRect.x += m_MTFLocROI1.x;
		displayRect.y += m_MTFLocROI1.y;

		cv::rectangle(matImage, displayRect, Scalar(255, 0, 0), 2, 8, 0);
	}
	if (m_MTFDetectROI2.width > 0 || m_MTFDetectROI2.height > 0)
	{
		cv::Rect displayRect = m_MTFDetectROI2;
		displayRect.x += m_MTFLocROI1.x + m_MTFLocROI2.x;
		displayRect.y += m_MTFLocROI1.y + m_MTFLocROI2.y;

		cv::rectangle(matImage, displayRect, Scalar(0, 255, 0), 2, 8, 0);
	}


	getVisionUI()->displayImage(matImage);
}


cv::Mat VisionDetectRunView::drawHeightGray(const cv::Mat &matHeight)
{
	double dMinValue = 0, dMaxValue = 0;
	cv::Mat matMask = matHeight == matHeight;
	cv::minMaxIdx(matHeight, &dMinValue, &dMaxValue, 0, 0, matMask);

	cv::Mat matNewPhase = matHeight - dMinValue;

	float dRatio = 255.f / ToFloat(dMaxValue - dMinValue);
	matNewPhase = matNewPhase * dRatio;

	cv::Mat matResultImg;
	matNewPhase.convertTo(matResultImg, CV_8UC1);
	cv::cvtColor(matResultImg, matResultImg, CV_GRAY2BGR);

	return matResultImg;
}

cv::Mat VisionDetectRunView::drawHeightGrid(const cv::Mat &matHeight, int nGridRow, int nGridCol) {
	double dMinValue = 0, dMaxValue = 0;
	cv::Mat matMask = matHeight == matHeight;
	cv::minMaxIdx(matHeight, &dMinValue, &dMaxValue, 0, 0, matMask);

	cv::Mat matNewPhase = matHeight - dMinValue;

	float dRatio = 255.f / ToFloat(dMaxValue - dMinValue);
	matNewPhase = matNewPhase * dRatio;

	cv::Mat matResultImg;
	matNewPhase.convertTo(matResultImg, CV_8UC1);
	cv::cvtColor(matResultImg, matResultImg, CV_GRAY2BGR);

	int ROWS = matNewPhase.rows;
	int COLS = matNewPhase.cols;
	int nIntervalX = matNewPhase.cols / nGridCol;
	int nIntervalY = matNewPhase.rows / nGridRow;

	int fontFace = cv::FONT_HERSHEY_SIMPLEX;
	double fontScale = 1;
	int thickness = 3;
	for (int j = 0; j < nGridRow; ++j) {
		for (int i = 0; i < nGridCol; ++i) {
			cv::Rect rectROI(i * nIntervalX, j * nIntervalY, nIntervalX, nIntervalY);
			cv::Mat matROI(matHeight, rectROI);
			cv::Mat matMask = matROI == matROI;
			float fAverage = ToFloat(cv::mean(matROI, matMask)[0]);

			char strAverage[100];
			_snprintf(strAverage, sizeof(strAverage), "%.4f", fAverage);
			int baseline = 0;
			cv::Size textSize = cv::getTextSize(strAverage, fontFace, fontScale, thickness, &baseline);
			//The height use '+' because text origin start from left-bottom.
			cv::Point ptTextOrg(rectROI.x + (rectROI.width - textSize.width) / 2, rectROI.y + (rectROI.height + textSize.height) / 2);
			cv::putText(matResultImg, strAverage, ptTextOrg, fontFace, fontScale, cv::Scalar(255, 0, 0), thickness);
		}
	}

	int nGridLineSize = 3;
	cv::Scalar scalarCyan(255, 255, 0);
	for (int i = 1; i < nGridCol; ++i)
		cv::line(matResultImg, cv::Point(i * nIntervalX, 0), cv::Point(i * nIntervalX, ROWS), scalarCyan, nGridLineSize);
	for (int i = 1; i < nGridRow; ++i)
		cv::line(matResultImg, cv::Point(0, i * nIntervalY), cv::Point(COLS, i * nIntervalY), scalarCyan, nGridLineSize);

	return matResultImg;
}

cv::Mat VisionDetectRunView::drawHeightGrid2(const cv::Mat &matHeight, int nGridRow, int nGridCol)
{
	double dMinValue = 0, dMaxValue = 0;
	cv::Mat matMask = matHeight == matHeight;
	cv::minMaxIdx(matHeight, &dMinValue, &dMaxValue, 0, 0, matMask);

	cv::Mat matNewPhase = matHeight - dMinValue;

	float dRatio = 255.f / ToFloat(dMaxValue - dMinValue);
	matNewPhase = matNewPhase * dRatio;

	cv::Mat matResultImg;
	matNewPhase.convertTo(matResultImg, CV_8UC1);
	cv::cvtColor(matResultImg, matResultImg, CV_GRAY2BGR);

	int ROWS = matNewPhase.rows;
	int COLS = matNewPhase.cols;
	int nIntervalX = matNewPhase.cols / nGridCol;
	int nIntervalY = matNewPhase.rows / nGridRow;

	int rectScaleFactor = 4;

	int fontFace = cv::FONT_HERSHEY_SIMPLEX;
	double fontScale = 1;
	int thickness = 3;
	for (int j = 0; j < nGridRow; ++j)
	{
		for (int i = 0; i < nGridCol; ++i)
		{
			//cv::Rect rectROI(i * nIntervalX * 1.5, j * nIntervalY * 1.5, nIntervalX * 0.5, nIntervalY * 0.5);
			cv::Rect rectROI(i * nIntervalX + nIntervalX / 2 - nIntervalX / 2 / rectScaleFactor, j * nIntervalY + nIntervalY / 2 - nIntervalY / 2 / rectScaleFactor, nIntervalX / rectScaleFactor, nIntervalY / rectScaleFactor);
			cv::Mat matROI(matHeight, rectROI);
			cv::Mat matMask = matROI == matROI;
			float fAverage = ToFloat(cv::mean(matROI, matMask)[0]);

			char strAverage[100];
			_snprintf(strAverage, sizeof(strAverage), "%.4f", fAverage);
			int baseline = 0;
			cv::Size textSize = cv::getTextSize(strAverage, fontFace, fontScale, thickness, &baseline);
			//The height use '+' because text origin start from left-bottom.
			cv::Point ptTextOrg(rectROI.x + (rectROI.width - textSize.width) / 2, rectROI.y + (rectROI.height + textSize.height) / 2);
			cv::putText(matResultImg, strAverage, ptTextOrg, fontFace, fontScale, cv::Scalar(255, 0, 0), thickness);
		}
	}

	int nGridLineSize = 3;
	cv::Scalar scalarCyan(255, 255, 0);
	for (int i = 1; i < nGridCol; ++i)
		cv::line(matResultImg, cv::Point(i * nIntervalX, 0), cv::Point(i * nIntervalX, ROWS), scalarCyan, nGridLineSize);
	for (int i = 1; i < nGridRow; ++i)
		cv::line(matResultImg, cv::Point(0, i * nIntervalY), cv::Point(COLS, i * nIntervalY), scalarCyan, nGridLineSize);

	cv::Scalar scalarCyanAverage(0, 255, 0);
	for (int j = 0; j < nGridRow; ++j)
	{
		for (int i = 0; i < nGridCol; ++i)
		{
			cv::Rect rectROI(i * nIntervalX + nIntervalX / 2 - nIntervalX / 2 / rectScaleFactor, j * nIntervalY + nIntervalY / 2 - nIntervalY / 2 / rectScaleFactor, nIntervalX / rectScaleFactor, nIntervalY / rectScaleFactor);
			cv::rectangle(matResultImg, rectROI, scalarCyanAverage, nGridLineSize);
		}
	}

	return matResultImg;
}
