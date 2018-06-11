#include "VisionCtrl.h"
#include "MatHeader.h"

#include "visiondetect_global.h"

#include "VLCellEditor.h"
#include "VLProfileEditor.h"

#include <QDateTime>
#include <QApplication>
#include <QVector>
#include <qmath.h>

#include "../Common/ModuleMgr.h"
#include "../Common/SystemData.h"
#include "../include/IData.h"
#include "../include/IdDefine.h"
#include "../Common/eos.h"

#include "../lib/VisionLibrary/include/VisionAPI.h"

#include "../DataModule/QDetectObj.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define ToInt(value)                (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))

const int RUN_VIEW_CTRL = 1;

using namespace AOI;

VisionCtrl::VisionCtrl(QObject *parent)
	: QObject(parent)
{
	//m_pVLCellTmpEditor = new QVLCellEditor(EM_DATA_TYPE_TMP);
	m_pVLCellObjEditor = new QVLCellEditor(EM_DATA_TYPE_OBJ);
	m_pVLProflieEditor = new QVLProfileEditor(this);

	Vision::PR_VERSION_INFO info;
	Vision::PR_GetVersion(&info);

	System->setTrackInfo(QString("Vision Version:%1").arg(info.chArrVersion));
    m_strVisionLibraryVersion = info.chArrVersion;

	//cv::Mat matNan(3, 3, CV_32FC1, NAN);
	//cv::Mat matCmpResult = cv::Mat(matNan == matNan);
	//int nCount = cv::countNonZero(matCmpResult);
	//System->setTrackInfo(QString("find NAN cout %1").arg(matCmpResult.total() - nCount));

	Vision::VisionStatus retStatus = Vision::PR_Init();
	if (retStatus != Vision::VisionStatus::OK)
	{
		System->setTrackInfo("Vision Lib Init Error!");
	}

	//for (int i = 0; i < STATION_COUNT; i++)
	//{
	//	m_matBaseSurfaces.push_back(cv::Mat());
	//}

	int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
	for (int i = 0; i < nStationNum; i++)
	{
		m_stCalcHeightCmds.push_back(Vision::PR_CALC_3D_HEIGHT_CMD());
	}
}

VisionCtrl::~VisionCtrl()
{
	//if (m_pVLCellTmpEditor)
	//{
	//	delete m_pVLCellTmpEditor;
	//	m_pVLCellTmpEditor = NULL;
	//}

	if (m_pVLProflieEditor)
	{
		delete m_pVLProflieEditor;
		m_pVLProflieEditor = NULL;
	}   
}


//QVLCellEditor* VisionCtrl::getCellTmpEditor()
//{
//	return m_pVLCellTmpEditor;
//}

QVLCellEditor* VisionCtrl::getCellObjEditor()
{
	return m_pVLCellObjEditor;
}

QVLProfileEditor* VisionCtrl::getProfileEditor()
{
	return m_pVLProflieEditor;
}

bool VisionCtrl::loadCmdData(int nStation)
{
	if (m_stCalcHeightCmds[nStation - 1].matThickToThinK.empty())
	{
		bool b3DDetectCaliUseThinPattern = System->getParam("3d_detect_thin_pattern").toBool();
		bool b3DDetectGaussionFilter = System->getParam("3d_detect_gaussion_filter").toBool();
		//bool b3DDetectReverseSeq = System->getParam("3d_detect_reverse_seq").toBool();
		double d3DDetectMinIntDiff = System->getParam("3d_detect_min_intensity_diff").toDouble();
		double d3DDetectPhaseShift = System->getParam("3d_detect_phase_shift").toDouble();
		m_stCalcHeightCmds[nStation - 1].bEnableGaussianFilter = b3DDetectGaussionFilter;
		//m_stCalcHeightCmds[nStation - 1].bReverseSeq = b3DDetectReverseSeq;
		m_stCalcHeightCmds[nStation - 1].fMinAmplitude = d3DDetectMinIntDiff;
		m_stCalcHeightCmds[nStation - 1].bUseThinnestPattern = b3DDetectCaliUseThinPattern;
		m_stCalcHeightCmds[nStation - 1].fPhaseShift = d3DDetectPhaseShift;

		cv::Mat matBaseSurfaceParam;

		// read config file
		QString path = QApplication::applicationDirPath();
		QString sz3DCaliRstFile = path + System->getParam(QString("3d_cali_rst_filename_%1").arg(nStation)).toString();
		std::string strResultMatPath = sz3DCaliRstFile.toStdString();
		cv::FileStorage fs(strResultMatPath, cv::FileStorage::READ);
		cv::FileNode fileNode = fs["K1"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].matThickToThinK, cv::Mat());
		fileNode = fs["K2"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].matThickToThinnestK, cv::Mat());
		fileNode = fs["BaseWrappedAlpha"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].matBaseWrappedAlpha, cv::Mat());
		fileNode = fs["BaseWrappedBeta"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].matBaseWrappedBeta, cv::Mat());
		fileNode = fs["BaseWrappedGamma"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].matBaseWrappedGamma, cv::Mat());
		fileNode = fs["ReverseSeq"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].bReverseSeq, 0);
		fs.release();

		//if (m_matBaseSurfaces[nStation-1].empty())
		//{
		//	Vision::PR_CALC_3D_BASE_CMD stCalc3DBaseCmd;
		//	Vision::PR_CALC_3D_BASE_RPY stCalc3DBaseRpy;
		//	stCalc3DBaseCmd.matBaseSurfaceParam = matBaseSurfaceParam;
		//	PR_Calc3DBase(&stCalc3DBaseCmd, &stCalc3DBaseRpy);
		//	if (Vision::VisionStatus::OK != stCalc3DBaseRpy.enStatus) {
		//		System->setTrackInfo(QString("PR_Calc3DBase fail. Status = ").arg(ToInt32(stCalc3DBaseRpy.enStatus)));
		//	}
		//	m_matBaseSurfaces[nStation-1] = stCalc3DBaseRpy.matBaseSurface;
		//}
		//stCmd.matBaseSurface = m_matBaseSurfaces[nStation-1];

		path = QApplication::applicationDirPath();
		path += "/3D/config/";

		QString fileName = QString("IntegrateCalibResult") + QString::number(nStation, 'g', 2) + QString(".yml");

		std::string strIntegratedCalibResultPath = path.toStdString() + fileName.toStdString();
		cv::FileStorage fsIntegrated(strIntegratedCalibResultPath, cv::FileStorage::READ);
		cv::FileNode fileNodeIntegrated = fsIntegrated["IntegratedK"];
		cv::read(fileNodeIntegrated, m_stCalcHeightCmds[nStation - 1].matIntegratedK, cv::Mat());
		fileNodeIntegrated = fsIntegrated["Order3CurveSurface"];
		cv::read(fileNodeIntegrated, m_stCalcHeightCmds[nStation - 1].matOrder3CurveSurface, cv::Mat());
		fsIntegrated.release();
	}

	return true;
}

