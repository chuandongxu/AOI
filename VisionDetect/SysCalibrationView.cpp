#include "SysCalibrationView.h"

#include <QFileDialog>

#include "visiondetect_global.h"

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/ICamera.h"
#include "../include/IDlp.h"
#include "../include/IVisionUI.h"
#include "../include/IMotion.h"
#include "../Common/eos.h"
#include <QMessageBox>
#include <QtMath>
#include <QThread>
#include <QDebug>
#include <QThreadPool>
#include <QDateTime>

#include "QCameraRunnable.h"
#include "QCaliGuideDialog.h"

#include "CryptLib.h"

#include "../lib/VisionLibrary/include/VisionAPI.h"
#define ToInt(value)                (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define MTR_READY_POS			0

using namespace AOI;

SysCalibrationView::SysCalibrationView(VisionCtrl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	initUI();
	initLimits();

	ui.pushButton_CalibGuide->setEnabled(true);
	ui.pushButton_CalibGuideNext->setEnabled(false);
	ui.pushButton_CalibGuideSkip->setEnabled(false);
	ui.pushButton_CalibGuidePrevious->setEnabled(false);
	m_nCaliGuideStep = 0;
	m_bGuideCali = false;
	m_pCameraRunnable = NULL;

	QString user;
	int level = 0;
	System->getUser(user, level);
	m_nLevel = level;
	if (USER_LEVEL_MANAGER > level)
	{		
	}
}

SysCalibrationView::~SysCalibrationView()
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
}

QWidget* SysCalibrationView::getTab()
{
	return ui.tab;
}

void SysCalibrationView::initUI()
{
	QStringList ls;

	//3D基面标定
	connect(ui.comboBox_selectDLP, SIGNAL(currentIndexChanged(int)), SLOT(onDLPIndexChanged(int)));
	int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
	for (int i = 0; i < nStationNum; i++)
	{
		ui.comboBox_selectDLP->addItem(QString("%1").arg(QStringLiteral("DLP%1").arg(i + 1)));
	}
	ui.comboBox_selectDLP->setCurrentIndex(0);

	bool b3DCaliGaussionFilter = System->getParam("3d_cali_gaussion_filter").toBool();
	bool b3DCaliReverseSeq = System->getParam("3d_cali_reverse_seq").toBool();
	QString sz3DCaliRstFile = System->getParam(QString("3d_cali_rst_filename_%1").arg(1)).toString();
	ui.checkBox_3DCaliGaussionFilter->setChecked(b3DCaliGaussionFilter);
	ui.checkBox_3DCaliReverseSeq->setChecked(b3DCaliReverseSeq);
	QString path = QApplication::applicationDirPath();
	QDir dir(path);
	ui.lineEdit_3DCaliRstFile->setText(QString("%1").arg(dir.absolutePath() + sz3DCaliRstFile));
	connect(ui.pushButton_3DCaliOpen, SIGNAL(clicked()), SLOT(on3DCaliOpen()));
	connect(ui.pushButton_3DCaliRstOpen, SIGNAL(clicked()), SLOT(on3DCaliRstOpen()));
	connect(ui.pushButton_3DCali, SIGNAL(clicked()), SLOT(on3DCali()));
	connect(ui.pushButton_Save3DDetectCaliParams, SIGNAL(clicked()), SLOT(onSave3DDetectCaliParams()));

	//3D标定
	connect(ui.comboBox_selectCaliType, SIGNAL(currentIndexChanged(int)), SLOT(onSelectCaliTypeChanged(int)));
	ls.clear();
	int nStepIndex = 0;
	for (int i = 5; i >= 1; i--)
	{
		ls = ls << QStringLiteral("H = -%1 mm").arg(i);
		m_caliStepMap.insert(nStepIndex++, -i);
	}
	for (int i = 1; i <= 5; i++)
	{
		ls = ls << QStringLiteral("H = %1 mm").arg(i);
		m_caliStepMap.insert(nStepIndex++, i);
	}
	ui.comboBox_selectCaliType->addItems(ls);
	ui.comboBox_selectCaliType->setCurrentIndex(0);	

	bool b3DDetectCaliUseThinPattern = System->getParam("3d_detect_cali_thin_pattern").toBool();
	bool b3DDetectCaliGaussionFilter = System->getParam("3d_detect_cali_gaussion_filter").toBool();
	bool b3DDetectCaliReverseSeq = System->getParam("3d_detect_cali_reverse_seq").toBool();
	bool b3DDetectCaliReverseHeight = System->getParam("3d_detect_cali_reverse_height").toBool();
	ui.checkBox_3DCaliUseThinPattern->setChecked(b3DDetectCaliUseThinPattern);
	ui.checkBox_3DDetectCaliGaussionFilter->setChecked(b3DDetectCaliGaussionFilter);
	ui.checkBox_3DDetectCaliReverseSeq->setChecked(b3DDetectCaliReverseSeq);
	ui.checkBox_3DDetectCaliReverseHeight->setChecked(b3DDetectCaliReverseHeight);
	double d3DDetectCaliMinIntDiff = System->getParam("3d_detect_cali_min_intensity_diff").toDouble();
	ui.lineEdit_3DDetectCaliMinIntDiff->setText(QString("%1").arg(d3DDetectCaliMinIntDiff));

	connect(ui.pushButton_3DDetectCaliOpen, SIGNAL(clicked()), SLOT(on3DDetectCaliOpen()));
	connect(ui.pushButton_3DDetectCali, SIGNAL(clicked()), SLOT(on3DDetectCali()));
	connect(ui.pushButton_Save3DDetectCaliHeightParams, SIGNAL(clicked()), SLOT(onSave3DDetectCaliHeightParams()));

	connect(ui.pushButton_3DDetectCaliComb, SIGNAL(clicked()), SLOT(on3DDetectCaliComb()));	

	connect(ui.pushButton_CalibGuide, SIGNAL(clicked()), SLOT(onCaliGuide()));
	connect(ui.pushButton_CalibGuideNext, SIGNAL(clicked()), SLOT(onCaliGuideNext()));
	connect(ui.pushButton_CalibGuideSkip, SIGNAL(clicked()), SLOT(onCaliGuideSkip()));
	connect(ui.pushButton_CalibGuidePrevious, SIGNAL(clicked()), SLOT(onCaliGuidePrevious()));

	connect(ui.pushButton_DecryptImages, SIGNAL(clicked()), SLOT(onDecryptImages()));
}

