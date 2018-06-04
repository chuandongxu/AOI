#include <QDir>
#include <QThread>
#include <QThreadPool>
#include <QDateTime>
#include <QMessageBox>
#include <time.h>
#include <qdebug.h>

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

#define ToInt(value)                (static_cast<int>(value))

ScanImageThread::ScanImageThread(const Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr, float fLeft, float fTop, float fRight, float fBottom) :
    m_vecVecFrameCtr (vecVecFrameCtr), 
    m_fLeft(fLeft),
    m_fTop(fTop),
    m_fRight(fRight),
    m_fBottom(fBottom),
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

    if (m_nDLPCount < 4) return;

    int ROWS = m_vecVecFrameCtr.size();
    int COLS = m_vecVecFrameCtr[0].size();
    int TOTAL = ROWS * COLS;

    Vision::VectorOfVectorOfFloat vecVecFrameDLP1(ROWS, Vision::VectorOfFloat(COLS, float(0.0)));
    Vision::VectorOfVectorOfFloat vecVecFrameDLP2(ROWS, Vision::VectorOfFloat(COLS, float(0.0)));
    Vision::VectorOfVectorOfFloat vecVecFrameDLP3(ROWS, Vision::VectorOfFloat(COLS, float(0.0)));
    Vision::VectorOfVectorOfFloat vecVecFrameDLP4(ROWS, Vision::VectorOfFloat(COLS, float(0.0)));
   
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

            //Vision::VectorOfMat vec2DCaptureImages(vecMatImages.begin() + m_nDLPCount * DLP_IMG_COUNT, vecMatImages.end());
            //if (vec2DCaptureImages.size() != CAPTURE_2D_IMAGE_SEQUENCE::TOTAL_COUNT) {
            //    System->setTrackInfo(QString(QStringLiteral("2D image count %0 not correct.")).arg(vec2DCaptureImages.size()));
            //    m_bGood = false;
            //    break;
            //}

            m_threadPoolCalc3DHeight.waitForDone();

            QVector<cv::Mat> vecMatHeight;
            for (const auto &ptrCalc3DHeightRunnable : vecCalc3dHeightRunnable)
                vecMatHeight.push_back(ptrCalc3DHeightRunnable->get3DHeight());

            Vision::PR_CALC_DLP_OFFSET_CMD stCmd;
            Vision::PR_CALC_DLP_OFFSET_RPY stRpy;
            stCmd.matHeight1 = vecMatHeight[0];
            stCmd.matHeight2 = vecMatHeight[1];
            stCmd.matHeight3 = vecMatHeight[2];
            stCmd.matHeight4 = vecMatHeight[3];

            if (Vision::VisionStatus::OK == Vision::PR_CalcDlpOffset(&stCmd, &stRpy)) {

                vecVecFrameDLP1[row][col] = stRpy.fOffset1;
                vecVecFrameDLP2[row][col] = stRpy.fOffset2;
                vecVecFrameDLP3[row][col] = stRpy.fOffset3;
                vecVecFrameDLP4[row][col] = stRpy.fOffset4;
            }
            else {
                System->setTrackInfo(QString(QStringLiteral("计算DLP的OFFSET错误.")));
                m_bGood = false;
            }
         }

        if (! m_bGood)
            break;
    }

    QThreadPool::globalInstance()->waitForDone();

    Vision::PR_CALC_FRAME_VALUE_CMD stCmd;
    Vision::PR_CALC_FRAME_VALUE_RPY stRpy;
    stCmd.vecVecRefFrameCenters = m_vecVecFrameCtr;  

    int nLeft = ToInt(m_fLeft);
    int nTop = ToInt(m_fTop);
    int nRight = ToInt(m_fRight);
    int nBottom = ToInt(m_fBottom);