bool VisionCtrl::calculate3DHeight(int nStation, QVector<cv::Mat>& imageMats, cv::Mat& heightMat, cv::Mat& matHeightResultImg)
{
	const int IMAGE_COUNT = 12;

	if (imageMats.size() < IMAGE_COUNT) return false;

	//Vision::PR_FAST_CALC_3D_HEIGHT_CMD stCmd;
	if (m_stCalcHeightCmds[nStation - 1].matThickToThinK.empty())
	{		
		for (int i = 0; i < IMAGE_COUNT; ++i)
		{
			m_stCalcHeightCmds[nStation - 1].vecInputImgs.push_back(imageMats[i]);
		}

		bool b3DDetectCaliUseThinPattern = System->getParam("3d_detect_thin_pattern").toBool();
		bool b3DDetectGaussionFilter = System->getParam("3d_detect_gaussion_filter").toBool();
		//bool b3DDetectReverseSeq = System->getParam("3d_detect_reverse_seq").toBool();
		double d3DDetectMinIntDiff = System->getParam("3d_detect_min_intensity_diff").toDouble();
		double d3DDetectPhaseShift = System->getParam("3d_detect_phase_shift").toDouble();
		m_stCalcHeightCmds[nStation - 1].bEnableGaussianFilter = b3DDetectGaussionFilter;
		//m_stCalcHeightCmds[nStation - 1].bReverseSeq = b3DDetectReverseSeq;
		m_stCalcHeightCmds[nStation - 1].fMinAmplitude = d3DDetectMinIntDiff;
		m_stCalcHeightCmds[nStation - 1].bUseThinnestPattern = b3DDetectCaliUseThinPattern;
		m_stCalcHeightCmds[nStation - 1].fPhaseShift = d3DDetectPhaseShift;

		cv::Mat matBaseSurfaceParam;

		// read config file
		QString path = QApplication::applicationDirPath();
		QString sz3DCaliRstFile = path + System->getParam(QString("3d_cali_rst_filename_%1").arg(nStation)).toString();
		std::string strResultMatPath = sz3DCaliRstFile.toStdString();
		cv::FileStorage fs(strResultMatPath, cv::FileStorage::READ);
        if (!fs.isOpened()) {
            System->setTrackInfo(QString("Failed to open 3D calibration result file %1").arg(strResultMatPath.c_str()));
            return false;
        }

		cv::FileNode fileNode = fs["K1"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].matThickToThinK, cv::Mat());
		fileNode = fs["K2"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].matThickToThinnestK, cv::Mat());
		fileNode = fs["BaseWrappedAlpha"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].matBaseWrappedAlpha, cv::Mat());
		fileNode = fs["BaseWrappedBeta"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].matBaseWrappedBeta, cv::Mat());
		fileNode = fs["BaseWrappedGamma"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].matBaseWrappedGamma, cv::Mat());
		fileNode = fs["ReverseSeq"];
		cv::read(fileNode, m_stCalcHeightCmds[nStation - 1].bReverseSeq, 0);
		fs.release();

		//if (m_matBaseSurfaces[nStation-1].empty())
		//{
		//	Vision::PR_CALC_3D_BASE_CMD stCalc3DBaseCmd;
		//	Vision::PR_CALC_3D_BASE_RPY stCalc3DBaseRpy;
		//	stCalc3DBaseCmd.matBaseSurfaceParam = matBaseSurfaceParam;
		//	PR_Calc3DBase(&stCalc3DBaseCmd, &stCalc3DBaseRpy);
		//	if (Vision::VisionStatus::OK != stCalc3DBaseRpy.enStatus) {
		//		System->setTrackInfo(QString("PR_Calc3DBase fail. Status = ").arg(ToInt32(stCalc3DBaseRpy.enStatus)));
		//	}
		//	m_matBaseSurfaces[nStation-1] = stCalc3DBaseRpy.matBaseSurface;
		//}
		//stCmd.matBaseSurface = m_matBaseSurfaces[nStation-1];

		path = QApplication::applicationDirPath();
		path += "/3D/config/";

		QString fileName = QString("IntegrateCalibResult") + QString::number(nStation, 'g', 2) + QString(".yml");

		std::string strIntegratedCalibResultPath = path.toStdString() + fileName.toStdString();
		cv::FileStorage fsIntegrated(strIntegratedCalibResultPath, cv::FileStorage::READ);
		cv::FileNode fileNodeIntegrated = fsIntegrated["IntegratedK"];
		cv::read(fileNodeIntegrated, m_stCalcHeightCmds[nStation - 1].matIntegratedK, cv::Mat());
		fileNodeIntegrated = fsIntegrated["Order3CurveSurface"];
		cv::read(fileNodeIntegrated, m_stCalcHeightCmds[nStation - 1].matOrder3CurveSurface, cv::Mat());
		fsIntegrated.release();
	}
	else
	{
		m_stCalcHeightCmds[nStation - 1].vecInputImgs.clear();
		for (int i = 0; i < IMAGE_COUNT; ++i)
		{
			m_stCalcHeightCmds[nStation - 1].vecInputImgs.push_back(imageMats[i]);
		}
	}

	Vision::PR_CALC_3D_HEIGHT_RPY stRpy;
	Vision::VisionStatus retStatus = PR_Calc3DHeight(&(m_stCalcHeightCmds[nStation - 1]), &stRpy);
	if (Vision::VisionStatus::OK == retStatus)
	{
		heightMat = stRpy.matHeight;
		//cv::patchNaNs(heightMat, 0.0);

		matHeightResultImg = drawHeightGray(stRpy.matHeight);
		return true;
	}
	else
	{
		System->setTrackInfo(QString("Error at 3D Height Calculation, error code = %1").arg((int)retStatus));
		return false;
	}
}

bool VisionCtrl::generateAverageImage(const QVector<cv::Mat>& imageMats, cv::Mat& grayMat)
{
	const int IMAGE_COUNT = 4;
	if (imageMats.size() < IMAGE_COUNT) return false;

    cv::Mat matSum = cv::Mat::zeros(imageMats[0].size(), imageMats[0].type());
	for (const auto &matImage : imageMats)
        matSum = matSum + matImage;
	grayMat = matSum / IMAGE_COUNT;
	return true;
}

