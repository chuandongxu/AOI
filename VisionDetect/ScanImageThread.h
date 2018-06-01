#pragma once
#include <QThread>
#include <QThreadPool>
#include <atomic>

#include "DataStoreAPI.h"
#include "VisionAPI.h"

using namespace NFG::AOI;
using namespace AOI;

class ScanImageThread : public QThread
{
public:
    ScanImageThread(const Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr, float fLeft, float fTop, float fRight, float fBottom);
	~ScanImageThread();

	void quit();
    static bool captureAllImages(QVector<cv::Mat>& imageMats);
    Vision::VectorOfVectorOfPoint2f getFrameChartData() const { return m_vecVecFrameChartData; }
    bool isGood() const { return m_bGood; }

protected:
    bool preRunning();
	void run() override;
    bool moveToCapturePos(float fPosX, float fPosY);
	bool isExit();

private:
	std::atomic<bool>               m_exit;	

    double                          m_dResolutionX;
    double                          m_dResolutionY;

    float m_fLeft;
    float m_fTop;
    float m_fRight;
    float m_fBottom;

    int                             m_nDLPCount;
    Vision::VectorOfVectorOfPoint2f m_vecVecFrameCtr;
    QThreadPool                     m_threadPoolCalc3DHeight;
    Vision::VectorOfVectorOfPoint2f m_vecVecFrameChartData;
    bool                            m_bGood = true;
};


