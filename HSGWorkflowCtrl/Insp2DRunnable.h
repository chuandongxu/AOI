#pragma once
#include <QRunnable>
#include <QVector>
#include "VisionAPI.h"
#include "DataStoreAPI.h"
#include "BoardInspResult.h"

using namespace NFG::AOI;
using namespace AOI;

class Insp2DRunnable :
    public QRunnable
{
public:
    Insp2DRunnable(const Vision::VectorOfMat    &vec2DImages,
                   const Engine::WindowVector   &vecWindows,
                   const cv::Point2f            &ptFramePos,
                   BoardInspResultPtr           &ptrBoardInsResult);

    ~Insp2DRunnable();
    void setResolution(double dResolutionX, double dResolutionY) { m_dResolutionX = dResolutionX; m_dResolutionY = dResolutionY; }
    void setImageSize(int nImageWidthPixel, int nImageHeightPixel) { m_nImageWidthPixel = nImageWidthPixel; m_nImageHeightPixel = nImageHeightPixel; }

protected:
    virtual void run() override;

private:
    bool _preprocessImage(const Engine::Window &window, cv::Mat &matOutput);
    void _inspHole(const Engine::Window &window);
    void _findLine(const Engine::Window &window);
    void _findCircle(const Engine::Window &window);
    void _alignment(const Engine::Window &window);
    
private:
    Vision::VectorOfMat                     m_vec2DImages;
    Engine::WindowVector                    m_vecWindows;
    BoardInspResultPtr                      m_ptrBoardInspResult;
    double                                  m_dResolutionX;
    double                                  m_dResolutionY;
    int                                     m_nImageWidthPixel;
    int                                     m_nImageHeightPixel;
    cv::Point2f                             m_ptFramePos;
};

