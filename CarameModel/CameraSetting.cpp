#include "CameraSetting.h"
#include <qfiledialog.h>
#include "../include/workflowDefine.h"
#include "../Common/eos.h"
#include "../Common/SystemData.h"
#include "../include/IdDefine.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMath.h>

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/calib3d.hpp"

#undef NO_ERROR
#include "../lib/VisionLibrary/include/VisionAPI.h"

#include "../CurveEditor/IGraphicEditor.h"

#include "MatHeader.h"

using namespace AOI;

//定义系统Vision LOG
#define VISION_LOG_ALL		2
#define VISION_LOG_FAIL		1

CameraSetting::CameraSetting(CameraCtrl* pCameraCtrl, ViewCtrl* mainWidget, QWidget *parent)
	: m_pCameraCtrl(pCameraCtrl), m_mainView(mainWidget), QWidget(parent)
{
	ui.setupUi(this);
	
	QHBoxLayout* pHBoxLayout = new QHBoxLayout();
	pHBoxLayout->addWidget(m_mainView);
	pHBoxLayout->setContentsMargins(0, 0, 0, 0);
	pHBoxLayout->setAlignment(Qt::AlignTop);
	pHBoxLayout->addStretch(0);
	pHBoxLayout->setSpacing(1);
	m_mainView->setFixedSize(750, 750);
	ui.frame->setLayout(pHBoxLayout);

	m_pGraphicsEditor = new IGraphicEditor();
	m_pGraphicsEditor->setViewPos(100, 0);
	m_pGraphicsEditor->setScale(1.0, 100);		

	//相机设置
	QString capturePath = System->getParam("camera_cap_image_path").toString();
	if (capturePath.isEmpty())capturePath = QApplication::applicationDirPath() + "/capture";
	ui.lineEdit->setText(capturePath);

	QString imgDLP1Path = System->getParam("camera_cap_image_dlp1_path").toString();
	if (imgDLP1Path.isEmpty())imgDLP1Path = QApplication::applicationDirPath() + "/capture";
	ui.lineEdit_imgDLP1->setText(imgDLP1Path);

	QString imgDLP2Path = System->getParam("camera_cap_image_dlp2_path").toString();
	if (imgDLP2Path.isEmpty())imgDLP2Path = QApplication::applicationDirPath() + "/capture";
	ui.lineEdit_imgDLP2->setText(imgDLP2Path);

	bool bHardwareTrigger = System->getParam("camera_hw_tri_enable").toBool();
	ui.checkBox->setChecked(bHardwareTrigger);

	bool bCaptureImage = System->getParam("camera_cap_image_enable").toBool();
	ui.checkBox_2->setChecked(bCaptureImage);

	bool bHardwareTriggerContinuous = System->getParam("camera_hw_tri_continuous").toBool();
	ui.checkBox_3->setChecked(bHardwareTriggerContinuous);
	ui.checkBox_3->setEnabled(false);

	bool bCaptureImageAsMatlab = System->getParam("camera_cap_image_matlab").toBool();
	ui.checkBox_4->setChecked(bCaptureImageAsMatlab);	

	ui.lineEdit_crossGapWidth->setText(QString("%1").arg(60));
	
	QStringList ls;
	ls << QStringLiteral("Baslar相机采集") << QStringLiteral("Dalsa采集卡采集") << QStringLiteral("手动导入图片");
	ui.comboBox_captureMode->addItems(ls);
	int nCaptureMode = System->getParam("camera_capture_mode").toInt();
	ui.comboBox_captureMode->setCurrentIndex(nCaptureMode);
	connect(ui.comboBox_captureMode, SIGNAL(currentIndexChanged(int)), SLOT(onCaptureModeIndexChanged(int)));
	ui.groupBox_imgDLP->setEnabled(2 == nCaptureMode);

	connect(ui.checkBox, SIGNAL(stateChanged(int)), SLOT(onStateChangeHWTrig(int)));
	connect(ui.checkBox_2, SIGNAL(stateChanged(int)), SLOT(onStateChangeCapture(int)));
	connect(ui.checkBox_3, SIGNAL(stateChanged(int)), SLOT(onStateChangeHWTrigCon(int)));
	connect(ui.checkBox_4, SIGNAL(stateChanged(int)), SLOT(onStateChangeCaptureAsMatlab(int)));	
	connect(ui.checkBox_crossEnable, SIGNAL(stateChanged(int)), SLOT(onStateChangeCrossEnable(int)));

	connect(ui.pushButton, SIGNAL(clicked()), SLOT(onSelCapturePath()));
	connect(ui.pushButton_2, SIGNAL(clicked()), SLOT(onSaveCapturePath()));
	connect(ui.pushButton_imgDLP1Select, SIGNAL(clicked()), SLOT(onSelImgDLP1Path()));
	connect(ui.pushButton_imgDLP1Save, SIGNAL(clicked()), SLOT(onSaveImgDLP1Path()));
	connect(ui.pushButton_imgDLP2Select, SIGNAL(clicked()), SLOT(onSelImgDLP2Path()));
	connect(ui.pushButton_imgDLP2Save, SIGNAL(clicked()), SLOT(onSaveImgDLP2Path()));

	//Vision System Setting
	bool bLogAllCase = System->getParam("camera_log_allcase_enable").toBool();
	ui.checkBox_LogAllCase->setChecked(bLogAllCase);

	int nLogType = System->getParam("vision_log_type").toInt();
	if (VISION_LOG_ALL == nLogType)
	{
		ui.radioButton_logFailCase->setChecked(false);
		ui.radioButton_logAllCase->setChecked(true);
	}
	else if (VISION_LOG_FAIL == nLogType)
	{
		ui.radioButton_logFailCase->setChecked(true);
		ui.radioButton_logAllCase->setChecked(false);
	}
	ui.groupBox_LogType->setEnabled(bLogAllCase);

	if (bLogAllCase)
	{
		if (VISION_LOG_ALL == nLogType)
		{
			Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_ALL_CASE);
		}
		else if (VISION_LOG_FAIL == nLogType)
		{
			Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_FAIL_CASE);
		}		
	}
	else
	{
		Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::DISABLED);
	}

	bool bAutoClearRecord = System->getParam("vision_record_auto_clear").toBool();
	ui.checkBox_AutoClearRecord->setChecked(bAutoClearRecord);

	connect(ui.checkBox_LogAllCase, SIGNAL(stateChanged(int)), SLOT(onLogAllCase(int)));
	connect(ui.radioButton_logFailCase, SIGNAL(toggled(bool)), SLOT(onClickLogFailCase(bool)));
	connect(ui.radioButton_logAllCase, SIGNAL(toggled(bool)), SLOT(onClickLogAllCase(bool)));
	connect(ui.pushButton_PRInit, SIGNAL(clicked()), SLOT(onInitPRSystem()));
	connect(ui.pushButton_PRRelease, SIGNAL(clicked()), SLOT(onUninitPRSystem()));
	connect(ui.pushButton_PRClearRecord, SIGNAL(clicked()), SLOT(onClearAllRecords()));
	connect(ui.checkBox_AutoClearRecord, SIGNAL(stateChanged(int)), SLOT(onAutoClearRecord(int)));

	//相机标定
	ui.lineEdit_2->setText(QString("%1").arg(15));
	ui.lineEdit_3->setText(QString("%1").arg(15));
	ui.lineEdit_4->setText(QString("%1").arg(2.0));
	ui.lineEdit_7->setText(QString("%1").arg(90));
	ui.lineEdit_8->setText(QString("%1").arg(10));

	connect(ui.pushButton_3, SIGNAL(clicked()), SLOT(onCalibration()));
	connect(ui.pushButton_4, SIGNAL(clicked()), SLOT(onView3D()));

	double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	double dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
	ui.lineEdit_5->setText(QString("%1").arg(dResolutionX));
	ui.lineEdit_6->setText(QString("%1").arg(dResolutionY));	

	//输入限制
	inputIntRangePos = new QIntValidator(0, 1000, this);
	inputIntRangeAll = new QIntValidator(-1000, 1000, this);
	inputDoubleRangePos = new QDoubleValidator(0, 1000, 10, this);
	inputDoubleRangeAll = new QDoubleValidator(-1000, 1000, 10, this);

	ui.lineEdit_2->setValidator(inputIntRangePos);
	ui.lineEdit_3->setValidator(inputIntRangePos);
	ui.lineEdit_4->setValidator(inputIntRangePos);
	ui.lineEdit_7->setValidator(inputDoubleRangePos);
	ui.lineEdit_8->setValidator(inputDoubleRangePos);
	ui.lineEdit_crossGapWidth->setValidator(inputIntRangePos);

	QString user;
	int level = 0;
	System->getUser(user, level);
	if (USER_LEVEL_TECH > level)
	{
		ui.checkBox_4->setEnabled(false);
	}
}

