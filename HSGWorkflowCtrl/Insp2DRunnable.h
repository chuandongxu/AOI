#pragma once
#include <QRunnable>
#include <QVector>
#include <mutex>
#include "InspRunnable.h"

using namespace AOI;

class Insp2DRunnable : public InspRunnable
{
public:
    Insp2DRunnable(const Vision::VectorOfMat    &vec2DImages,
                   const DeviceInspWindowVector &vecDeviceWindows,
                   const cv::Point2f            &ptFramePos,
                   BoardInspResultPtr           &ptrBoardInsResult);

    ~Insp2DRunnable();
    DeviceInspWindowVector getDeviceInspWindow() const { return m_vecDeviceWindows; };

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

private:
    Vision::VectorOfMat     m_vec2DImages;
    DeviceInspWindowVector  m_vecDeviceWindows;
};

using Insp2DRunnablePtr = std::shared_ptr<Insp2DRunnable>;