void SysCalibrationView::initLimits()
{
	//输入限制
	inputIntRangePos = new QIntValidator(0, 1000, this);
	inputIntRangeAll = new QIntValidator(-1000, 1000, this);
	inputDoubleRangePos = new QDoubleValidator(0, 1000, 10, this);
	inputDoubleRangeAll = new QDoubleValidator(-1000, 1000, 10, this);

	ui.lineEdit_3DDetectCaliMinIntDiff->setValidator(inputDoubleRangeAll);
}

IVisionUI* SysCalibrationView::getVisionUI()
{
	return getModule<IVisionUI>(UI_MODEL);
}

void SysCalibrationView::onDLPIndexChanged(int iState)
{
	int nDLPIndex = ui.comboBox_selectDLP->currentIndex();
	System->setParam("dlp_image_capture_index", nDLPIndex);

	QString sz3DCaliRstFile = System->getParam(QString("3d_cali_rst_filename_%1").arg(nDLPIndex + 1)).toString();
	QString path = QApplication::applicationDirPath();
	QDir dir(path);
	ui.lineEdit_3DCaliRstFile->setText(QString("%1").arg(dir.absolutePath() + sz3DCaliRstFile));
}

void SysCalibrationView::on3DCaliOpen()
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

			ui.lineEdit_3DCaliFile->setText(QString("%1").arg(filePath + "/"));
		}
		else
		{
			ui.lineEdit_3DCaliFile->setText("");
		}		
	}
	else
	{
		ui.lineEdit_3DCaliFile->setText("");
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请选择文件夹"));
	}
}

void SysCalibrationView::on3DCaliRstOpen()
{
	QString path = QApplication::applicationDirPath();
	path += "/3D/config/";

	QString picFilter = "Config(*.yml)";
	QString strFileName = QFileDialog::getOpenFileName(this, QStringLiteral("打开文件"), path, picFilter);

	QDir dir(path);
	if (!strFileName.isEmpty() && strFileName.indexOf(dir.absolutePath()) >= 0)
	{
		ui.lineEdit_3DCaliRstFile->setText(QString("%1").arg(strFileName));
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请在子目录下设置路径"));
	}
}