CameraSetting::~CameraSetting()
{
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

	if (m_pGraphicsEditor)
	{
		delete m_pGraphicsEditor;
		m_pGraphicsEditor = NULL;
	}
}


void CameraSetting::onStateChangeHWTrig(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	if (QMessageBox::Ok == QMessageBox::warning(NULL, QStringLiteral("提示"),
		QStringLiteral("设置硬件触发方式？"), QMessageBox::Ok, QMessageBox::Cancel))
	{
		if (m_mainView)
		{
			CameraDevice* pDev = m_mainView->getCurrentDevice();
			if (pDev)
			{
				pDev->setHardwareTrigger((bool)data);
			}
		}

		System->setParam("camera_hw_tri_enable", (bool)data);
	}
}

void CameraSetting::onStateChangeHWTrigCon(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	System->setParam("camera_hw_tri_continuous", (bool)data);
}

void CameraSetting::onStateChangeCapture(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	System->setParam("camera_cap_image_enable", (bool)data);
}

void CameraSetting::onStateChangeCaptureAsMatlab(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	System->setParam("camera_cap_image_matlab", (bool)data);
}

void CameraSetting::onStateChangeCrossEnable(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	if ((bool)data)
	{
		int nCrossGap = ui.lineEdit_crossGapWidth->text().toInt();

		m_pCameraCtrl->setCrossEnable(true);
		m_pCameraCtrl->setCrossGap(nCrossGap);
	}
	else
	{
		m_pCameraCtrl->setCrossEnable(false);
		m_pCameraCtrl->setCrossGap(0);
	}
}

