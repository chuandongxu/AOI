#include "AlignmentRunnable.h"
#include "../Common/SystemData.h"

AlignmentRunnable::AlignmentRunnable(const cv::Mat &matImage, const Engine::Alignment &alignment, const cv::Rect &rectSrchWindow) :
    m_matImage          (matImage),
    m_alignment         (alignment),
    m_rectSrchWindow    (rectSrchWindow)
{
}

AlignmentRunnable::~AlignmentRunnable() {
}

void AlignmentRunnable::run()
{
    if (m_alignment.isFM)
        _srchStandardFM();
    else
        _srchRealImageFM();
}

void AlignmentRunnable::_srchStandardFM()
{
    Vision::PR_SRCH_FIDUCIAL_MARK_CMD stCmd;
    Vision::PR_SRCH_FIDUCIAL_MARK_RPY stRpy;

    stCmd.matInputImg = m_matImage;
    stCmd.enType = static_cast<Vision::PR_FIDUCIAL_MARK_TYPE> (m_alignment.fmShape);
    stCmd.fSize = m_alignment.tmplWidth / m_dResolutionX;
    stCmd.fMargin = stCmd.fSize / 2.f;
    stCmd.rectSrchWindow = m_rectSrchWindow;
    if (stCmd.rectSrchWindow.x < 0) stCmd.rectSrchWindow.x = 0;
    if (stCmd.rectSrchWindow.y < 0) stCmd.rectSrchWindow.y = 0;
    if ((stCmd.rectSrchWindow.x + stCmd.rectSrchWindow.width) > stCmd.matInputImg.cols)
        stCmd.rectSrchWindow.width = stCmd.matInputImg.cols - stCmd.rectSrchWindow.x;
    if ((stCmd.rectSrchWindow.y + stCmd.rectSrchWindow.height) > stCmd.matInputImg.rows)
        stCmd.rectSrchWindow.height = stCmd.matInputImg.rows - stCmd.rectSrchWindow.y;

    m_enVisionStatus = Vision::PR_SrchFiducialMark(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK == m_enVisionStatus) {
        m_ptResultCtrOffset.x =  (stRpy.ptPos.x - (m_rectSrchWindow.x + m_rectSrchWindow.width  / 2)) * m_dResolutionX;
        m_ptResultCtrOffset.y = -(stRpy.ptPos.y - (m_rectSrchWindow.y + m_rectSrchWindow.height / 2)) * m_dResolutionY;    //The direction of image Y and table Y are inversed.
    }else {
        Vision::PR_GET_ERROR_INFO_RPY stErrStrRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrStrRpy);
        QString strMsg(QStringLiteral("基准定位错误, 错误消息: "));
        strMsg += stErrStrRpy.achErrorStr;
        System->setTrackInfo(strMsg, true);
    }
}

void AlignmentRunnable::_srchRealImageFM()
{
    Vision::PR_MATCH_TEMPLATE_CMD stCmd;
    Vision::PR_MATCH_TEMPLATE_RPY stRpy;

    stCmd.matInputImg = m_matImage;
    stCmd.nRecordId = m_alignment.recordId;
    stCmd.rectSrchWindow = m_rectSrchWindow;
    if (stCmd.rectSrchWindow.x < 0) stCmd.rectSrchWindow.x = 0;
    if (stCmd.rectSrchWindow.y < 0) stCmd.rectSrchWindow.y = 0;
    if ((stCmd.rectSrchWindow.x + stCmd.rectSrchWindow.width) > stCmd.matInputImg.cols)
        stCmd.rectSrchWindow.width = stCmd.matInputImg.cols - stCmd.rectSrchWindow.x;
    if ((stCmd.rectSrchWindow.y + stCmd.rectSrchWindow.height) > stCmd.matInputImg.rows)
        stCmd.rectSrchWindow.height = stCmd.matInputImg.rows - stCmd.rectSrchWindow.y;

    m_enVisionStatus = Vision::PR_MatchTmpl(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK == m_enVisionStatus) {
        m_ptResultCtrOffset.x =  (stRpy.ptObjPos.x - m_matImage.cols / 2) * m_dResolutionX;
        m_ptResultCtrOffset.y = -(stRpy.ptObjPos.y - m_matImage.rows / 2) * m_dResolutionY;    //The direction of image Y and table Y are inversed.
    }else {
        Vision::PR_GET_ERROR_INFO_RPY stErrStrRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrStrRpy);
        QString strMsg(QStringLiteral("基准定位错误, 错误消息: "));
        strMsg += stErrStrRpy.achErrorStr;
        System->setTrackInfo(strMsg, true);
    }
}