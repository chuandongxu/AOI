#pragma once
#include <QRunnable>
#include <QVector>
#include "VisionAPI.h"
#include "DataStoreAPI.h"

using namespace NFG::AOI;
using namespace AOI;

class Insp2DRunnable :
    public QRunnable
{
public:
    Insp2DRunnable(const Vision::VectorOfMat &vec2DImages, const Engine::WindowVector &vecWindows);
    ~Insp2DRunnable();

protected:
    virtual void run() override;

private:
    bool _preprocessImage(const cv::Mat &matInput, const Engine::Window &window, cv::Mat &matOutput);
    void _inspHole(const Engine::Window &window);
    void _findLine(const Engine::Window &window);
    
private:
    Vision::VectorOfMat                     m_vec2DImages;
    Engine::WindowVector                    m_vecWindows;
    std::map<Int64, Vision::VisionStatus>   m_mapWindowStatus; 
};

