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
        BoardInspResultPtr                         &ptrBoardInsResult);
    ~Insp3DHeightRunnable();

protected:
    virtual void run() override;
    void _insp3DHeightGroup(const Engine::WindowGroup &windowGroup);

private:
    QThreadPool                          *m_pThreadPoolCalc3DInsp2D;
    std::vector<Calc3DHeightRunnablePtr>  m_vecCalc3DHeightRunnable;
    Insp2DRunnablePtr                     m_ptrInsp2DRunnable;
    cv::Mat                               m_mat3DHeight;
};
