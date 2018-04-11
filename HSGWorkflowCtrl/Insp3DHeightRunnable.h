#pragma once

#include <QRunnable>
#include <QThreadPool>
#include "Calc3DHeightRunnable.h"

class Insp3DHeightRunnable : public QRunnable
{
public:
    Insp3DHeightRunnable(QThreadPool *pCalc3DHeightThreadPool, const std::vector<Calc3dHeightRunnablePtr> &vecCalc3DHeightRunnable);
    ~Insp3DHeightRunnable();

protected:
    virtual void run() override;

private:
    QThreadPool                          *m_pCalc3dHeightThreadPool;
    std::vector<Calc3dHeightRunnablePtr>  m_vecCalc3DHeightRunnable;
    cv::Mat                               m_mat3DHeight;
};