bool VisionCtrl::matchPosition(cv::Mat& matDisplay, QVector<QDetectObj*>& objTests)
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return false;

	for (int i = 0; i < pData->getObjNum(); i++)
	{
		QDetectObj* pObj = pData->getObj(i);
		if (pObj)
		{
			cv::RotatedRect rtRect = pObj->getLoc();
			rtRect.size.width *= 2;
			rtRect.size.height *= 2;

			cv::Rect searchRect = rtRect.boundingRect();
			int nImgWidth = matDisplay.cols;
			int nImgHeight = matDisplay.rows;
			if (searchRect.x < 0) searchRect.x = 0;
			if (searchRect.y < 0) searchRect.y = 0;
			if (searchRect.x + searchRect.width > nImgWidth) searchRect.width = nImgWidth - searchRect.x;
			if (searchRect.y + searchRect.height > nImgHeight) searchRect.height = nImgHeight - searchRect.y;

			bool bTmpMatchSubPixel = System->getParam("srh_tmp_sub_pixel").toBool();

			Vision::PR_MATCH_TEMPLATE_CMD  stCmd;
			stCmd.matInputImg = matDisplay;
			stCmd.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
			stCmd.nRecordId = pObj->getRecordID();
			stCmd.rectSrchWindow = searchRect;
			stCmd.bSubPixelRefine = bTmpMatchSubPixel;

			int nTmpMatchObjMotion = System->getParam("srh_tmp_obj_motion").toInt();
			stCmd.enMotion = (Vision::PR_OBJECT_MOTION)nTmpMatchObjMotion;

			Vision::PR_MATCH_TEMPLATE_RPY stRpy;
			Vision::VisionStatus retStatus = Vision::PR_MatchTmpl(&stCmd, &stRpy);
			if (retStatus == Vision::VisionStatus::OK)
			{
				QDetectObj* pTestObj = new QDetectObj(*pObj);

				pTestObj->setObjPos(stRpy.ptObjPos.x, stRpy.ptObjPos.y);

				RotatedRect rtFrame = pTestObj->getFrame();
				//rtFrame.center.x += pTestObj->getX() - pObj->getX();
				//rtFrame.center.y += pTestObj->getY() - pObj->getY();
				//pTestObj->setFrame(rtFrame);

				cv::rectangle(matDisplay, rtFrame.boundingRect(), Scalar(255, 0, 0), 1);

				objTests.push_back(pTestObj);
			}
			else
			{				
				System->setTrackInfo(QString("Error at Template Match, error code = %1").arg((int)retStatus));
			}
		}
	}

	return true;
}

bool VisionCtrl::calculateDetectHeight(cv::Mat& matHeight, QVector<QDetectObj*>& objTests)
{
	for (int i = 0; i < objTests.size(); i++)
	{
		QDetectObj* pObj = objTests[i];
		if (pObj)
		{
			for (int j = 0; j < pObj->getHeightDetectNum(); j++)
			{
				// 获取检测位置的Base区域
				QVector<cv::Rect> baseRects;
				pObj->getHeightDetectBase(j, baseRects);

				int nDataBaseMin = System->getParam("3d_detect_data_base_min").toInt();
				int nDataBaseMax = System->getParam("3d_detect_data_base_max").toInt();

				//std::vector<float> vecXt, vecYt, vecPhaseTmp;
				//cv::Mat matX, matY;
				//meshgrid<float>(1.f, 1.f, matHeight.cols, 1.f, 1.f, matHeight.rows, matX, matY);

				//AOI::Vision::VectorOfPoint vecBaseLocations;
				//for (int m = 0; m < baseRects.size(); m++)
				//{
				//	//平面拟合，拉平Base面
				//	cv::Mat matMask = cv::Mat::zeros(matHeight.size(), CV_8UC1);
				//	cv::Mat matROI(matMask, baseRects[m]);
				//	cv::Mat matNotNan = matHeight(baseRects[m]) == matHeight(baseRects[m]);
				//	cv::Mat matNan = 255 - matNotNan;
				//	matROI.setTo(1);
				//	matROI.setTo(0, matNan);
				//	
				//	AOI::Vision::VectorOfPoint vecPtLocations;
				//	cv::findNonZero(matMask, vecPtLocations);
				//	cv::Mat matHeight = matHeight;
				//	std::sort(vecPtLocations.begin(), vecPtLocations.end(), [&matHeight](const cv::Point &pt1, const cv::Point &pt2) {
				//		return matHeight.at<float>(pt1) < matHeight.at<float>(pt2);
				//	});
				//	AOI::Vision::VectorOfPoint vecTrimedLocations(vecPtLocations.begin() + ToInt32(vecPtLocations.size() * nDataBaseMin / 100.0), vecPtLocations.begin() + ToInt32(vecPtLocations.size() * nDataBaseMax / 100.0));
				//	vecBaseLocations.insert(vecBaseLocations.end(), vecTrimedLocations.begin(), vecTrimedLocations.end());

				//	vecXt.reserve(vecXt.size() + vecTrimedLocations.size());
				//	vecYt.reserve(vecYt.size() + vecTrimedLocations.size());
				//	vecPhaseTmp.reserve(vecPhaseTmp.size() + vecTrimedLocations.size());
				//	for (const auto &point : vecTrimedLocations) {
				//		vecXt.push_back(matX.at<float>(point));
				//		vecYt.push_back(matY.at<float>(point));
				//		vecPhaseTmp.push_back(matHeight.at<float>(point));
				//	}					
				//}

				//cv::Mat matXX;
				//matXX.push_back(cv::Mat(vecXt).reshape(1, 1));
				//matXX.push_back(cv::Mat(vecYt).reshape(1, 1));
				//matXX.push_back(cv::Mat(cv::Mat::ones(1, ToInt(vecPhaseTmp.size()), CV_32FC1)));
				//cv::transpose(matXX, matXX);
				//cv::Mat matYY(vecPhaseTmp);
				//cv::Mat matK;
				//cv::solve(matXX, matYY, matK, cv::DecompTypes::DECOMP_QR);

				//float k1 = matK.at<float>(0);
				//float k2 = matK.at<float>(1);
				//float k3 = matK.at<float>(2);

				////计算Base值
				//float zValueBase = 0;
				//for (int i = 0; i < vecBaseLocations.size(); i++)
				//{
				//	float zDiffValue = vecBaseLocations[i].x* k1 + vecBaseLocations[i].y* k2 + k3;

				//	float zTestValue = matHeight.at<float>(vecBaseLocations[i]);

				//	float zValue = zTestValue - zDiffValue;

				//	zValueBase += zValue;
				//}
				//zValueBase = vecBaseLocations.size() > 0 ? zValueBase / vecBaseLocations.size() : 0;

				//// 获取检测位置的Detect区域
				//cv::Rect detectRect = pObj->getHeightDetect(j).boundingRect();

				//int nDataDetectMin = System->getParam("3d_detect_data_detect_min").toInt();
				//int nDataDetectMax = System->getParam("3d_detect_data_detect_max").toInt();

				//cv::Mat matMask = cv::Mat::zeros(matHeight.size(), CV_8UC1);
				//cv::Mat matROI(matMask, detectRect);
				//cv::Mat matNotNan = matHeight(detectRect) == matHeight(detectRect);
				//cv::Mat matNan = 255 - matNotNan;
				//matROI.setTo(1);
				//matROI.setTo(0, matNan);

				//AOI::Vision::VectorOfPoint vecPtLocations;
				//cv::findNonZero(matMask, vecPtLocations);
				//cv::Mat matHeight = matHeight;
				//std::sort(vecPtLocations.begin(), vecPtLocations.end(), [&matHeight](const cv::Point &pt1, const cv::Point &pt2) {
				//	return matHeight.at<float>(pt1) < matHeight.at<float>(pt2);
				//});
				//AOI::Vision::VectorOfPoint vecTrimedLocations(vecPtLocations.begin() + ToInt32(vecPtLocations.size() * nDataDetectMin / 100.0), vecPtLocations.begin() + ToInt32(vecPtLocations.size() * nDataDetectMax / 100.0));

				//float zValueDetect= 0;
				//for (int i = 0; i < vecTrimedLocations.size(); i++)
				//{
				//	float zDiffValue = vecTrimedLocations[i].x* k1 + vecTrimedLocations[i].y* k2 + k3;

				//	float zTestValue = matHeight.at<float>(vecTrimedLocations[i]);

				//	float zValue = zTestValue - zDiffValue;

				//	zValueDetect += zValue;
				//}
				//zValueDetect = vecTrimedLocations.size() > 0 ? zValueDetect / vecTrimedLocations.size() : 0;

				////输出结果
				//float dDetectHeight = zValueDetect - zValueBase;		

				cv::Rect detectRect = pObj->getHeightDetect(j).boundingRect();

				Vision::PR_CALC_3D_HEIGHT_DIFF_CMD  stCmd;
				stCmd.matHeight = matHeight;
				stCmd.fEffectHRatioStart = nDataBaseMin * 1.0 / 100.0;
				stCmd.fEffectHRatioEnd = nDataBaseMax * 1.0 / 100.0;
				for (int m = 0; m < baseRects.size(); m++)
				{
					stCmd.vecRectBases.push_back(baseRects[m]);
				}
				stCmd.rectROI = detectRect;	

				Vision::PR_CALC_3D_HEIGHT_DIFF_RPY stRpy;
				Vision::VisionStatus retStatus = Vision::PR_Calc3DHeightDiff(&stCmd, &stRpy);
				if (retStatus == Vision::VisionStatus::OK)
				{
					pObj->setHeightValue(j, stRpy.fHeightDiff);
				}
				else
				{
					System->setTrackInfo(QString("Error at PR_Calc3DHeightDiff, error code = %1").arg((int)retStatus));
				}
			}
		}
	}

	return true;
}