void SysCalibrationView::on3DCali()
{
	QString sz3DCaliFile = ui.lineEdit_3DCaliFile->text();
	if (sz3DCaliFile.isEmpty() && !m_bGuideCali)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	QString sz3DCaliRstFile = ui.lineEdit_3DCaliRstFile->text();
	if (sz3DCaliRstFile.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择校准输出文件"));
		return;
	}

	const int IMAGE_COUNT = 12;

	std::string strFolder = sz3DCaliFile.toStdString();

	Vision::PR_CALIB_3D_BASE_CMD stCmd;
	Vision::PR_CALIB_3D_BASE_RPY stRpy;
	if (!readImages(sz3DCaliFile, stCmd.vecInputImgs))
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("读取文件错误"));
		return;
	}

	int nDLPIndex = ui.comboBox_selectDLP->currentIndex();
	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("标定DLP%1的Base面...请耐心等待...").arg(nDLPIndex + 1), 0);
	QApplication::processEvents();

	bool b3DCaliGaussionFilter = ui.checkBox_3DCaliGaussionFilter->isChecked();
	bool b3DCaliReverseSeq = ui.checkBox_3DCaliReverseSeq->isChecked();
	stCmd.bEnableGaussianFilter = b3DCaliGaussionFilter;
	//stCmd.bReverseSeq = b3DCaliReverseSeq;
	stCmd.fRemoveHarmonicWaveK = 0.f;
	//stCmd.szMeasureWinSize = cv::Size(20, 20);

	Vision::VisionStatus retStatus = PR_Calib3DBase(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		std::string strResultMatPath = sz3DCaliRstFile.toStdString();
		cv::FileStorage fs(strResultMatPath, cv::FileStorage::WRITE);
		if (!fs.isOpened())
		{
			System->setTrackInfo(QStringLiteral("配置文件路径不正确！"));
			QSystem::closeMessage();
			return;
		}

		write(fs, "K1", stRpy.matThickToThinK);
		write(fs, "K2", stRpy.matThickToThinnestK);
		write(fs, "BaseWrappedAlpha", stRpy.matBaseWrappedAlpha);
		write(fs, "BaseWrappedBeta", stRpy.matBaseWrappedBeta);
		write(fs, "BaseWrappedGamma", stRpy.matBaseWrappedGamma);
		write(fs, "ReverseSeq", stRpy.bReverseSeq);
		fs.release();		

		//QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("标定成功"));
		System->setTrackInfo(QStringLiteral("标定DLP%1的Base成功").arg(nDLPIndex + 1));
	}
	else
	{
		getVisionUI()->addImageText(QString("Error at 3D Calibration, error code = %1").arg((int)retStatus));
		System->setTrackInfo(QString("Error at 3D Calibration, error code = %1").arg((int)retStatus));
	}

	QSystem::closeMessage();	
}

void SysCalibrationView::onSave3DDetectCaliParams()
{
	bool b3DCaliGaussionFilter = ui.checkBox_3DCaliGaussionFilter->isChecked();
	bool b3DCaliReverseSeq = ui.checkBox_3DCaliReverseSeq->isChecked();
	QString sz3DCaliRstFile = ui.lineEdit_3DCaliRstFile->text();
	int nDLPIndex = ui.comboBox_selectDLP->currentIndex();

	System->setParam("3d_cali_gaussion_filter", b3DCaliGaussionFilter);
	System->setParam("3d_cali_reverse_seq", b3DCaliReverseSeq);

	QString path = QApplication::applicationDirPath();
	QDir dir(path);
	//QString szSavePath = sz3DCaliRstFile.right(sz3DCaliRstFile.size() - sz3DCaliRstFile.indexOf("/3D/config/"));
	QString szSavePath = sz3DCaliRstFile.remove(dir.absolutePath());
	System->setParam(QString("3d_cali_rst_filename_%1").arg(nDLPIndex + 1), szSavePath);
}

void SysCalibrationView::onSelectCaliTypeChanged(int iState)
{
	int nCaliTypeIndex = ui.comboBox_selectCaliType->currentIndex();
}

void SysCalibrationView::on3DDetectCaliOpen()
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

			ui.lineEdit_3DDetectCaliFile->setText(QString("%1").arg(filePath + "/"));
		}
		else
		{
			ui.lineEdit_3DDetectCaliFile->setText("");
		}		
	}
	else
	{
		ui.lineEdit_3DDetectCaliFile->setText("");
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请选择文件夹"));
	}
}

