#include "Insp3DHeightRunnable.h"
#include "TimeLog.h"
#include "../include/IVision.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"

Insp3DHeightRunnable::Insp3DHeightRunnable(QThreadPool *pCalc3DHeightThreadPool, const std::vector<Calc3dHeightRunnablePtr> &vecCalc3DHeightRunnable) :
    m_pCalc3dHeightThreadPool(pCalc3DHeightThreadPool),
    m_vecCalc3DHeightRunnable(vecCalc3DHeightRunnable)
{
}

Insp3DHeightRunnable::~Insp3DHeightRunnable()
{
}

void Insp3DHeightRunnable::run()
{
    m_pCalc3dHeightThreadPool->waitForDone();
    
    TimeLogInstance->addTimeLog(std::string("Finished wait for 3d calculation done in thead ") + QThread::currentThread()->objectName().toStdString());

    QVector<cv::Mat> vecMatHeight;
    for (const auto &ptrCalc3DHeightRunnable : m_vecCalc3DHeightRunnable)
        vecMatHeight.push_back(ptrCalc3DHeightRunnable->get3DHeight());

    IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return;

    pVision->merge3DHeight(vecMatHeight, m_mat3DHeight);
}