bool VisionCtrl::merge3DHeight(QVector<cv::Mat>& matHeights, cv::Mat& matHeight)
{
	if (2 == matHeights.size())
	{
		Vision::PR_MERGE_3D_HEIGHT_CMD stCmd;
		Vision::PR_MERGE_3D_HEIGHT_RPY stRpy;

		for (int i = 0; i < matHeights.size(); i++)
		{
			stCmd.vecMatHeight.push_back(matHeights[i]);
		}
		double d3DDetectHeightDiffThd = System->getParam("3d_detect_height_diff_threshold").toDouble();
		stCmd.fHeightDiffThreshold = d3DDetectHeightDiffThd;
		double d3DDetectHeightNoiseThd = System->getParam("3d_detect_height_noise_threshold").toDouble();
		stCmd.fRemoveLowerNoiseRatio = d3DDetectHeightNoiseThd;

		Vision::VisionStatus retStatus = PR_Merge3DHeight(&stCmd, &stRpy);
		if (retStatus == Vision::VisionStatus::OK)
		{
			matHeight = stRpy.matHeight;
		}
		else
		{
			System->setTrackInfo(QString("Error at PR_Merge3DHeight, error code = %1").arg((int)retStatus));
		}
	}
	else // 4 DLPs
	{
		cv::Mat matHeightMerges[2];
		for (int j = 0; j < 2; j++)
		{
			Vision::PR_MERGE_3D_HEIGHT_CMD stCmd;
			Vision::PR_MERGE_3D_HEIGHT_RPY stRpy;

			stCmd.vecMatHeight.push_back(matHeights[j + 0]);
			stCmd.vecMatHeight.push_back(matHeights[j + 2]);

			double d3DDetectHeightDiffThd = System->getParam("3d_detect_height_diff_threshold").toDouble();
			stCmd.fHeightDiffThreshold = d3DDetectHeightDiffThd;
			double d3DDetectHeightNoiseThd = System->getParam("3d_detect_height_noise_threshold").toDouble();
			stCmd.fRemoveLowerNoiseRatio = d3DDetectHeightNoiseThd;

			Vision::VisionStatus retStatus = PR_Merge3DHeight(&stCmd, &stRpy);
			if (retStatus == Vision::VisionStatus::OK)
			{
				matHeightMerges[j] = stRpy.matHeight;
			}
			else
			{
				System->setTrackInfo(QString("Error at PR_Merge3DHeight, error code = %1").arg((int)retStatus));
			}
		}

		matHeight = (matHeightMerges[0] + matHeightMerges[1]) / 2;
	}

	return true;
}

bool VisionCtrl::mergeImage(QVector<cv::Mat>& matInputImages, QVector<cv::Mat>& matOutputImages)
{
	Vision::PR_COMBINE_IMG_CMD stCmd;
	Vision::PR_COMBINE_IMG_RPY stRpy;

	//for (int i = matInputImages.size() - 1; i >= 0; i--)
	//{
	//	stCmd.vecInputImages.push_back(matInputImages[i]);
	//}

	for (int i = 0; i < matInputImages.size(); i++)
	{
		stCmd.vecInputImages.push_back(matInputImages[i]);
	}

	double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	double dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

	auto nScanDirection = System->getParam("scan_image_Direction").toInt();

	stCmd.nCountOfImgPerFrame = System->getParam("scan_image_OneFrameImageCount").toInt();
	stCmd.nCountOfFrameX = System->getParam("scan_image_FrameCountX").toInt();
	stCmd.nCountOfFrameY = System->getParam("scan_image_FrameCountY").toInt();
	stCmd.nOverlapX = ToInt(System->getParam("scan_image_OverlapX").toDouble() / dResolutionX + 0.5);
	stCmd.nOverlapY = ToInt(System->getParam("scan_image_OverlapY").toDouble() / dResolutionY + 0.5);
	stCmd.nCountOfImgPerRow = System->getParam("scan_image_RowImageCount").toInt();
	stCmd.enScanDir = static_cast<Vision::PR_SCAN_IMAGE_DIR> (nScanDirection);

	Vision::VisionStatus retStatus = PR_CombineImg(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		for each (cv::Mat img in stRpy.vecResultImages)
		{
			matOutputImages.push_back(img);
		}
	}
	else
	{
		System->setTrackInfo(QString("Error at PR_Merge3DHeight, error code = %1").arg((int)retStatus));
	}

	return true;
}