void SysCalibrationView::on3DDetectCali()
{
	QString sz3DDetectCaliFile = ui.lineEdit_3DDetectCaliFile->text();
	if (sz3DDetectCaliFile.isEmpty() && !m_bGuideCali)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择图片"));
		return;
	}

	QString sz3DCaliRstFile = ui.lineEdit_3DCaliRstFile->text();
	if (sz3DCaliRstFile.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择校准输出文件"));
		return;
	}
	
	const int IMAGE_COUNT = 12;
	std::string strFolder = sz3DDetectCaliFile.toStdString();
	Vision::PR_CALC_3D_HEIGHT_CMD stCmd;
	Vision::PR_CALC_3D_HEIGHT_RPY stRpy;
	
	if (!readImages(sz3DDetectCaliFile, stCmd.vecInputImgs))
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("读取文件错误"));
		return;
	}

	int nDLPIndex = ui.comboBox_selectDLP->currentIndex();
	int nStepIndex = ui.comboBox_selectCaliType->currentIndex();
	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("标定DLP%1的H=%2mm面...请耐心等待...").arg(nDLPIndex + 1).arg(m_caliStepMap.value(nStepIndex)), 0);
	QApplication::processEvents();

	bool b3DDetectCaliUseThinPattern = ui.checkBox_3DCaliUseThinPattern->isChecked();
	bool b3DDetectGaussionFilter = ui.checkBox_3DDetectCaliGaussionFilter->isChecked();
	//bool b3DDetectReverseSeq = ui.checkBox_3DDetectCaliReverseSeq->isChecked();
	double d3DDetectMinIntDiff = ui.lineEdit_3DDetectCaliMinIntDiff->text().toDouble();
	stCmd.bEnableGaussianFilter = b3DDetectGaussionFilter;
	//stCmd.bReverseSeq = b3DDetectReverseSeq;
	stCmd.fMinAmplitude = d3DDetectMinIntDiff;
	stCmd.bUseThinnestPattern = b3DDetectCaliUseThinPattern;

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
	fileNode = fs["ReverseSeq"];	
	cv::read(fileNode, stCmd.bReverseSeq, 0);
	fs.release();	

	Vision::VisionStatus retStatus = PR_Calc3DHeight(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		QString path = QApplication::applicationDirPath();
		path += "/3D/config/";

		std::string strCalibDataFile((path + QString("DLP%1_").arg(ui.comboBox_selectDLP->currentIndex() + 1) + QString("H%1.yml").arg(nStepIndex + 1)).toStdString());
		cv::FileStorage fsCalibData(strCalibDataFile, cv::FileStorage::WRITE);
		if (!fsCalibData.isOpened())
		{
			System->setTrackInfo(QStringLiteral("配置文件路径不正确！"));
			QSystem::closeMessage();
			return;
		}
		cv::write(fsCalibData, "Phase", stRpy.matPhase);
		fsCalibData.release();

		cv::Mat matHeightResultImg = drawHeightGray(stRpy.matHeight);
		getVisionUI()->displayImage(matHeightResultImg);

		System->setTrackInfo(QStringLiteral("标定DLP%1的H%2面成功").arg(nDLPIndex + 1).arg(nStepIndex + 1));
	}
	else
	{
		getVisionUI()->addImageText(QString("Error at 3D Height Calculation, error code = %1").arg((int)retStatus));
	}

	QSystem::closeMessage();
}

void SysCalibrationView::onSave3DDetectCaliHeightParams()
{
	bool b3DDetectCaliUseThinPattern = ui.checkBox_3DCaliUseThinPattern->isChecked();
	bool b3DDetectCaliGaussionFilter = ui.checkBox_3DDetectCaliGaussionFilter->isChecked();
	bool b3DDetectCaliReverseSeq = ui.checkBox_3DDetectCaliReverseSeq->isChecked();
	bool b3DDetectCaliReverseHeight = ui.checkBox_3DDetectCaliReverseHeight->isChecked();
	double d3DDetectCaliMinIntDiff = ui.lineEdit_3DDetectCaliMinIntDiff->text().toDouble();

	System->setParam("3d_detect_cali_thin_pattern", b3DDetectCaliUseThinPattern);
	System->setParam("3d_detect_cali_gaussion_filter", b3DDetectCaliGaussionFilter);
	System->setParam("3d_detect_cali_reverse_seq", b3DDetectCaliReverseSeq);
	System->setParam("3d_detect_cali_reverse_height", b3DDetectCaliReverseHeight);
	System->setParam("3d_detect_cali_min_intensity_diff", d3DDetectCaliMinIntDiff);
}

