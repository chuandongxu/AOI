#pragma once
#include <QThread>
#include <QThreadPool>
#include <atomic>

#include "DataStoreAPI.h"
#include "VisionAPI.h"

using namespace NFG::AOI;
using namespace AOI;

class AutoRunThread : public QThread
{
public:
	AutoRunThread(const Engine::AlignmentVector         &vecAlignments,
                 const Engine::WindowVector            &vecWindows,
                 const Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr);
	~AutoRunThread();

	void quit();

protected:
	bool preRunning();

	void run() override;
	
	bool waitStartBtn();
    bool moveToCapturePos(float fPosX, float fPosY);
	bool mergeImages(QString& szImagePath);
	bool isExit();

private:
	bool captureAllImages(QVector<cv::Mat>& imageMats);
	void setResoultLight(bool isOk);
	void resetResoultLight();

	bool getLightIO(int &okLight, int &ngLight);

	QString generateImagePath();
	void saveImages(const QString& szImagePath, int nRowIndex, int nColIndex, int nCountOfImgPerRow, const QVector<cv::Mat>& imageMats);
	void saveCombineImages(const QString& szImagePath, const QVector<cv::Mat>& imageMats);

    cv::Rect _calcImageRect(float fImgCapPosUmX, float fImgCapPosUmY, float fRectPosUmX, float fRectPosUmY, float fRectWidthUm, float fRectHeightUm);
    bool _feedBoard();
    bool _readBarcode();
    bool _doAlignment();
    bool _alignWindows();
    bool _doInspection();
    Engine::WindowVector _getWindowInFrame(const cv::Point2f &ptFrameCtr);
    Vision::VectorOfMat _generate2DImages(const Vision::VectorOfMat &vecInputImages);

private:
	std::atomic<bool> m_exit;
	
	cv::Mat m_3DMatHeight;

	int m_nImageIndex;

    Engine::AlignmentVector         m_vecAlignments;
    Engine::WindowVector            m_vecWindows;
    Engine::WindowVector            m_vecAlignedWindows;
    double                          m_dResolutionX;
    double                          m_dResolutionY;
    int                             m_nDLPCount;
    cv::Mat                         m_matTransform;
    Vision::VectorOfVectorOfPoint2f m_vecVecFrameCtr;
    QThreadPool                     m_threadPoolCalc3DHeight;
    float                           m_fFovWidthUm;
    float                           m_fFovHeightUm;
    int                             m_nImageWidthPixel;
    int                             m_nImageHeightPixel;
};