bool VisionCtrl::matchAlignment(cv::Mat& matDisplay, QVector<QProfileObj*>& objProfTests)
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return false;

	cv::Point2d alignOffset;

	QBoardObj* pBoardObj = pData->getBoardObj();
	if (pBoardObj)
	{
		int nAlignNum = pBoardObj->getBoardAlignNum();
		if (nAlignNum > 0)
		{
			cv::Point2d alignRect;	
			alignRect.x = 0;
			alignRect.y = 0;
			int nCalcNum = 0;
			for (int i = 0; i < nAlignNum; i++)
			{
				int nRecordID = pBoardObj->getRecordID(i);
				cv::RotatedRect rect = pBoardObj->getBoardAlign(i);

				cv::RotatedRect rtRect = rect;
				rtRect.size.width *= 2;
				rtRect.size.height *= 2;

				cv::Rect searchRect = rtRect.boundingRect();
				int nImgWidth = matDisplay.cols;
				int nImgHeight = matDisplay.rows;
				if (searchRect.x < 0) searchRect.x = 0;
				if (searchRect.y < 0) searchRect.y = 0;
				if (searchRect.x + searchRect.width > nImgWidth) searchRect.width = nImgWidth - searchRect.x;
				if (searchRect.y + searchRect.height > nImgHeight) searchRect.height = nImgHeight - searchRect.y;

				bool bTmpMatchSubPixel = System->getParam("srh_tmp_sub_pixel").toBool();

				Vision::PR_MATCH_TEMPLATE_CMD  stCmd;
				stCmd.matInputImg = matDisplay;
				stCmd.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;
				stCmd.nRecordId = nRecordID;
				stCmd.rectSrchWindow = searchRect;
				stCmd.bSubPixelRefine = bTmpMatchSubPixel;

				int nTmpMatchObjMotion = System->getParam("srh_tmp_obj_motion").toInt();
				stCmd.enMotion = (Vision::PR_OBJECT_MOTION)nTmpMatchObjMotion;

				Vision::PR_MATCH_TEMPLATE_RPY stRpy;
				Vision::VisionStatus retStatus = Vision::PR_MatchTmpl(&stCmd, &stRpy);
				if (retStatus == Vision::VisionStatus::OK)
				{
					alignRect.x += stRpy.ptObjPos.x - rect.center.x;
					alignRect.y += stRpy.ptObjPos.y - rect.center.y;
					nCalcNum++;
				}
				else
				{
					System->setTrackInfo(QString("Error at Template Match, error code = %1").arg((int)retStatus));
				}
			}

			if (nCalcNum > 0)
			{
				alignOffset.x = alignRect.x / nCalcNum;
				alignOffset.y = alignRect.y / nCalcNum;
			}
			else
			{
				alignOffset.x = 0;
				alignOffset.y = 0;
			}			
		}
		else
		{
			alignOffset.x = 0;
			alignOffset.y = 0;			
		}
	}

	qDebug() << "Board Offset: x = " << alignOffset.x << " , y = " << alignOffset.y;

	for (int i = 0; i < pData->getProfObjNum(); i++)
	{
		QProfileObj* pObj = pData->getProfObj(i);
		if (pObj)
		{
			QProfileObj* pTestObj = new QProfileObj(*pObj);

			pTestObj->setOffset(alignOffset);

			objProfTests.push_back(pTestObj);
		}
	}

	return true;
}

bool VisionCtrl::calculateDetectProfile(cv::Mat& matHeight, QVector<QProfileObj*>& objProfTests)
{
	for (int i = 0; i < objProfTests.size(); i++)
	{
		QProfileObj* pObj = objProfTests.at(i);
		if (pObj)
		{
			double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
			pObj->getCutingPosn(x1, y1, x2, y2);

			QVector<cv::Point2d> profDatas;
			cutingUsingBilinearInterpolation(matHeight, x1, y1, x2, y2, profDatas);
			pObj->setProfData(profDatas);

			for (int j = 0; j < pObj->getMeasObjNum(); j++)
			{
				QProfMeasureObj*  pMeasObj = pObj->getMeasObj(j);
				if (pMeasObj)
				{					
					if (!calculateDetectProfileMeas(pObj, pMeasObj))
					{
						continue;
					}
				}
			}
		}
	}

	return true;
}