void SysCalibrationView::on3DDetectCaliComb()
{
	QString path = QApplication::applicationDirPath();
	path += "/3D/config/";

	int nDLPIndex = ui.comboBox_selectDLP->currentIndex();
	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("导入DLP%1的标定结果...请耐心等待...").arg(nDLPIndex + 1), 0);
	QApplication::processEvents();

	Vision::PR_MOTOR_CALIB_3D_CMD stCmd;
	Vision::PR_MOTOR_CALIB_3D_RPY stRpy;

	for (int i = 0; i < m_caliStepMap.size(); i++)
	{
		int nStepIndex = m_caliStepMap.keys().at(i);

		cv::Mat matPhase;
		std::string strCalibDataFile((path + QString("DLP%1_").arg(ui.comboBox_selectDLP->currentIndex() + 1) + QString("H%1.yml").arg(nStepIndex + 1)).toStdString());
		cv::FileStorage fsCalibData(strCalibDataFile, cv::FileStorage::READ);
		if (!fsCalibData.isOpened())
		{
			System->setTrackInfo(QStringLiteral("配置文件路径不正确！"));
			QSystem::closeMessage();
			return;
		}
		cv::FileNode fileNode = fsCalibData["Phase"];
		cv::read(fileNode, matPhase, cv::Mat());
		fsCalibData.release();
		
		Vision::PairHeightPhase pairHeightPhase;
		pairHeightPhase.first = -m_caliStepMap.value(nStepIndex);	
		pairHeightPhase.second = matPhase;
		stCmd.vecPairHeightPhase.push_back(pairHeightPhase);
	}

	QSystem::closeMessage();

	QSystem::showMessage(QStringLiteral("提示"), QStringLiteral("合成DLP%1的标定结果...请耐心等待...").arg(nDLPIndex + 1), 0);
	QApplication::processEvents();

	Vision::VisionStatus retStatus = Vision::PR_MotorCalib3D(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		if (USER_LEVEL_MANAGER <= m_nLevel)
		{
			int i = 1;
			for (const auto &matResultImg : stRpy.vecMatResultImg) 
			{				
				std::string strDataFile = path.toStdString() + QString("ResultImg_DLP%1_%2").arg(nDLPIndex + 1).arg(i).toStdString() + ".bmp";
				cv::imwrite(strDataFile, matResultImg);

				QString nameEncrypt = path + QString("ResultImg_DLP%1_%2").arg(nDLPIndex + 1).arg(i) + ".ent";
				AOI::Crypt::EncryptFileNfg(strDataFile, nameEncrypt.toStdString());
				QFile::remove(QString(strDataFile.c_str()));

				++i;
			}

			if (i > 1) getVisionUI()->displayImage(stRpy.vecMatResultImg[0]);
		}


		QString fileName = QString("IntegrateCalibResult") + QString::number(nDLPIndex + 1, 'g', 2) + QString(".yml");

		std::string strCalibResultFile(path.toStdString() + fileName.toStdString());
		cv::FileStorage fsCalibResultData(strCalibResultFile, cv::FileStorage::WRITE);
		if (!fsCalibResultData.isOpened())
		{
			qDebug() << "Failed to open file: " << strCalibResultFile.c_str();
			QSystem::closeMessage();
			return;
		}
		cv::write(fsCalibResultData, "IntegratedK", stRpy.matIntegratedK);
		cv::write(fsCalibResultData, "Order3CurveSurface", stRpy.matOrder3CurveSurface);
		fsCalibResultData.release();

		System->setTrackInfo(QStringLiteral("合成DLP%1标定成功").arg(nDLPIndex + 1));
	}
	else
	{
		getVisionUI()->addImageText(QString("Error at 3D PR_MotorCalib3D Height, error code = %1").arg((int)retStatus));
		System->setTrackInfo(QString("Error at 3D PR_MotorCalib3D Height, error code = %1").arg((int)retStatus));
	}

	QSystem::closeMessage();
}

bool SysCalibrationView::startCaliGuide()
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return false;

	IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
	if (!pUI) return false;

	if (m_nCaliGuideStep <= 0)
	{
		if (pCam->getCameraNum() > 0)
		{
			if (!pCam->startUpCapture() || !pUI->startUpCapture())
			{
				QSystem::closeMessage();
				QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("相机初始化问题。"));
				return false;
			}
		}
		else
		{
			QSystem::closeMessage();
			QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("请检查相机是否连接。"));
			return false;
		}

		int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
		for (int i = 0; i < nStationNum; i++)
		{
			if (pDlp->isConnected(i))
			{
				if (!pDlp->startUpCapture(i))
				{
					QSystem::closeMessage();
					QMessageBox::warning(NULL, QStringLiteral("警告"), QStringLiteral("DLP启动失败！"));
					return false;
				}
			}
			else
			{
				QSystem::closeMessage();
				System->setTrackInfo(QString(QStringLiteral("DLP%0启动失败, 请检查硬件连接！")).arg(i + 1));
			}
		}
	}

	ui.toolBox_2->setEnabled(false);
	ui.comboBox_selectDLP->setEnabled(false);
	ui.pushButton_CalibGuideNext->setEnabled(true);
	ui.pushButton_CalibGuideSkip->setEnabled(true);
	ui.pushButton_CalibGuidePrevious->setEnabled(true);
	ui.pushButton_CalibGuide->setText(QStringLiteral("取消标定"));
	m_bGuideCali = true;
	m_nCaliGuideStep = 0;

	m_pCameraRunnable = new QCameraRunnable(this);
	QThreadPool::globalInstance()->start(m_pCameraRunnable, QThread::NormalPriority);

	bool bStationStarted = true;
	int nWaitTime = 30 * 5;
	do
	{
		bStationStarted = true;
		if (m_pCameraRunnable && !m_pCameraRunnable->isRunning())
		{
			bStationStarted = false;
		}
		QThread::msleep(200);
	} while (!bStationStarted && nWaitTime-- > 0);

	return true;
}

