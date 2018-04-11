#include "AlignmentRunnable.h"

AlignmentRunnable::AlignmentRunnable(const cv::Mat &matImage, const Engine::Alignment &alignment, const cv::Rect &rectSrchWindow) :
    m_matImage          (matImage),
    m_alignment         (alignment),
    m_rectSrchWindow    (rectSrchWindow)
{
}

AlignmentRunnable::~AlignmentRunnable()
{
}

void AlignmentRunnable::run()
{
    Vision::PR_SRCH_FIDUCIAL_MARK_CMD stCmd;
    Vision::PR_SRCH_FIDUCIAL_MARK_RPY stRpy;

    stCmd.matInputImg = m_matImage;
    stCmd.rectSrchWindow = m_rectSrchWindow;
    stCmd.enType = static_cast<Vision::PR_FIDUCIAL_MARK_TYPE>(m_alignment.fmShape);
    stCmd.fSize = m_alignment.tmplWidth / m_dResolutionX;
    stCmd.fMargin = stCmd.fSize / 2.f;
    
    m_enVisionStatus = Vision::PR_SrchFiducialMark(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK == m_enVisionStatus) {
        m_ptResultCtrOffset.x =  (stRpy.ptPos.x - m_matImage.cols / 2) / m_dResolutionX;
        m_ptResultCtrOffset.y = -(stRpy.ptPos.y - m_matImage.rows / 2) / m_dResolutionY;    //The direction of image Y and table Y are inversed.
    }
}