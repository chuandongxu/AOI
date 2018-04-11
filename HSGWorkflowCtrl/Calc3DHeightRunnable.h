#pragma once

#include <QRunnable>
#include <QVector>
#include <memory>
#include "VisionAPI.h"

using namespace AOI;

class Calc3DHeightRunnable : public QRunnable
{
public:
    Calc3DHeightRunnable(int nStationId, const QVector<cv::Mat> &vecMatImages);
    ~Calc3DHeightRunnable();
    inline cv::Mat get3DHeight() const { return m_mat3DHeight; }
    inline cv::Mat getHeightResultImg() const { return m_matHeightResultImg; }

protected:
    virtual void run() override;

private:
    int                     m_nDlpId;
    QVector<cv::Mat>        m_vecMatImages;
    cv::Mat                 m_mat3DHeight;
    cv::Mat                 m_matHeightResultImg;
};

using Calc3dHeightRunnablePtr = std::shared_ptr<Calc3DHeightRunnable>;
