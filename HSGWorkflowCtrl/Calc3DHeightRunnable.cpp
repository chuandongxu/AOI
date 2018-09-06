#include <QThread>
#include <QDebug>

#include "Calc3DHeightRunnable.h"
#include "../include/IVision.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"

Calc3DHeightRunnable::Calc3DHeightRunnable(int nDlpId, const QVector<cv::Mat> &vecMatImages) :
    m_nDlpId       (nDlpId),
    m_vecMatImages (vecMatImages)
{
}

Calc3DHeightRunnable::~Calc3DHeightRunnable() {
}

void Calc3DHeightRunnable::run()
{
    IVision* pVision = getModule<IVision>(VISION_MODEL);
    if (!pVision) return;

    pVision->calculate3DHeight(m_nDlpId, m_vecMatImages, m_mat3DHeight, m_matNanMask, m_matHeightResultImg);
}