void CameraSetting::onLogAllCase(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	System->setParam("camera_log_allcase_enable", (bool)data);
	ui.groupBox_LogType->setEnabled((bool)data);

	if ((bool)data)
	{
		int nLogType = System->getParam("vision_log_type").toInt();
		if (VISION_LOG_ALL == nLogType)
		{
			Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_ALL_CASE);
		}
		else if (VISION_LOG_FAIL == nLogType)
		{
			Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_FAIL_CASE);
		}
	}
	else
	{
		Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::DISABLED);
	}
}

void CameraSetting::onClickLogFailCase(bool s)
{
	if (s)
	{
		System->setParam("vision_log_type", (int)VISION_LOG_FAIL);
		Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_FAIL_CASE);
	}
}

void CameraSetting::onClickLogAllCase(bool s)
{
	if (s)
	{
		System->setParam("vision_log_type", (int)VISION_LOG_ALL);
		Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_ALL_CASE);
	}
}

void CameraSetting::onInitPRSystem()
{
	Vision::VisionStatus retStatus = Vision::PR_Init();
	if (retStatus == Vision::VisionStatus::OK)
	{
		QMessageBox::information(this, QStringLiteral("信息"), QStringLiteral("初始化成功"));
	}
	else
	{
		System->setTrackInfo(QString("Error at PR_Init, error code = %1").arg((int)retStatus));
	}
}

void CameraSetting::onUninitPRSystem()
{
	/*Vision::VisionStatus retStatus = Vision::PR_Init();
	if (retStatus == Vision::VisionStatus::OK)
	{
		QMessageBox::information(this, QStringLiteral("信息"), QStringLiteral("初始化成功"));
	}
	else
	{
		System->setTrackInfo(QString("Error at PR_Init, error code = %1").arg((int)retStatus));
	}*/
}

void CameraSetting::onClearAllRecords()
{
	if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
		QStringLiteral("全部清除 Records ？"), QMessageBox::Ok, QMessageBox::Cancel))
	{
		Vision::VisionStatus retStatus = Vision::PR_FreeAllRecord();
		if (retStatus == Vision::VisionStatus::OK)
		{
			QMessageBox::information(this, QStringLiteral("信息"), QStringLiteral("清除Records成功"));
		}
		else
		{
			System->setTrackInfo(QString("Error at PR_FreeAllRecord, error code = %1").arg((int)retStatus));
		}
	}
}

