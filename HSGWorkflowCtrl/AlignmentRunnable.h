#pragma once
#include <QRunnable>
#include <memory>
#include "opencv/cv.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"

using namespace NFG::AOI;
using namespace AOI;

class AlignmentRunnable :
    public QRunnable
{
public:
    AlignmentRunnable(const cv::Mat &matImage, const Engine::Alignment &alignment, const cv::Rect &rectSrchWindow);
    ~AlignmentRunnable();
    void setResolution(double dResolutionX, double dResolutionY) { m_dResolutionX = dResolutionX; m_dResolutionY = dResolutionY; }
    void run() override;
    cv::Point2f getResultCtrOffset() const { return m_ptResultCtrOffset; }

private:
    cv::Mat                 m_matImage;
    Engine::Alignment       m_alignment;
    cv::Rect                m_rectSrchWindow;
    cv::Point2f             m_ptResultCtrOffset;
    double                  m_dResolutionX;
    double                  m_dResolutionY;
    Vision::VisionStatus    m_enVisionStatus;
};

using AlignmentRunnablePtr = std::unique_ptr<AlignmentRunnable>;