bool VisionCtrl::calculateDetectProfileMeas(QProfileObj* pObj, QProfMeasureObj*  pMeasObj)
{
	if (pMeasObj)
	{
		switch (pMeasObj->getType())
		{
		case MEASURE_TYPE_LNTOLNANGLE:
		{
			if (pMeasObj->getRangeNum() != 2) return false;
			cv::Point ptLine1 = pMeasObj->getRange(0);
			cv::Point ptLine2 = pMeasObj->getRange(1);

			Vision::PR_Line2f stLine1;
			if (!fitLine(pObj->getProfData(), ptLine1.x, ptLine1.y, stLine1))
			{
				return false;
			}

			Vision::PR_Line2f stLine2;
			if (!fitLine(pObj->getProfData(), ptLine2.x, ptLine2.y, stLine2))
			{
				return false;
			}

			if (!calcTwoLineAngle(stLine1, stLine2, dynamic_cast<QProfMeasLnToLnAngle*>(pMeasObj)))
			{
				return false;
			}
		}
		break;
		case MEASURE_TYPE_LNTOLN_DIST:
		{
			if (pMeasObj->getRangeNum() != 2) return false;
			cv::Point ptLine1 = pMeasObj->getRange(0);
			cv::Point ptLine2 = pMeasObj->getRange(1);

			Vision::PR_Line2f stLine1;
			if (!fitLine(pObj->getProfData(), ptLine1.x, ptLine1.y, stLine1))
			{
				return false;
			}

			Vision::PR_Line2f stLine2;
			if (!fitLine(pObj->getProfData(), ptLine2.x, ptLine2.y, stLine2))
			{
				return false;
			}

			if (!calcTwoLineDist(stLine1, stLine2, dynamic_cast<QProfMeasLnToLnDist*>(pMeasObj)))
			{
				return false;
			}
		}
		break;
		case MEASURE_TYPE_PTTOLN_DIST:
		{
			if (pMeasObj->getRangeNum() != 2) return false;
			cv::Point ptLine1 = pMeasObj->getRange(0);
			cv::Point ptLine2 = pMeasObj->getRange(1);

			bool bReversedFit = false;  double fSlope = 0, fIntercept = 0;
			if (!fitLine(pObj->getProfData(), ptLine2.x, ptLine2.y, bReversedFit, fSlope, fIntercept))
			{
				return false;
			}

			cv::Point2d ptOrg = pObj->getProfData()[ptLine1.x];
			if (!calcPtToLineDist(ptOrg, bReversedFit, fSlope, fIntercept, pMeasObj))
			{
				return false;
			}
		}
		break;
		case MEASURE_TYPE_PTTOPT_DIST:
		{
			if (pMeasObj->getRangeNum() != 2) return false;
			cv::Point ptLine1 = pMeasObj->getRange(0);
			cv::Point ptLine2 = pMeasObj->getRange(1);		

			cv::Point2d pt1 = pObj->getProfData()[ptLine1.x];
			cv::Point2d pt2 = pObj->getProfData()[ptLine2.x];
			if (!calcPtToPtDist(pt1, pt2, pMeasObj))
			{
				return false;
			}
		}
		break;
		case MEASURE_TYPE_PT_HEIGHT:
		{
			if (pMeasObj->getRangeNum() != 1) return false;
			cv::Point ptLine1 = pMeasObj->getRange(0);		

			cv::Point2d pt1 = pObj->getProfData()[ptLine1.x];	
			QProfMeasPtHeight* pObjTmp = dynamic_cast<QProfMeasPtHeight*>(pMeasObj);
			if (pObjTmp)
			{
				pObjTmp->_dHeight = pt1.y;
			}
		
		}
		break;
		case MEASURE_TYPE_CRTOCR_DIST:
		{
			if (pMeasObj->getRangeNum() != 2) return false;
			cv::Point ptLine1 = pMeasObj->getRange(0);
			cv::Point ptLine2 = pMeasObj->getRange(1);

			cv::Point2f ptCirCenter1; double dRadius1 = 0;
			if (!fitCircle(pObj->getProfData(), ptLine1.x, ptLine1.y, ptCirCenter1, dRadius1))
			{
				return false;
			}

			cv::Point2f ptCirCenter2; double dRadius2 = 0;
			if (!fitCircle(pObj->getProfData(), ptLine2.x, ptLine2.y, ptCirCenter2, dRadius2))
			{
				return false;
			}

			cv::Point2d pt1 = ptCirCenter1;
			cv::Point2d pt2 = ptCirCenter2;
			if (!calcPtToPtDist(pt1, pt2, pMeasObj))
			{
				return false;
			}

			QProfMeasCirToCir* pObjTmp = dynamic_cast<QProfMeasCirToCir*>(pMeasObj);
			if (pObjTmp)
			{
				pObjTmp->_dRadius1 = dRadius1;
				pObjTmp->_ptCirCenter1 = ptCirCenter1;

				pObjTmp->_dRadius2 = dRadius2;
				pObjTmp->_ptCirCenter2 = ptCirCenter2;
			}
		}
		break;
		case MEASURE_TYPE_CRTOLN_DIST:
		{
			if (pMeasObj->getRangeNum() != 2) return false;
			cv::Point ptLine1 = pMeasObj->getRange(0);
			cv::Point ptLine2 = pMeasObj->getRange(1);

			cv::Point2f ptCirCenter1; double dRadius1 = 0;
			if (!fitCircle(pObj->getProfData(), ptLine1.x, ptLine1.y, ptCirCenter1, dRadius1))
			{
				return false;
			}

			bool bReversedFit = false;  double fSlope = 0, fIntercept = 0;
			if (!fitLine(pObj->getProfData(), ptLine2.x, ptLine2.y, bReversedFit, fSlope, fIntercept))
			{
				return false;
			}

			cv::Point2d ptOrg = ptCirCenter1;
			if (!calcPtToLineDist(ptOrg, bReversedFit, fSlope, fIntercept, pMeasObj))
			{
				return false;
			}

			QProfMeasCirToLn* pObjTmp = dynamic_cast<QProfMeasCirToLn*>(pMeasObj);
			if (pObjTmp)
			{
				pObjTmp->_dRadius = dRadius1;
				pObjTmp->_ptCirCenter = ptCirCenter1;				
			}
		}
		break;
		case MEASURE_TYPE_CRTOPT_DIST:
		{
			if (pMeasObj->getRangeNum() != 2) return false;
			cv::Point ptLine1 = pMeasObj->getRange(0);
			cv::Point ptLine2 = pMeasObj->getRange(1);

			cv::Point2f ptCirCenter1; double dRadius1 = 0;
			if (!fitCircle(pObj->getProfData(), ptLine1.x, ptLine1.y, ptCirCenter1, dRadius1))
			{
				return false;
			}			

			cv::Point2d pt1 = ptCirCenter1;
			cv::Point2d pt2 = pObj->getProfData()[ptLine2.x];
			if (!calcPtToPtDist(pt1, pt2, pMeasObj))
			{
				return false;
			}

			QProfMeasCirToPt* pObjTmp = dynamic_cast<QProfMeasCirToPt*>(pMeasObj);
			if (pObjTmp)
			{
				pObjTmp->_dRadius = dRadius1;
				pObjTmp->_ptCirCenter = ptCirCenter1;
			}
		}
		break;
		case MEASURE_TYPE_CR_RADIUS:
		{
			if (pMeasObj->getRangeNum() != 1) return false;
			cv::Point ptLine1 = pMeasObj->getRange(0);

			cv::Point2f ptCirCenter; double dRadius = 0;
			if (!fitCircle(pObj->getProfData(), ptLine1.x, ptLine1.y, ptCirCenter, dRadius))
			{
				return false;
			}

			QProfMeasCirRadius* pObjTmp = dynamic_cast<QProfMeasCirRadius*>(pMeasObj);
			if (pObjTmp)
			{
				pObjTmp->_dRadius = dRadius;
				pObjTmp->_ptCirCenter = ptCirCenter;
			}
		}
		break;
		case MEASURE_TYPE_AREA:
		{
			if (pMeasObj->getRangeNum() != 1) return false;
			cv::Point ptLine1 = pMeasObj->getRange(0);

			double dArea = 0;
			if (!CrossSectionArea(pObj->getProfData(), ptLine1.x, ptLine1.y, dArea))
			{
				return false;
			}

			QProfMeasProfArea* pObjTmp = dynamic_cast<QProfMeasProfArea*>(pMeasObj);
			if (pObjTmp)
			{
				pObjTmp->setValue(dArea);
			}
		}
		break;
		case MEASURE_TYPE_PRF_LENGTH:
		{
			if (pMeasObj->getRangeNum() != 1) return false;
			cv::Point ptLine1 = pMeasObj->getRange(0);

			double dLength = 0;
			if (!CrossSectionProfLength(pObj->getProfData(), ptLine1.x, ptLine1.y, dLength))
			{
				return false;
			}

			QProfMeasProfLength* pObjTmp = dynamic_cast<QProfMeasProfLength*>(pMeasObj);
			if (pObjTmp)
			{
				pObjTmp->setValue(dLength);
			}
		}
		break;
		case MEASURE_TYPE_NULL:
		default:
			break;
		}
	}
	return true;
}

bool VisionCtrl::calcPtToPtDist(cv::Point2d& pt1, cv::Point2d& pt2, QProfMeasureObj* pProfObj)
{
	double fDistance = qSqrt(qPow(pt1.x - pt2.x, 2) + qPow(pt1.y - pt2.y, 2));
	pProfObj->setValue(fDistance);
	return true;
}

bool VisionCtrl::calcPtToLineDist(cv::Point2d& ptOrg, bool bReversedFit, double fSlope, double fIntercept, QProfMeasureObj* pProfObj)
{
	Vision::PR_POINT_LINE_DISTANCE_CMD  stCmd;
	stCmd.ptInput = ptOrg;	
	stCmd.bReversedFit = bReversedFit;
	stCmd.fSlope = fSlope;
	stCmd.fIntercept = fIntercept;

	Vision::PR_POINT_LINE_DISTANCE_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_PointLineDistance(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		//pProfObj->_line1Pt1 = stLine1.pt1;
		//pProfObj->_line1Pt2 = stLine1.pt2;
		//pProfObj->_line2Pt1 = stLine2.pt1;
		//pProfObj->_line2Pt2 = stLine2.pt2;

		pProfObj->setValue(stRpy.fDistance);

		return true;
	}
	else
	{
		System->setTrackInfo(QString("Error at calcPtToLineDist, error code = %1").arg((int)retStatus));
	}

	return false;
}

bool VisionCtrl::calcTwoLineDist(Vision::PR_Line2f& stLine1, Vision::PR_Line2f& stLine2, QProfMeasLnToLnDist* pProfObj)
{
	Vision::PR_PARALLEL_LINE_DIST_CMD  stCmd;
	stCmd.line1 = stLine1;
	stCmd.line2 = stLine2;

	Vision::PR_PARALLEL_LINE_DIST_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_ParallelLineDist(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		//pProfObj->_line1Pt1 = stLine1.pt1;
		//pProfObj->_line1Pt2 = stLine1.pt2;
		//pProfObj->_line2Pt1 = stLine2.pt1;
		//pProfObj->_line2Pt2 = stLine2.pt2;

		pProfObj->_dDist = stRpy.fDistance;

		return true;
	}
	else
	{
		System->setTrackInfo(QString("Error at calcTwoLineDist, error code = %1").arg((int)retStatus));
	}

	return false;
}

