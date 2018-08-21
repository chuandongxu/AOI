#pragma once
#include <QRunnable>
#include <QVector>
#include <QThreadPool>

#include <mutex>
#include "InspRunnable.h"
#include "Calc3DHeightRunnable.h"

using namespace AOI;

class Insp2DRunnable : public InspRunnable
{
public:
    Insp2DRunnable(
        const cv::Mat                &mat3DHeight,
        const Vision::VectorOfMat    &vec2DImages,
        const DeviceInspWindowVector &vecDeviceWindows,
        const cv::Point2f            &ptFramePos,
        BoardInspResultPtr           &ptrBoardInsResult);
    Insp2DRunnable(const Vision::VectorOfMat            &vec2DCaptureImages,
                   Vision::VectorOfVectorOfMat          *pVecVecFrameImages,
                   QThreadPool                          *pThreadPoolCalc3D,
                   const VectorCalc3DHeightRunnablePtr  &vecCalc3DHeightRunnable,
                   Vision::VectorOfMat                  *pVec3DFrameImages,
                   const DeviceInspWindowVector         &vecDeviceWindows,
                   int                                   nRow,
                   int                                   nCol,
                   int                                   nTotalRows,
                   int                                   nTotalCols,
                   const cv::Point2f                    &ptFramePos,
                   BoardInspResultPtr                   &ptrBoardInsResult
                   );

    ~Insp2DRunnable();
    DeviceInspWindowVector getDeviceInspWindow() const { return m_vecDeviceWindows; };
    cv::Mat get3DHeight() const { return m_mat3DHeight; }

protected:
    virtual void run() override;

private:
    void _inspWindow(const Engine::Window &window);
    bool _preprocessImage(const Engine::Window &window, cv::Mat &matOutput);
    void _inspChip(const Engine::Window &window);
    void _inspHole(const Engine::Window &window);
    void _findLine(const Engine::Window &window);
    void _findCircle(const Engine::Window &window);
    void _inspContour(const Engine::Window &window);
    void _inspPolarityGroup(const Engine::WindowGroup &windowGroup);
    void _inspBridge(const Engine::Window &window);
    void _inspLead(const Engine::Window &window);
    void _ocv(const Engine::Window &window);
    Vision::VisionStatus _alignment(const Engine::Window &window, DeviceInspWindow &deviceInspWindow);
    Vision::VectorOfMat _generate2DImages(const Vision::VectorOfMat &vecInputImages);
    void _prepareImages();

private:
    Vision::VectorOfMat           m_vec2DCaptureImages;
    Vision::VectorOfMat           m_vec2DImages;
    QThreadPool                  *m_pThreadPoolCalc3D;
    Vision::VectorOfVectorOfMat  *m_pVecVecFrameImages;
    VectorCalc3DHeightRunnablePtr m_vecCalc3DHeightRunnable;
    Vision::VectorOfMat          *m_pVec3DFrameImages;
    DeviceInspWindowVector        m_vecDeviceWindows;
    int                           m_nRow, m_nCol, m_nTotalRows, m_nTotalCols;
    cv::Mat                       m_mat3DHeight;
};

using Insp2DRunnablePtr = std::shared_ptr<Insp2DRunnable>;