void SysCalibrationView::stopCaliGuide()
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return;

	IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
	if (!pDlp) return;

	IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
	if (!pUI) return;

	if (m_pCameraRunnable)
	{
		m_pCameraRunnable->quit();
	}

	bool bStationEnded = false;
	int nWaitTime = 30 * 5;
	do
	{
		bStationEnded = false;
		if (m_pCameraRunnable && !m_pCameraRunnable->isRunning())
		{
			bStationEnded = true;
		}
		QThread::msleep(200);
	} while (!bStationEnded && nWaitTime-- > 0);

	//if (QThreadPool::globalInstance()->activeThreadCount())
	QThreadPool::globalInstance()->waitForDone();
	//delete m_pCameraRunnable;
	//m_pCameraRunnable = NULL;

	if (pCam->getCameraNum() > 0)
	{
		pCam->endUpCapture();
	}
	pUI->endUpCapture();

	int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
	for (int i = 0; i < nStationNum; i++)
	{
		if (pDlp->isConnected(i))
		{
			if (!pDlp->endUpCapture(i)) continue;
		}
	}

	QSystem::closeMessage();

	ui.toolBox_2->setEnabled(true);
	ui.comboBox_selectDLP->setEnabled(true);
	ui.pushButton_CalibGuideNext->setEnabled(false);
	ui.pushButton_CalibGuideSkip->setEnabled(false);
	ui.pushButton_CalibGuidePrevious->setEnabled(false);
	ui.pushButton_CalibGuide->setText(QStringLiteral("标定向导"));

	m_nCaliGuideStep = 0;
	m_bGuideCali = false;
	m_guideImgMats.clear();
}

void SysCalibrationView::guideDisplayImages()
{
	cv::Mat matImg;
	if (guideReadImage(matImg))
	{
		getVisionUI()->setImage(matImg);
	}
}

void SysCalibrationView::startCameraCapturing()
{
	if (m_pCameraRunnable) m_pCameraRunnable->startCapture();
	//System->setParam("camera_cap_image_sw_enable", false);
}

bool SysCalibrationView::stopCameraCaptureing()
{
	//System->setParam("camera_cap_image_sw_enable", true);

	if (m_pCameraRunnable)
	{
		m_pCameraRunnable->stopCapture();

		bool bCaptureEnded = false;
		int nWaitTime = 30 * 5;
		do
		{
			bCaptureEnded = true;
			if (m_pCameraRunnable->isCapturing())
			{
				bCaptureEnded = false;
			}
			QThread::msleep(200);
		} while (!bCaptureEnded && nWaitTime-- > 0);

		if (bCaptureEnded)
		{
			return true;
		}
	}

	return false;
}

bool SysCalibrationView::guideReadImages(QVector<cv::Mat>& matImgs)
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return false;

	matImgs.clear();

	if (!pCam->selectCaptureMode(ICamera::TRIGGER_DLP_ALL))// all images
	{
		System->setTrackInfo(QString("startCapturing error"));
		return false;
	}

	if (!pCam->startCapturing())
	{
		System->setTrackInfo(QString("startCapturing error"));
		return false;
	}

	if (!pMotion->triggerCapturing(IMotion::TRIGGER_DLP, true))
	{
		System->setTrackInfo(QString("triggerCapturing error"));
		return false;
	}

	int nWaitTime = 10 * 100;
	while (!pCam->isCaptureImageBufferDone() && nWaitTime-- > 0)
	{
		QThread::msleep(10);
	}

	if (nWaitTime <= 0)
	{
		System->setTrackInfo(QString("CaptureImageBufferDone error"));
		return false;
	}

	int nCaptureNum = pCam->getImageBufferCaptureNum();
	for (int i = 0; i < nCaptureNum; i++)
	{
		cv::Mat matImage = pCam->getImageItemBuffer(i);

		matImgs.push_back(matImage);
	}

	System->setTrackInfo(QString("System captureImages Image Num: %1").arg(nCaptureNum));

	if (nCaptureNum != pCam->getImageBufferNum())
	{
		System->setTrackInfo(QString("System captureImages Image Num error: %1").arg(nCaptureNum));
		return false;
	}

	return true;
}