void CameraSetting::onAutoClearRecord(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	System->setParam("vision_record_auto_clear", (bool)data);	
}

void CameraSetting::onCaptureModeIndexChanged(int iIndex)
{
	int nCaptureMode = ui.comboBox_captureMode->currentIndex();

	ui.groupBox_imgDLP->setEnabled(2 == nCaptureMode);
	
	System->setParam("camera_capture_mode", nCaptureMode);

	QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("重启软件，相机采集模式才可以应用！"));
}

void CameraSetting::onSelCapturePath()
{
	QString str = QFileDialog::getExistingDirectory();
	if (!str.isEmpty())ui.lineEdit->setText(str);
}

void CameraSetting::onSaveCapturePath()
{
	QString str = ui.lineEdit->text();
	if (!str.isEmpty())System->setParam("camera_cap_image_path", str);
}

void CameraSetting::onSelImgDLP1Path()
{
	QString str = QFileDialog::getExistingDirectory();
	if (!str.isEmpty())ui.lineEdit_imgDLP1->setText(str);
}

void CameraSetting::onSaveImgDLP1Path()
{
	QString str = ui.lineEdit_imgDLP1->text();
	if (!str.isEmpty())System->setParam("camera_cap_image_dlp1_path", str);
}

void CameraSetting::onSelImgDLP2Path()
{
	QString str = QFileDialog::getExistingDirectory();
	if (!str.isEmpty())ui.lineEdit_imgDLP2->setText(str);
}

void CameraSetting::onSaveImgDLP2Path()
{
	QString str = ui.lineEdit_imgDLP2->text();
	if (!str.isEmpty())System->setParam("camera_cap_image_dlp2_path", str);
}

