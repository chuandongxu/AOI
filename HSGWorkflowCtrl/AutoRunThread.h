#pragma once
#include <QThread>
#include <QThreadPool>
#include <QObject>
#include <atomic>

#include "VisionAPI.h"
#include "BoardInspResult.h"
#include "AutoRunDataStructs.h"

using namespace NFG::AOI;
using namespace AOI;

struct AutoRunParams
{
    AutoRunParams(int     nImgWidthPixel,
                  int     nImgHeightPixel,
                  float   fBoardLeftPos,
                  float   fBoardBtmPos,
                  float   fFrameOverlapX,
                  float   fFrameOverlapY,
                  Vision::PR_SCAN_IMAGE_DIR       enScanDir) :
                  nImgWidthPixel    (nImgWidthPixel),
                  nImgHeightPixel   (nImgHeightPixel),
                  fBoardLeftPos     (fBoardLeftPos),
                  fBoardBtmPos      (fBoardBtmPos),
                  fOverlapUmX       (fOverlapUmX),
                  fOverlapUmY       (fOverlapUmY),
                  enScanDir         (enScanDir) {}
    int     nImgWidthPixel;
    int     nImgHeightPixel;
    float   fBoardLeftPos;
    float   fBoardBtmPos;
    float   fOverlapUmX;
    float   fOverlapUmY;
    Vision::PR_SCAN_IMAGE_DIR       enScanDir;
};

class AutoRunThread : public QThread
{
    Q_OBJECT

public:
	AutoRunThread(const Engine::AlignmentVector         &vecAlignments,
                  const DeviceInspWindowVector          &vecDeviceWindows,
                  const Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr,
                  const AutoRunParams                   &stAutoRunParams,
                  MapBoardInspResult                    *pMapBoardInspResult);
	~AutoRunThread();
	
    static bool captureAllImages(QVector<cv::Mat>& imageMats);
    cv::Mat getBigImage() const { return m_matBigImage; }

public slots:
    void onThreadState(const QVariantList &data);

protected:
    void quit();

	bool preRunning();

	void run() override;

    void postRunning();
	
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
    void _transformPosition(float &x, float &y);
    bool _alignWindows();
    bool _doInspection(BoardInspResultPtr ptrBoardInspResult);
    DeviceInspWindowVector _getDeviceWindowInFrame(const cv::Point2f &ptFrameCtr);
    bool _combineBigImage(const Vision::VectorOfMat &vecMatImages);

private:
	std::atomic<bool>               m_exit;	
	cv::Mat                         m_3DMatHeight;
    Engine::AlignmentVector         m_vecAlignments;
    //Engine::WindowVector            m_vecWindows;
    //Engine::WindowVector            m_vecAlignedWindows;
    double                          m_dResolutionX;
    double                          m_dResolutionY;
    int                             m_nDLPCount;
    cv::Mat                         m_matTransform;
    Vision::VectorOfVectorOfPoint2f m_vecVecFrameCtr;
    QThreadPool                     m_threadPoolCalc3DInsp2D;
    float                           m_fFovWidthUm;
    float                           m_fFovHeightUm;
    int                             m_nImageWidthPixel;
    int                             m_nImageHeightPixel;
    Vision::VectorOfMat             m_vecCombinedBigImages;
    cv::Mat                         m_matCombinedBigHeight;
    float                           m_fBoardLeftPos;
    float                           m_fBoardBtmPos;
    float                           m_fOverlapUmX;
    float                           m_fOverlapUmY;
    MapBoardInspResult             *m_pMapBoardInspResult;
    QString                         m_boardName;
    DeviceInspWindowVector          m_vecDeviceInspWindow;
    DeviceInspWindowVector          m_vecAlignedDeviceInspWindow;
    Vision::VectorOfMat             m_vecDisplayFrameImages;
    cv::Mat                         m_matBigImage;
    Vision::PR_SCAN_IMAGE_DIR       m_enScanDir;
};