bool SysCalibrationView::guideReadImage(cv::Mat& matImg)
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return false;

	if (!pCam->selectCaptureMode(ICamera::TRIGGER_ONE))// 1 image
	{
		System->setTrackInfo(QString("startCapturing error"));
		return false;
	}

	if (!pCam->startCapturing())
	{
		System->setTrackInfo(QString("startCapturing error"));
		return false;
	}

	QVector<int> nPorts;

	nPorts.push_back(DO_LIGHT1_CH2);
	nPorts.push_back(DO_LIGHT2_CH1);
	nPorts.push_back(DO_CAMERA_TRIGGER2);

	pMotion->setDOs(nPorts, 1);
	QThread::msleep(10);
	pMotion->setDOs(nPorts, 0);

	int nWaitTime = 10 * 100;
	while (!pCam->isCaptureImageBufferDone() && nWaitTime-- > 0)
	{
		QThread::msleep(10);
	}

	if (nWaitTime <= 0)
	{
		System->setTrackInfo(QString("CaptureImageBufferDone error"));
		return false;
	}

	int nCaptureNum = pCam->getImageBufferCaptureNum();
	for (int i = 0; i < nCaptureNum; i++)
	{
		cv::Mat matImage = pCam->getImageItemBuffer(i);
		matImg = matImage;
	}

	System->setTrackInfo(QString("System captureImages Image Num: %1").arg(nCaptureNum));

	return true;
}

void SysCalibrationView::onCaliGuide()
{
	if (m_bGuideCali)
	{
		stopCaliGuide();
	}
	else
	{
		System->setParam("camera_hw_tri_enable", true);

		QCaliGuideDialog dlg;
		if (QDialog::Rejected == dlg.exec())
		{
			return;
		}

		if (!startCaliGuide())
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("启动标定失败！"));
		}
		else
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("开始标定..."));
		}
	}
}

void SysCalibrationView::onCaliGuideNext()
{
	IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return;

	switch (m_nCaliGuideStep)
	{
	case 0:
	{
		if (!pMotion->moveToPosGroup(MTR_READY_POS, true))// Ready Position
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("运动马达错误！"));
		}

		m_nCaliGuideStep++;
	}
	break;
	case 1:
	{
		ui.toolBox_2->setCurrentIndex(0);	

		startCameraCapturing();
		if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
			QStringLiteral("请放置DLP的Base标定块，确定开始采集图像？"), QMessageBox::Ok, QMessageBox::Cancel))
		{
			if (!stopCameraCaptureing())
			{
				QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("无法停止实时图像，请重新采集！"));
				return;
			}

			m_guideImgMats.clear();
			if (!guideReadImages(m_guideImgMats))
			{
				return;
			}
		}
		else
		{
			return;
		}

		//标定DLP1
		ui.comboBox_selectDLP->setCurrentIndex(0);
		QString szFileName = ui.lineEdit_3DCaliRstFile->text();
		QFile file(szFileName);
		if (!file.exists())
		{
			if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
				QStringLiteral("标定文件路径不存在，是否重新选择路径？"), QMessageBox::Ok, QMessageBox::Cancel))
			{
				on3DCaliRstOpen();
			}
		}
		on3DCali();

		//标定DLP2
		ui.comboBox_selectDLP->setCurrentIndex(1);
		szFileName = ui.lineEdit_3DCaliRstFile->text();
		file.setFileName(szFileName);
		if (!file.exists())
		{
			if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
				QStringLiteral("标定文件路径不存在，是否重新选择路径？"), QMessageBox::Ok, QMessageBox::Cancel))
			{
				on3DCaliRstOpen();
			}
		}
		on3DCali();

		//标定DLP3
		ui.comboBox_selectDLP->setCurrentIndex(2);
		szFileName = ui.lineEdit_3DCaliRstFile->text();
		file.setFileName(szFileName);
		if (!file.exists())
		{
			if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
				QStringLiteral("标定文件路径不存在，是否重新选择路径？"), QMessageBox::Ok, QMessageBox::Cancel))
			{
				on3DCaliRstOpen();
			}
		}
		on3DCali();

		//标定DLP4
		ui.comboBox_selectDLP->setCurrentIndex(3);
		szFileName = ui.lineEdit_3DCaliRstFile->text();
		file.setFileName(szFileName);
		if (!file.exists())
		{
			if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
				QStringLiteral("标定文件路径不存在，是否重新选择路径？"), QMessageBox::Ok, QMessageBox::Cancel))
			{
				on3DCaliRstOpen();
			}
		}
		on3DCali();

		m_nCaliGuideStep++;
	}
	break;
	case 2:
	{
		ui.toolBox_2->setCurrentIndex(1);	

		double zZeroPos = 0;		
		if (!pMotion->getCurrentPos(AXIS_MOTOR_Z, &zZeroPos))
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("获取马达位置错误！"));
		}

		for (int i = 0; i < m_caliStepMap.size(); i++)
		{
			int nStepIndex = m_caliStepMap.keys().at(i);
			int nStepValue = m_caliStepMap.value(nStepIndex);

			pMotion->moveTo(AXIS_MOTOR_Z, 0, zZeroPos + nStepValue, true);

			QThread::msleep(200);

			m_guideImgMats.clear();
			if (!guideReadImages(m_guideImgMats))
			{
				return;
			}

			ui.comboBox_selectCaliType->setCurrentIndex(nStepIndex);

			ui.comboBox_selectDLP->setCurrentIndex(0);
			on3DDetectCali();

			ui.comboBox_selectDLP->setCurrentIndex(1);
			on3DDetectCali();

			ui.comboBox_selectDLP->setCurrentIndex(2);
			on3DDetectCali();

			ui.comboBox_selectDLP->setCurrentIndex(3);
			on3DDetectCali();			
		}		

		m_nCaliGuideStep++;
	}
	break;	
	case 3:
	{
		ui.comboBox_selectDLP->setCurrentIndex(0);
		on3DDetectCaliComb();

		ui.comboBox_selectDLP->setCurrentIndex(1);
		on3DDetectCaliComb();

		ui.comboBox_selectDLP->setCurrentIndex(2);
		on3DDetectCaliComb();

		ui.comboBox_selectDLP->setCurrentIndex(3);
		on3DDetectCaliComb();

		m_nCaliGuideStep++;
	}
	break;
	default:
	{
		ui.toolBox_2->setCurrentIndex(2);
		stopCaliGuide();

		if (!pMotion->moveToPosGroup(MTR_READY_POS, true))// Ready Position
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("运动马达错误！"));
		}

		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("自动标定向导完成！"));
	}
	break;
	}
}

