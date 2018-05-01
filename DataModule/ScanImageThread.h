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
	ScanImageThread(
        const Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr,
        float                                  fOverlapUmX,
        float                                  fOverlapUmY,
        Vision::PR_SCAN_IMAGE_DIR              enScanDir);
	~ScanImageThread();

	void quit();
    static bool captureAllImages(QVector<cv::Mat>& imageMats);
    Vision::VectorOfMat getCombinedBigImages() const { return m_vecCombinedBigImages; }
    cv::Mat getCombinedBigHeight() const { return m_matCombinedBigHeight; }
    bool isGood() const { return m_bGood; }

protected:
    bool preRunning();
	void run() override;
    bool moveToCapturePos(float fPosX, float fPosY);
	bool mergeImages(QString& szImagePath);
	bool isExit();

private:
    Vision::VectorOfMat _generate2DImages(const Vision::VectorOfMat &vecInputImages);

    QString generateImagePath();
	void saveImages(const QString& szImagePath, int nRowIndex, int nColIndex, int nCountOfImgPerRow, const QVector<cv::Mat>& imageMats);
	void saveCombineImages(const QString& szImagePath, const QVector<cv::Mat>& imageMats);

private:
	std::atomic<bool>               m_exit;
	
	cv::Mat                         m_3DMatHeight;

    float                           m_fOverlapUmX;
    float                           m_fOverlapUmY;
    double                          m_dResolutionX;
    double                          m_dResolutionY;
    Vision::PR_SCAN_IMAGE_DIR       m_enScanDir;

    int                             m_nDLPCount;
    cv::Mat                         m_matTransform;
    Vision::VectorOfVectorOfPoint2f m_vecVecFrameCtr;
    QThreadPool                     m_threadPoolCalc3DHeight;
    Vision::VectorOfMat             m_vecCombinedBigImages;
    cv::Mat                         m_matCombinedBigHeight;
    bool                            m_bGood = true;
};


