#include <QDir>
#include <QThread>
#include <QThreadPool>
#include <QDateTime>
#include <QMessageBox>
#include <time.h>

#include "opencv2/highgui.hpp"
#include "opencv2/video.hpp"

#include "ScanImageThread.h"
#include "../Common/eos.h"
#include "../Common/ModuleMgr.h"
#include "../Common/SystemData.h"
#include "../include/IMotion.h"
#include "../include/IData.h"
#include "../include/IDlp.h"
#include "../include/ICamera.h"
#include "../include/IVision.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../include/constants.h"
#include "../DataModule/DataUtils.h"
#include "../HSGWorkflowCtrl/Calc3DHeightRunnable.h"
#include "Merge3DHeightRunnable.h"

ScanImageThread::ScanImageThread(
    const Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr,
    float                                  fOverlapUmX,
    float                                  fOverlapUmY,
    Vision::PR_SCAN_IMAGE_DIR              enScanDir) :
    m_vecVecFrameCtr (vecVecFrameCtr),
    m_fOverlapUmX    (fOverlapUmX),
    m_fOverlapUmY    (fOverlapUmY),
    m_enScanDir      (enScanDir),
    m_exit           (false)
{
}

ScanImageThread::~ScanImageThread() {
}

void ScanImageThread::quit()
{
	m_exit = true;
}

bool ScanImageThread::preRunning()
{
    m_dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    m_dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    m_nDLPCount = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
	return true;
}

