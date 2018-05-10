#pragma once

#include <QRunnable>

#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "BoardInspResult.h"
#include "AutoRunDataStructs.h"

using namespace NFG::AOI;
using namespace AOI;

class InspRunnable :
    public QRunnable
{
public:
    InspRunnable(const cv::Point2f            &ptFramePos,
                 BoardInspResultPtr           &ptrBoardInsResult) :
      m_ptFramePos          (ptFramePos),
      m_ptrBoardInspResult  (ptrBoardInsResult) {}

    ~InspRunnable() = default;
    void setResolution(double dResolutionX, double dResolutionY) { m_dResolutionX = dResolutionX; m_dResolutionY = dResolutionY; }
    void setImageSize(int nImageWidthPixel, int nImageHeightPixel) { m_nImageWidthPixel = nImageWidthPixel; m_nImageHeightPixel = nImageHeightPixel; }

protected:
    BoardInspResultPtr                      m_ptrBoardInspResult;
    double                                  m_dResolutionX;
    double                                  m_dResolutionY;
    int                                     m_nImageWidthPixel;
    int                                     m_nImageHeightPixel;
    cv::Point2f                             m_ptFramePos;
};