/*    int nTotalCount = 0;
    if ((nTop - nBottom) > (nRight - nLeft))
    {
        nTotalCount = (nTop - nBottom) + 1;
    }
    else
    {
        nTotalCount = (nRight - nLeft) + 1;
    } */ 

    m_vecVecFrameChartData.clear();
    m_vecVecFrameChartData = Vision::VectorOfVectorOfPoint2f(8, Vision::VectorOfPoint2f());

    for (int x = nLeft; x <= nRight; ++x)
    {
        stCmd.ptTargetFrameCenter.x = x;
        stCmd.ptTargetFrameCenter.y = (nTop + nBottom)/2;

        qDebug() << "test x " << stCmd.ptTargetFrameCenter.x << " " <<stCmd.ptTargetFrameCenter.y;
    
        stCmd.vecVecRefFrameValues = vecVecFrameDLP1;
        if (Vision::VisionStatus::OK == Vision::PR_CalcFrameValue(&stCmd, &stRpy)) {
            m_vecVecFrameChartData[0].push_back(cv::Point2f(x, stRpy.fResult));
        }
        else {
            System->setTrackInfo(QString(QStringLiteral("PR_CalcFrameValue error!")));
            m_bGood = false;
        }

        stCmd.vecVecRefFrameValues = vecVecFrameDLP2;
        if (Vision::VisionStatus::OK == Vision::PR_CalcFrameValue(&stCmd, &stRpy)) {
            m_vecVecFrameChartData[1].push_back(cv::Point2f(x, stRpy.fResult));
        }
        else {
            System->setTrackInfo(QString(QStringLiteral("PR_CalcFrameValue error!")));
            m_bGood = false;
        }

        stCmd.vecVecRefFrameValues = vecVecFrameDLP3;
        if (Vision::VisionStatus::OK == Vision::PR_CalcFrameValue(&stCmd, &stRpy)) {
            m_vecVecFrameChartData[2].push_back(cv::Point2f(x, stRpy.fResult));
        }
        else {
            System->setTrackInfo(QString(QStringLiteral("PR_CalcFrameValue error!")));
            m_bGood = false;
        }

        stCmd.vecVecRefFrameValues = vecVecFrameDLP4;
        if (Vision::VisionStatus::OK == Vision::PR_CalcFrameValue(&stCmd, &stRpy)) {
            m_vecVecFrameChartData[3].push_back(cv::Point2f(x, stRpy.fResult));
        }
        else {
            System->setTrackInfo(QString(QStringLiteral("PR_CalcFrameValue error!")));
            m_bGood = false;
        }

    }
   
    for (int y = nBottom; y <= nTop; ++y)
    {
        stCmd.ptTargetFrameCenter.x = (nLeft + nRight)/2;
        stCmd.ptTargetFrameCenter.y = y;

        qDebug() << "test y " << stCmd.ptTargetFrameCenter.x << " " << stCmd.ptTargetFrameCenter.y;
       
        stCmd.vecVecRefFrameValues = vecVecFrameDLP1;
        if (Vision::VisionStatus::OK == Vision::PR_CalcFrameValue(&stCmd, &stRpy)) {
            m_vecVecFrameChartData[4].push_back(cv::Point2f(y, stRpy.fResult));           
        }
        else {
            System->setTrackInfo(QString(QStringLiteral("PR_CalcFrameValue error!")));
            m_bGood = false;
        }

        stCmd.vecVecRefFrameValues = vecVecFrameDLP2;
        if (Vision::VisionStatus::OK == Vision::PR_CalcFrameValue(&stCmd, &stRpy)) {
            m_vecVecFrameChartData[5].push_back(cv::Point2f(y, stRpy.fResult));
        }
        else {
            System->setTrackInfo(QString(QStringLiteral("PR_CalcFrameValue error!")));
            m_bGood = false;
        }

        stCmd.vecVecRefFrameValues = vecVecFrameDLP3;
        if (Vision::VisionStatus::OK == Vision::PR_CalcFrameValue(&stCmd, &stRpy)) {
            m_vecVecFrameChartData[6].push_back(cv::Point2f(y, stRpy.fResult));
        }
        else {
            System->setTrackInfo(QString(QStringLiteral("PR_CalcFrameValue error!")));
            m_bGood = false;
        }

        stCmd.vecVecRefFrameValues = vecVecFrameDLP4;
        if (Vision::VisionStatus::OK == Vision::PR_CalcFrameValue(&stCmd, &stRpy)) {
            m_vecVecFrameChartData[7].push_back(cv::Point2f(y, stRpy.fResult));
        }
        else {
            System->setTrackInfo(QString(QStringLiteral("PR_CalcFrameValue error!")));
            m_bGood = false;
        }
    }


	System->setTrackInfo(QString(QStringLiteral("扫描区域完成")));
}

bool ScanImageThread::moveToCapturePos(float fPosX, float fPosY)
{
    IMotion* pMotion = getModule<IMotion>(MOTION_MODEL);
	if (!pMotion) return false;   

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


bool ScanImageThread::isExit()
{
	return m_exit;
}