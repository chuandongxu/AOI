#include "Merge3DHeightRunnable.h"
#include "../include/IVision.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"

Merge3DHeightRunnable::Merge3DHeightRunnable(
    QThreadPool                                *pCalc3DHeightThreadPool,
    const std::vector<Calc3DHeightRunnablePtr> &vecCalc3DHeightRunnable,
    const cv::Point2f                          &ptFramePos,
    VectorOfMat                                *pVec3DFrameImages,
    int                                         nRow,
    int                                         nCol,
    int                                         nTotalRows,
    int                                         nTotalCols) :
    m_pCalc3dHeightThreadPool (pCalc3DHeightThreadPool),
    m_vecCalc3DHeightRunnable (vecCalc3DHeightRunnable),
    m_ptFramePos              (ptFramePos),
    m_pVec3DFrameImages       (pVec3DFrameImages),
    m_nRow                    (nRow),
    m_nCol                    (nCol),
    m_nTotalRows              (nTotalRows),
    m_nTotalCols              (nTotalCols)
{
}

Merge3DHeightRunnable::~Merge3DHeightRunnable()
{
}

void Merge3DHeightRunnable::run()
{
    m_pCalc3dHeightThreadPool->waitForDone();

    QVector<cv::Mat> vecMatHeight, vecNanMask;
    for (const auto &ptrCalc3DHeightRunnable : m_vecCalc3DHeightRunnable) {
        vecMatHeight.push_back(ptrCalc3DHeightRunnable->get3DHeight());
        vecNanMask.push_back(ptrCalc3DHeightRunnable->getNanMask());
    }

    IVision* pVision = getModule<IVision>(VISION_MODEL);
    if (!pVision) return;

    cv::Mat matMerged3DHeight, matNanMask;
    pVision->setInspect3DHeight(vecMatHeight, m_nRow, m_nCol, m_nTotalRows, m_nTotalCols);
    pVision->merge3DHeight(vecMatHeight, vecNanMask, matMerged3DHeight, matNanMask, m_ptFramePos);

    (*m_pVec3DFrameImages)[m_nRow * m_nTotalCols + m_nCol] = matMerged3DHeight;
}