#pragma once

#include <QRunnable>
#include <QThreadPool>
#include "Calc3DHeightRunnable.h"
#include "Insp2DRunnable.h"

class Insp3DHeightRunnable : public InspRunnable
{
public:
    Insp3DHeightRunnable(
        QThreadPool*         pThreadPoolInsp2D,
        Insp2DRunnablePtr    ptrInsp2DRunnable,
        const cv::Point2f&   ptFramePos,        
        int                  nRow,
        int                  nCol,
        int                  nTotalRows,
        int                  nTotalCols,
        const AutoRunParams& stAutoRunParams,
        BoardInspResultPtr& ptrBoardInsResult);
    ~Insp3DHeightRunnable();
    void set3DHeight(const cv::Mat &mat3DHeight);

protected:
    virtual void run() override;
    void _insp3DHeightGroup(const Engine::WindowGroup &windowGroup);
    void _insp3DHeightGlobalBase(const Engine::Window &window);
    void _insp3DSolder(const Engine::Window &window);

private:
    QThreadPool                          *m_pThreadPoolInsp2D;
    std::vector<Calc3DHeightRunnablePtr>  m_vecCalc3DHeightRunnable;
    Insp2DRunnablePtr                     m_ptrInsp2DRunnable;
    cv::Mat                               m_mat3DHeight;
    Vision::VectorOfMat                  *m_pVec3DFrameImages;
    cv::Point2f                           m_ptFramePos;
    int                                   m_nRow, m_nCol, m_nTotalRows, m_nTotalCols;
    AutoRunParams                         m_stAutoRunParams;
};
