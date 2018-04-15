#include "Merge3DHeightRunnable.h"
#include "../include/IVision.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"

Merge3DHeightRunnable::Merge3DHeightRunnable(
    QThreadPool                                *pCalc3DHeightThreadPool,
    const std::vector<Calc3dHeightRunnablePtr> &vecCalc3DHeightRunnable,
    VectorOfMat                                *pVec3DFrameImages,
    int                                         nRow,
    int                                         nCol,
    int                                         nTotalRows,
    int                                         nTotalCols) :
    m_pCalc3dHeightThreadPool (pCalc3DHeightThreadPool),
    m_vecCalc3DHeightRunnable (vecCalc3DHeightRunnable),
    m_pVec3DFrameImages       (pVec3DFrameImages),
    m_nRow                    (nRow),
    m_nCol                    (nCol),
    m_nTotalRows              (nTotalRows)
{
}

Merge3DHeightRunnable::~Merge3DHeightRunnable()
{
}

void Merge3DHeightRunnable::run()
{
    m_pCalc3dHeightThreadPool->waitForDone();    

    QVector<cv::Mat> vecMatHeight;
    for (const auto &ptrCalc3DHeightRunnable : m_vecCalc3DHeightRunnable)
        vecMatHeight.push_back(ptrCalc3DHeightRunnable->get3DHeight());

    IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return;

    cv::Mat matMerged3DHeight;
    pVision->merge3DHeight(vecMatHeight, matMerged3DHeight);

    m_pVec3DFrameImages[m_nRow * m_nTotalCols + m_nCol] = matMerged3DHeight;
}