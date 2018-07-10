#pragma once

#include <QRunnable>
#include <QThreadPool>
#include "Calc3DHeightRunnable.h"
#include "Insp2DRunnable.h"

class Insp3DHeightRunnable : public InspRunnable
{
public:
    Insp3DHeightRunnable(
        QThreadPool                                *pThreadPoolCalc3DInsp2D,
        const std::vector<Calc3DHeightRunnablePtr> &vecCalc3DHeightRunnable,
        Insp2DRunnablePtr                           ptrInsp2DRunnable,
        const cv::Point2f                          &ptFramePos,
        Vision::VectorOfMat                        *pVec3DFrameImages,
        int                                         nRow,
        int                                         nCol,
        int                                         nTotalRows,
        int                                         nTotalCols,
        BoardInspResultPtr                         &ptrBoardInsResult);
    ~Insp3DHeightRunnable();
    void set3DHeight(const cv::Mat &mat3DHeight);

protected:
    virtual void run() override;
    void _insp3DHeightGroup(const Engine::WindowGroup &windowGroup);

private:
    QThreadPool                          *m_pThreadPoolCalc3DInsp2D;
    std::vector<Calc3DHeightRunnablePtr>  m_vecCalc3DHeightRunnable;
    Insp2DRunnablePtr                     m_ptrInsp2DRunnable;
    cv::Mat                               m_mat3DHeight;
    Vision::VectorOfMat                  *m_pVec3DFrameImages;
    cv::Point2f                           m_ptFramePos;
    int                                   m_nRow, m_nCol, m_nTotalRows, m_nTotalCols;
};