void SysCalibrationView::onCaliGuideSkip()
{
	if (m_nCaliGuideStep > 0)
	{
		m_nCaliGuideStep += 1;
		onCaliGuideNext();
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("无法跳过当前标定，请下一步操作或者取消标定！"));
	}
}

void SysCalibrationView::onCaliGuidePrevious()
{
	if (m_nCaliGuideStep > 0)
	{
		m_nCaliGuideStep -= 1;
		onCaliGuideNext();
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("无法继续上一步标定，请下一步操作或者取消标定！"));
	}
}

void SysCalibrationView::onDecryptImages()
{
	QString path = QApplication::applicationDirPath();
	path += "/capture/";

	QString filePath = QFileDialog::getExistingDirectory(this, QStringLiteral("打开图片文件夹"), path);

	if (!filePath.isEmpty())
	{
		decrptImages(filePath);
	}
}

double SysCalibrationView::convertToPixel(double umValue)
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

double SysCalibrationView::convertToUm(double pixel)
{
	double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	double dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

	double dResolution = (dResolutionX + dResolutionY) / 2;

	return pixel*dResolution;
}

bool SysCalibrationView::convertToGrayImage(QString& szFilePath, cv::Mat &matGray)
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

bool SysCalibrationView::decrptImages(QString& szFilePath)
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

	for (int i = 0; i < file_count; i++)
	{
		QFileInfo file_info = list.at(i);
		QString suffix = file_info.suffix();
		if (QString::compare(suffix, QString("ent"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			QString nameDecrypt = file_info.absolutePath() + "/" + file_info.baseName() + ".bmp";
			AOI::Crypt::DecryptFileNfg(absolute_file_path.toStdString(), nameDecrypt.toStdString());
		}		
		else
		{
			return false;
		}
	}

	return true;
}

bool SysCalibrationView::readImages(QString& szFilePath, AOI::Vision::VectorOfMat& matImgs)
{
	const int IMAGE_COUNT = 12;

	if (m_bGuideCali)
	{
		int nDLPIndex = ui.comboBox_selectDLP->currentIndex();

		for (int i = 0; i < IMAGE_COUNT; i++)
		{
			matImgs.push_back(m_guideImgMats.at(i + nDLPIndex * IMAGE_COUNT));
		}
		return true;
	}

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

cv::Mat SysCalibrationView::drawHeightGray(const cv::Mat &matHeight)
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

cv::Mat SysCalibrationView::drawHeightGrid(const cv::Mat &matHeight, int nGridRow, int nGridCol) {
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

cv::Mat SysCalibrationView::drawHeightGrid2(const cv::Mat &matHeight, int nGridRow, int nGridCol)
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
