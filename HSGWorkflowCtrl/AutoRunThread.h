#pragma once
#include <QThread>
#include <QThreadPool>
#include <atomic>

#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "BoardInspResult.h"

using namespace NFG::AOI;
using namespace AOI;

class AutoRunThread : public QThread
{
public:
	AutoRunThread(const Engine::AlignmentVector         &vecAlignments,
                  const Engine::WindowVector            &vecWindows,
                  const Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr,
                  MapBoardInspResult                    *pMapBoardInspResult);
	~AutoRunThread();

	void quit();
    static bool captureAllImages(QVector<cv::Mat>& imageMats);
    inline void setImageSize(int nImgWidth, int nImgHeight) { m_nImageWidthPixel = nImgWidth; m_nImageHeightPixel = nImgHeight; }
    inline void setBoardStartPos(float fBoardLeftPos, float fBoardBtmPos) { m_fBoardLeftPos = fBoardLeftPos; m_fBoardBtmPos = fBoardBtmPos; }

protected:
	bool preRunning();

	void run() override;
	
	bool waitStartBtn();
    bool moveToCapturePos(float fPosX, float fPosY);
	bool mergeImages(QString& szImagePath);
	bool isExit();

private:	
	void setResoultLight(bool isOk);
	void resetResoultLight();

	bool getLightIO(int &okLight, int &ngLight);

	QString generateImagePath();
	void saveImages(const QString& szImagePath, int nRowIndex, int nColIndex, int nCountOfImgPerRow, const QVector<cv::Mat>& imageMats);
	void saveCombineImages(const QString& szImagePath, const QVector<cv::Mat>& imageMats);

    bool _feedBoard();
    bool _readBarcode();
    bool _doAlignment();
    bool _alignWindows();
    bool _doInspection(BoardInspResultPtr ptrBoardInspResult);
    Engine::WindowVector _getWindowInFrame(const cv::Point2f &ptFrameCtr);
    Vision::VectorOfMat _generate2DImages(const Vision::VectorOfMat &vecInputImages);

private:
	std::atomic<bool>               m_exit;	
	cv::Mat                         m_3DMatHeight;
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
    Vision::VectorOfMat             m_vecCombinedBigImages;
    cv::Mat                         m_matCombinedBigHeight;
    float                           m_fBoardLeftPos;
    float                           m_fBoardBtmPos;
    MapBoardInspResult             *m_pMapBoardInspResult;
    QString                         m_boardName;
};