void ScanImageThread::run()
{
	if (! preRunning()) return;

	double dtime_start = 0, dtime_movePos = 0;
    if (m_vecVecFrameCtr.empty())
        return;

    int ROWS = m_vecVecFrameCtr.size();
    int COLS = m_vecVecFrameCtr[0].size();
    int TOTAL = ROWS * COLS;
    Vision::VectorOfVectorOfMat vecVecFrameImages(PROCESSED_IMAGE_SEQUENCE::TOTAL_COUNT, Vision::VectorOfMat(TOTAL, cv::Mat()));
	Vision::VectorOfMat vecFrame3DHeight(TOTAL, cv::Mat());
    
    m_bGood = true;
    for (int row = 0; row < ROWS; ++ row) {
        for (int col = 0; col < COLS; ++ col) {

            System->setTrackInfo(QStringLiteral("正在扫描行 (%1) 列 (%2)").arg(row).arg(col));

            auto ptFrameCtr = m_vecVecFrameCtr[row][col];
            if (! moveToCapturePos(ptFrameCtr.x, ptFrameCtr.y)) {
                m_bGood = false;
                break;
            }

            QVector<cv::Mat> vecMatImages;
            if (! captureAllImages(vecMatImages)) {
                m_bGood = false;
                break;
            }

            QThreadPool::globalInstance()->waitForDone();

            std::vector<Calc3DHeightRunnablePtr> vecCalc3dHeightRunnable;
            for (int nDlpId = 0; nDlpId < m_nDLPCount; ++ nDlpId) {
                QVector<cv::Mat> vecDlpImage = vecMatImages.mid(nDlpId * DLP_IMG_COUNT, DLP_IMG_COUNT);
                auto pCalc3DHeightRunnable = std::make_shared<Calc3DHeightRunnable>(nDlpId + 1, vecDlpImage);
                pCalc3DHeightRunnable->setAutoDelete(false);
                m_threadPoolCalc3DHeight.start(pCalc3DHeightRunnable.get());
                vecCalc3dHeightRunnable.push_back(std::move(pCalc3DHeightRunnable));
            }

            Vision::VectorOfMat vec2DCaptureImages(vecMatImages.begin() + m_nDLPCount * DLP_IMG_COUNT, vecMatImages.end());
            if (vec2DCaptureImages.size() != CAPTURE_2D_IMAGE_SEQUENCE::TOTAL_COUNT) {
                System->setTrackInfo(QString(QStringLiteral("2D image count %0 not correct.")).arg(vec2DCaptureImages.size()));
                m_bGood = false;
                break;
            }

            auto vec2DImage = _generate2DImages(vec2DCaptureImages);
            for (size_t i = 0; i < vec2DImage.size(); ++ i)
                vecVecFrameImages[i][row * COLS + col] = vec2DImage[i];

            Merge3DHeightRunnable *pMerge3DHeightRunnable = new Merge3DHeightRunnable(&m_threadPoolCalc3DHeight, vecCalc3dHeightRunnable,
                &vecFrame3DHeight, row, col, ROWS, COLS);
            QThreadPool::globalInstance()->start(pMerge3DHeightRunnable);
        }

        if (! m_bGood)
            break;
    }

    QThreadPool::globalInstance()->waitForDone();

    Vision::PR_COMBINE_IMG_CMD stCmd;
    Vision::PR_COMBINE_IMG_RPY stRpy;
    stCmd.nCountOfImgPerFrame = 1;
    stCmd.nCountOfFrameX = m_vecVecFrameCtr[0].size();
    stCmd.nCountOfFrameY = m_vecVecFrameCtr.size();
    stCmd.nOverlapX = m_fOverlapUmX / m_dResolutionX;
    stCmd.nOverlapY = m_fOverlapUmY / m_dResolutionY;
    stCmd.nCountOfImgPerRow = m_vecVecFrameCtr[0].size();
    stCmd.enScanDir = m_enScanDir;

    for (int i = 0; i < 4; ++ i) {        
        stCmd.vecInputImages = vecVecFrameImages[i];
        if ( Vision::VisionStatus::OK == Vision::PR_CombineImg(&stCmd, &stRpy))
            m_vecCombinedBigImages.push_back(stRpy.vecResultImages[0]);
        else {
            System->setTrackInfo(QString(QStringLiteral("合并大图失败.")));
            m_bGood = false;
        }
    }

    stCmd.vecInputImages = vecFrame3DHeight;
    if (Vision::VisionStatus::OK == Vision::PR_CombineImg(&stCmd, &stRpy)) {
        m_matCombinedBigHeight = stRpy.vecResultImages[0];

        double dMinValue = 0, dMaxValue = 0;
        cv::Mat matMask = (m_matCombinedBigHeight == m_matCombinedBigHeight);
        cv::minMaxIdx(m_matCombinedBigHeight, &dMinValue, &dMaxValue, 0, 0, matMask);

        cv::Mat matNewPhase = m_matCombinedBigHeight - dMinValue;

        float dRatio = 255.f / ToFloat(dMaxValue - dMinValue);
		matNewPhase = matNewPhase * dRatio;

        cv::Mat matHeightGrayImg;
        matNewPhase.convertTo(matHeightGrayImg, CV_8UC1);
        m_vecCombinedBigImages.push_back(matHeightGrayImg);
    }else {
        System->setTrackInfo(QString(QStringLiteral("合并大图失败.")));
        m_bGood = false;
    }

	System->setTrackInfo(QString(QStringLiteral("扫描图片完成")));
}

bool ScanImageThread::moveToCapturePos(float fPosX, float fPosY)
{
    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return false;

    fPosX *= UM_TO_MM;
    fPosY *= UM_TO_MM;

    if (! pMotion->moveToGroup(std::vector<int>({AXIS_MOTOR_X, AXIS_MOTOR_Y}), std::vector<double>({fPosX, fPosY}), std::vector<int>({0, 0}), true))
    {
        System->setTrackInfo(QString("move to position error"));
		return false;
    }
    return true;
}

bool ScanImageThread::captureAllImages(QVector<cv::Mat>& imageMats)
{
    if (System->isRunOffline()) {
        imageMats.clear();
        std::string strImagePath("D:/Data/20180203_TestImageOnKB/0203125013/");
        char strfileName[100];
        for (int i = 1; i <= 54; ++ i) {
            _snprintf(strfileName, sizeof(strfileName), "%02d.bmp", i);
            cv::Mat matImage = cv::imread(strImagePath + strfileName, cv::IMREAD_GRAYSCALE);
            imageMats.push_back(matImage);
        }
        return true;
    }

	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return false;

	return pCam->captureAllImages(imageMats);
}