bool VisionCtrl::calcTwoLineAngle(Vision::PR_Line2f& stLine1, Vision::PR_Line2f& stLine2, QProfMeasLnToLnAngle* pProfObj)
{
	Vision::PR_TWO_LINE_ANGLE_CMD  stCmd;
	stCmd.line1 = stLine1;
	stCmd.line2 = stLine2;

	Vision::PR_TWO_LINE_ANGLE_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_TwoLineAngle(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{		
		//pProfObj->_line1Pt1 = stLine1.pt1;
		//pProfObj->_line1Pt2 = stLine1.pt2;
		//pProfObj->_line2Pt1 = stLine2.pt1;
		//pProfObj->_line2Pt2 = stLine2.pt2;
	
		pProfObj->_dAngle = stRpy.fAngle;	

		return true;
	}
	else
	{
		System->setTrackInfo(QString("Error at PR_TwoLineAngle, error code = %1").arg((int)retStatus));
	}

	return false;
}

bool VisionCtrl::fitLine(QVector<cv::Point2d>& profDatas, int nProfDataIndex1, int nProfDataIndex2, Vision::PR_Line2f& stLine)
{
	Vision::PR_FIT_LINE_BY_POINT_CMD  stCmd;
	for (int i = 0 + nProfDataIndex1; (i < nProfDataIndex2) && (i < profDatas.size()); i++)
	{
		stCmd.vecPoints.push_back(profDatas.at(i));
	}
	stCmd.enMethod = Vision::PR_FIT_METHOD::LEAST_SQUARE_REFINE;
	stCmd.bPreprocessed = false;
	stCmd.nThreshold = 20;
	stCmd.enAttribute = Vision::PR_OBJECT_ATTRIBUTE::DARK;
	stCmd.enRmNoiseMethod = Vision::PR_RM_FIT_NOISE_METHOD::ABSOLUTE_ERR;
	stCmd.fErrTol = 10;

	Vision::PR_FIT_LINE_BY_POINT_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_FitLineByPoint(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		stLine = stRpy.stLine;
		return true;
	}
	else
	{
		System->setTrackInfo(QString("Error at fitLine, error code = %1").arg((int)retStatus));
	}
	return false;
}

bool VisionCtrl::fitLine(QVector<cv::Point2d>& profDatas, int nProfDataIndex1, int nProfDataIndex2, bool& bReversedFit, double& fSlope, double& fIntercept)
{
	Vision::PR_FIT_LINE_BY_POINT_CMD  stCmd;
	for (int i = 0 + nProfDataIndex1; (i < nProfDataIndex2) && (i < profDatas.size()); i++)
	{
		stCmd.vecPoints.push_back(profDatas.at(i));
	}
	stCmd.enMethod = Vision::PR_FIT_METHOD::LEAST_SQUARE_REFINE;
	stCmd.bPreprocessed = false;
	stCmd.nThreshold = 20;
	stCmd.enAttribute = Vision::PR_OBJECT_ATTRIBUTE::DARK;
	stCmd.enRmNoiseMethod = Vision::PR_RM_FIT_NOISE_METHOD::ABSOLUTE_ERR;
	stCmd.fErrTol = 10;

	Vision::PR_FIT_LINE_BY_POINT_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_FitLineByPoint(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		bReversedFit = stRpy.bReversedFit;
		fSlope = stRpy.fSlope;
		fIntercept = stRpy.fIntercept;
		return true;
	}
	else
	{
		System->setTrackInfo(QString("Error at fitLine, error code = %1").arg((int)retStatus));
	}
	return false;
}

bool VisionCtrl::fitCircle(QVector<cv::Point2d>& profDatas, int nProfDataIndex1, int nProfDataIndex2, cv::Point2f& ptCirCenter, double& dRadius)
{
	Vision::PR_FIT_CIRCLE_BY_POINT_CMD  stCmd;
	for (int i = 0 + nProfDataIndex1; (i < nProfDataIndex2) && (i < profDatas.size()); i++)
	{
		stCmd.vecPoints.push_back(profDatas.at(i));
	}
	stCmd.enMethod = Vision::PR_FIT_METHOD::LEAST_SQUARE_REFINE;
	stCmd.bPreprocessed = false;
	stCmd.nThreshold = 20;
	stCmd.enAttribute = Vision::PR_OBJECT_ATTRIBUTE::DARK;
	stCmd.enRmNoiseMethod = Vision::PR_RM_FIT_NOISE_METHOD::ABSOLUTE_ERR;
	stCmd.fErrTol = 10;

	Vision::PR_FIT_CIRCLE_BY_POINT_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_FitCircleByPoint(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		ptCirCenter = stRpy.ptCircleCtr;
		dRadius = stRpy.fRadius;		
		return true;
	}
	else
	{
		System->setTrackInfo(QString("Error at PR_FitCircleByPoint, error code = %1").arg((int)retStatus));
	}
	return false;
}

bool VisionCtrl::CrossSectionArea(QVector<cv::Point2d>& profDatas, int nProfDataIndex1, int nProfDataIndex2, double& dArea)
{
	Vision::PR_CROSS_SECTION_AREA_CMD  stCmd;

	cv::Point2f pt1;
	pt1.x = profDatas.at(nProfDataIndex1).x;
	pt1.y = 0;
	stCmd.vecContourPoints.push_back(pt1);
	for (int i = 0 + nProfDataIndex1; (i < nProfDataIndex2) && (i < profDatas.size()); i++)
	{
		stCmd.vecContourPoints.push_back(profDatas.at(i));
	}
	cv::Point2f pt2;
	pt2.x = profDatas.at(nProfDataIndex2).x;
	pt2.y = 0;
	stCmd.vecContourPoints.push_back(pt2);

	stCmd.bClosed = true;

	Vision::PR_CROSS_SECTION_AREA_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_CrossSectionArea(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{
		dArea = stRpy.fArea;		
		return true;
	}
	else
	{
		System->setTrackInfo(QString("Error at PR_CrossSectionArea, error code = %1").arg((int)retStatus));
	}
	return false;
}

bool VisionCtrl::CrossSectionProfLength(QVector<cv::Point2d>& profDatas, int nProfDataIndex1, int nProfDataIndex2, double& dLength)
{
	std::vector<cv::Point2f> vecPoints;
	for (int i = 0 + nProfDataIndex1; (i < nProfDataIndex2) && (i < profDatas.size()); i++)
	{
		vecPoints.push_back(profDatas.at(i));
	}

	dLength = cv::arcLength(vecPoints, false);
	
	return true;
}

void VisionCtrl::cutingUsingBilinearInterpolation(cv::Mat& matHeight, double posX1, double posY1, double posX2, double posY2, QVector<cv::Point2d>& profDatas)
{
	// assume the value is same to index 
	double dExtlen = 0;
	double posX[2] = { posX1, posX2 };
	double posY[2] = { posY1, posY2 };
	double posExtX[2] = { 0 };
	double posExtY[2] = { 0 };

	//getLineLimitPos(posX, posY, dExtlen, posExtX, posExtY);
	posExtX[0] = posX1; posExtX[1] = posX2;
	posExtY[0] = posY1; posExtY[1] = posY2;

	int nSizeX = matHeight.cols;
	int nSizeY = matHeight.rows;	

	int nSampLing = 1;
	int nLimitX = nSizeX * nSampLing;
	int nLimitY = nSizeY * nSampLing;

	double x1 = posExtX[0];
	double x2 = posExtX[1];
	double y1 = posExtY[0];
	double y2 = posExtY[1];

	if (x1 < 0) x1 = 0; if (x1 >= nLimitX) x1 = nLimitX - 1;
	if (y1 < 0) y1 = 0; if (y1 >= nLimitY) y1 = nLimitY - 1;
	if (x2 < 0) x2 = 0; if (x2 >= nLimitX) x2 = nLimitX - 1;
	if (y2 < 0) y2 = 0; if (y2 >= nLimitY) y2 = nLimitY - 1;

	double dDist = qSqrt(qPow(x1 - x2, 2) + qPow(y1 - y2, 2));
	dDist = qCeil(dDist);

	int nNum = qCeil(dDist); // x = 1,2,...m / y = 1,2,...n	

	double* xValues = new double[nNum];
	double* yValues = new double[nNum];
	double* fxs = new double[nNum];
	double* fys = new double[nNum];

	for (int i = 0; i < nNum; i++)
	{
		xValues[i] = x1 + (x2 - x1)* i / dDist;
		yValues[i] = y1 + (y2 - y1)* i / dDist;

		//xValues[i] += (m_dMeshSizeMaxX - m_dMeshSizeMinX) / 2 - 0;//80
		//yValues[i] += (m_dMeshSizeMaxY - m_dMeshSizeMinY) / 2 + 0;//80

		xValues[i] = xValues[i] / nSampLing + 1;
		yValues[i] = yValues[i] / nSampLing + 1;

		fxs[i] = xValues[i] - qFloor(xValues[i]);
		fys[i] = yValues[i] - qFloor(yValues[i]);
		xValues[i] = qFloor(xValues[i]);
		yValues[i] = qFloor(yValues[i]);		
	}

	//QVector<int> indicesMesh;
	//int nNumTriangles = (nSizeX - 1) * (nSizeY - 1);
	//for (int i = 0; i < nNumTriangles; i++)
	//{
	//	int colIndex = i % (nSizeX - 1);
	//	int rowIndex = i / (nSizeY - 1);

	//	int nTriIndex1 = nSizeX*rowIndex + colIndex;
	//	int nTriIndex2 = nSizeX*rowIndex + colIndex + 1;
	//	int nTriIndex3 = nSizeX*(rowIndex + 1) + colIndex;

	//	//if ((m_verticesMesh->GetVector(nTriIndex1).z != 0)
	//	//	&& (m_verticesMesh->GetVector(nTriIndex2).z != 0)
	//	//	&& (m_verticesMesh->GetVector(nTriIndex3).z != 0))
	//	{
	//		indicesMesh.push_back(nTriIndex1);
	//		indicesMesh.push_back(nTriIndex2);
	//		indicesMesh.push_back(nTriIndex3);
	//	}

	//	nTriIndex1 = nSizeX*rowIndex + colIndex + 1;
	//	nTriIndex2 = nSizeX*(rowIndex + 1) + colIndex;
	//	nTriIndex3 = nSizeX*(rowIndex + 1) + colIndex + 1;

	//	//if ((m_verticesMesh->GetVector(nTriIndex1).z != 0)
	//	//	&& (m_verticesMesh->GetVector(nTriIndex2).z != 0)
	//	//	&& (m_verticesMesh->GetVector(nTriIndex3).z != 0))
	//	{
	//		indicesMesh.push_back(nTriIndex1);
	//		indicesMesh.push_back(nTriIndex2);
	//		indicesMesh.push_back(nTriIndex3);
	//	}
	//}

	int nDataCount = nSizeX * nSizeY;
	double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	for (int i = 0; i < nNum - 1; i++)
	{
		int xValue = xValues[i];// X,Y 值互换
		int yValue = yValues[i];

		int index1 = (xValue - 1)*(nSizeY)+yValue - 1;		
		int index2 = (xValue - 1)*(nSizeY)+yValue - 1 + 1;
		int index3 = (xValue - 1 + 1)*(nSizeY)+yValue - 1;
		int index4 = (xValue - 1 + 1)*(nSizeY)+yValue - 1 + 1;

		//cv::Mat_<float>::iterator it = matHeight.begin<float>();
		//cv::Mat_<float>::iterator itend = matHeight.end<float>();

		double z1 = 0, z2 = 0, z3 = 0, z4 = 0;

		if (index1 < 0 || index1 >= nDataCount) z1 = 0;
		else z1 = /*(it + index1);*/matHeight.at<float>(index1%nSizeY, index1 / nSizeY);

		if (index2 < 0 || index2 >= nDataCount) z2 = 0;
		else z2 = /*(it + index2);*/matHeight.at<float>(index2%nSizeY, index2 / nSizeY);

		if (index3 < 0 || index3 >= nDataCount) z3 = 0;
		else z3 = /*(it + index3);*/ matHeight.at<float>(index3%nSizeY, index3 / nSizeY);

		if (index4 < 0 || index4 >= nDataCount) z4 = 0;
		else z4 = /*(it + index4);*/ matHeight.at<float>(index4%nSizeY, index4 / nSizeY);

		//double z1 = matHeight.at<double>(index1);
		//double z2 = matHeight.at<double>(index2);
		//double z3 = matHeight.at<double>(index3);
		//double z4 = matHeight.at<double>(index4);

		double zValue = z1*(1 - fxs[i])*(1 - fys[i]) + z2*(1 - fxs[i])*fys[i]
			+ z3*fxs[i] * (1 - fys[i]) + z4*fxs[i] * fys[i];

		//zValue = zValue * dResolutionX / 1000;

		double dValueX = i * dResolutionX / 1000;

		profDatas.push_back(cv::Point2d(dValueX, zValue));
	}	

	delete[] xValues;
	delete[] yValues;
	delete[] fxs;
	delete[] fys;
}

cv::Mat VisionCtrl::drawHeightGray(const cv::Mat &matHeight)
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

void VisionCtrl::addImageText(cv::Mat image, cv::Point ptPos, QString szText)
{
	double dScaleFactor = 1.0;


	cv::String text = szText.toStdString();

	Point ptPos1;
	ptPos1.x = ptPos.x - 10 * dScaleFactor;
	ptPos1.y = ptPos.y + 10 * dScaleFactor;
	Point ptPos2;
	ptPos2.x = ptPos.x + 10 * dScaleFactor * text.length();
	ptPos2.y = ptPos.y - 20 * dScaleFactor;
	rectangle(image, ptPos1, ptPos2, Scalar(125, 125, 125), -1);
	rectangle(image, ptPos1, ptPos2, Scalar(200, 200, 200), 1);

	double fontScale = dScaleFactor*0.5f;
	cv::putText(image, text, ptPos, CV_FONT_HERSHEY_COMPLEX, fontScale, Scalar(0, 255, 255), 1);
}

