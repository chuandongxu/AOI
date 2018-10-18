#pragma once
#include <QThread>
#include <QThreadPool>
#include <QObject>
#include <atomic>
#include <condition_variable>

#include "VisionAPI.h"
#include "BoardInspResult.h"
#include "AutoRunDataStructs.h"
#include "../include/constants.h"

using namespace NFG::AOI;
using namespace AOI;

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
    
    bool captureAllImages(QVector<cv::Mat>& imageMats, int col);
    bool captureLightImages(QVector<cv::Mat>& imageMats, int index);
    cv::Mat getBigImage() const { return m_vecMatBigImage[PROCESSED_IMAGE_SEQUENCE::SOLDER_LIGHT]; }
    QString getErrorMsg() const { return m_strErrorMsg; }
    void nofityResponse(bool bExit);

protected slots:
    void onThreadState(const QVariantList &data);

protected:
    void quit();

    bool preRunning();

    void run() override;

    void postRunning();

    bool waitStartBtn();
    bool moveToCapturePos(float fPosX, float fPosY);
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
    DeviceInspWindowVector _getNotInspectedDeviceWindow() const;
    bool _combineBigImage(const Vision::VectorOfMat &vecMatImages, cv::Mat &matBigImage);
    void _generateResultBigImage(cv::Mat matBigImage, BoardInspResultPtr ptrBoardInspResult);
    void _sendErrorAndWaitForResponse();

private:
    std::atomic<bool>               m_exit;
    cv::Mat                         m_3DMatHeight;
    Engine::AlignmentVector         m_vecAlignments;
    double                          m_dResolutionX;
    double                          m_dResolutionY;
    int                             m_nDLPCount;
    int                             m_nTotalImageCount;
    cv::Mat                         m_matTransform;
    Vision::VectorOfVectorOfPoint2f m_vecVecFrameCtr;
    QThreadPool                     m_threadPoolCalc3D;
    QThreadPool                     m_threadPoolInsp2D;
    float                           m_fFovWidthUm;
    float                           m_fFovHeightUm;
    Vision::VectorOfMat             m_vecCombinedBigImages;
    cv::Mat                         m_matCombinedBigHeight;
    MapBoardInspResult             *m_pMapBoardInspResult;
    QString                         m_boardName;
    DeviceInspWindowVector          m_vecDeviceInspWindow;
    DeviceInspWindowVector          m_vecAlignedDeviceInspWindow;
    Vision::VectorOfVectorOfMat     m_vecVecFrameImages;
    Vision::VectorOfMat             m_vecFrame3DHeight;
    Vision::VectorOfMat             m_vecMatBigImage;
    cv::Mat                         m_matWhole3DHeight;
    QString                         m_strErrorMsg;
    std::condition_variable         m_conditionVariable;
    std::mutex                      m_mutex;
    AutoRunParams                   m_stAutoRunParams;
};