bool ScanImageThread::mergeImages(QString& szImagePath)
{
	return true;
}

bool ScanImageThread::isExit()
{
	return m_exit;
}

QString ScanImageThread::generateImagePath()
{
	QString capturePath = System->getParam("camera_cap_image_path").toString();

	QDateTime dtm = QDateTime::currentDateTime();
	QString fileDir = capturePath + "/image/" + dtm.toString("MMddhhmmss") + "/";
	QDir dir; dir.mkdir(fileDir);

	return fileDir;
}

void ScanImageThread::saveImages(const QString& szImagePath, int nRowIndex, int nColIndex, int nCountOfImgPerRow, const QVector<cv::Mat>& imageMats)
{
	int nCountOfImgPerFrame = imageMats.size();
	for (int i = 0; i < nCountOfImgPerFrame; i++)
	{
		int nImageIndex = nRowIndex * nCountOfImgPerRow + nColIndex*nCountOfImgPerFrame + i + 1;

		QString strSave = szImagePath + QString("F") + QString::number(nRowIndex + 1, 'g', 2) + QString("-") + QString::number(nImageIndex, 'g', 2) + QString("-") +
			QString("1") + QString(".bmp");

		IplImage frameImg = IplImage(imageMats[i]);
		cvSaveImage(strSave.toStdString().c_str(), &frameImg);
	}	
}

void ScanImageThread::saveCombineImages(const QString& szImagePath, const QVector<cv::Mat>& imageMats)
{
	for (int i = 0; i < imageMats.size(); i++)
	{
		QString strSave = szImagePath + QString("CombineResult_") + QString::number(i + 1, 'g', 2) + QString(".bmp");

		IplImage frameImg = IplImage(imageMats[i]);
		cvSaveImage(strSave.toStdString().c_str(), &frameImg);
	}
}

Vision::VectorOfMat ScanImageThread::_generate2DImages(const Vision::VectorOfMat &vecInputImages)
{
    assert(vecInputImages.size() == CAPTURE_2D_IMAGE_SEQUENCE::TOTAL_COUNT);

    Vision::VectorOfMat vecResultImages;
    cv::Mat matRed, matGreen, matBlue;
    bool bColorCamera = false;
    if (bColorCamera) {
        cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::RED_LIGHT],   matRed,   CV_BayerGR2GRAY);
        cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::GREEN_LIGHT], matGreen, CV_BayerGR2GRAY);
        cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::BLUE_LIGHT],  matBlue,  CV_BayerGR2GRAY);
    }else {
        matRed   = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::RED_LIGHT];
        matGreen = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::GREEN_LIGHT];
        matBlue  = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::BLUE_LIGHT];
    }

    Vision::VectorOfMat vecChannels{matBlue, matGreen, matRed};
    cv::Mat matPseudocolorImage;
    cv::merge(vecChannels, matPseudocolorImage);

    cv::Mat matTopLightImage = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::WHITE_LIGHT];
    if (bColorCamera)
	    cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::WHITE_LIGHT], matTopLightImage, CV_BayerGR2BGR);
	vecResultImages.push_back(matTopLightImage);

    cv::Mat matLowAngleLightImage = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::LOW_ANGLE_LIGHT];
    if (bColorCamera)
	    cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::LOW_ANGLE_LIGHT], matLowAngleLightImage, CV_BayerGR2BGR);
    vecResultImages.push_back(matLowAngleLightImage);

    vecResultImages.push_back(matPseudocolorImage);

	cv::Mat matUniformLightImage = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::UNIFORM_LIGHT];
    if (bColorCamera)
	    cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::UNIFORM_LIGHT], matUniformLightImage, CV_BayerGR2BGR);
	vecResultImages.push_back(matUniformLightImage);

    return vecResultImages;
}