void CameraSetting::onCalibration()
{
	int nRow = ui.lineEdit_2->text().toInt();
	int nCol = ui.lineEdit_3->text().toInt();
	double dDist = ui.lineEdit_4->text().toDouble();
	double dScore = ui.lineEdit_7->text().toDouble();
	double dPixelOffsDiff = ui.lineEdit_8->text().toDouble();

	cv::Mat matImage = m_mainView->getImage();
	if (!matImage.empty())
	{
		Vision::PR_CALIBRATE_CAMERA_CMD  stCmd;
		stCmd.matInputImg = matImage;
		stCmd.szBoardPattern = cv::Size(nCol, nRow);
		stCmd.fPatternDist = dDist;
		stCmd.fMinTmplMatchScore = dScore;

		m_mainView->clearImage();

		Vision::PR_CALIBRATE_CAMERA_RPY stRpy;
		Vision::VisionStatus retStatus = Vision::PR_CalibrateCamera(&stCmd, &stRpy);
		if (retStatus == Vision::VisionStatus::OK)
		{
			double dResolutionX = stRpy.dResolutionX;
			double dResolutionY = stRpy.dResolutionY;

			ui.lineEdit_5->setText(QString("%1").arg(dResolutionX));
			ui.lineEdit_6->setText(QString("%1").arg(dResolutionY));

			System->setSysParam("CAM_RESOLUTION_X", dResolutionX);
			System->setSysParam("CAM_RESOLUTION_Y", dResolutionY);

			std::vector<aaAaa::aaSpline> splines;

			cv::Mat rvecIni;
			cv::Mat matRotionIni(stRpy.matInitialExtrinsicMatrix, cv::Rect(0, 0, 3, 3));
			cv::Rodrigues(matRotionIni, rvecIni);

			cv::Mat tvecIni(stRpy.matInitialExtrinsicMatrix, cv::Rect(3, 0, 1, 3));
			std::vector<cv::Point2f> vecProjectPointsIni;
			cv::projectPoints(stRpy.vecObjectPoints, rvecIni, tvecIni, stRpy.matInitialIntrinsicMatrix, stRpy.matDistCoeffs, vecProjectPointsIni);

			aaAaa::aaSpline splineXIni, splineYIni;
			for (int i = 0; i < stRpy.vecImagePoints.size(); i++)
			{
				cv::Point2f ptImage = stRpy.vecImagePoints[i];
				cv::Point2f ptProject = vecProjectPointsIni[i];

				splineXIni.addKnots(aaAaa::aaPoint(i, (ptImage.x - ptProject.x)));
				splineYIni.addKnots(aaAaa::aaPoint(i, (ptImage.y - ptProject.y)));
			}
			splineXIni.name = "Initial X";
			splines.push_back(splineXIni);
			splineYIni.name = "Initial Y";
			//spline.setLimit(0, 255, 255, 0);
			splines.push_back(splineYIni);

			QString runPath = QCoreApplication::applicationDirPath() + "/output/";

			QString fileName = runPath + "matInitialIntrinsicMatrix" + ".txt";
			printfMatToFile<double>(stRpy.matInitialIntrinsicMatrix, fileName);

			fileName = runPath + "matInitialExtrinsicMatrix" + ".txt";
			printfMatToFile<double>(stRpy.matInitialExtrinsicMatrix, fileName);

			fileName = runPath + "matIntrinsicMatrix" + ".txt";
			printfMatToFile<double>(stRpy.matIntrinsicMatrix, fileName);

			fileName = runPath + "matExtrinsicMatrix" + ".txt";
			printfMatToFile<double>(stRpy.matExtrinsicMatrix, fileName);

			fileName = runPath + "matDistCoeffs" + ".txt";
			printfMatToFile<double>(stRpy.matDistCoeffs, fileName);

			fileName = runPath + "vecObjectPoints" + ".txt";
			printf3fVectorToFile(stRpy.vecObjectPoints, fileName);

			fileName = runPath + "vecImagePoints" + ".txt";
			printf2fVectorToFile(stRpy.vecImagePoints, fileName);

			cv::Mat rvec;
			cv::Mat matRotion(stRpy.matExtrinsicMatrix, cv::Rect(0, 0, 3, 3));
			cv::Rodrigues(matRotion, rvec);

			cv::Mat tvec(stRpy.matExtrinsicMatrix, cv::Rect(3, 0, 1, 3));
			std::vector<cv::Point2f> vecProjectPoints;
			cv::projectPoints(stRpy.vecObjectPoints, rvec, tvec, stRpy.matIntrinsicMatrix, stRpy.matDistCoeffs, vecProjectPoints);
								
			aaAaa::aaSpline splineX, splineY;
			for (int i = 0; i < stRpy.vecImagePoints.size(); i++)
			{
				cv::Point2f ptImage = stRpy.vecImagePoints[i];
				cv::Point2f ptProject = vecProjectPoints[i];

				splineX.addKnots(aaAaa::aaPoint(i, (ptImage.x - ptProject.x)));
				splineY.addKnots(aaAaa::aaPoint(i, (ptImage.y - ptProject.y)));

				if (qAbs(ptImage.x - ptProject.x) > dPixelOffsDiff || qAbs(ptImage.y - ptProject.y) > dPixelOffsDiff)
				{
					int nRectSize = convertToPixel(dDist * 1000 / 2);
					cv::Rect select;
					select.x = ptImage.x - nRectSize/2;
					select.y = ptImage.y - nRectSize/2;
					select.width = nRectSize;
					select.height = nRectSize;
					cv::rectangle(stRpy.matCornerPointsImg, select, Scalar(0, 255, 255), 3, 8, 0);
				}
			}	
			splineX.name = "Corrected X";
			splines.push_back(splineX);
			splineY.name = "Corrected Y";
			//spline.setLimit(0, 255, 255, 0);
			splines.push_back(splineY);	

			m_pGraphicsEditor->setSplines(splines);
			m_pGraphicsEditor->show();

			m_mainView->setImage(stRpy.matCornerPointsImg);					
		}
		else
		{
			ui.lineEdit_5->setText(QString("%1").arg(0.0));
			ui.lineEdit_6->setText(QString("%1").arg(0.0));
			m_mainView->addImageText(QString("Error at Calibration, error code = %1").arg((int)retStatus));
		}
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
	}
	
}

void CameraSetting::onView3D()
{
	m_mainView->show3D();
}

double CameraSetting::convertToPixel(double umValue)
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

double CameraSetting::convertToUm(double pixel)
{
	double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	double dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

	double dResolution = (dResolutionX + dResolutionY) / 2;

	return pixel*dResolution;
}
