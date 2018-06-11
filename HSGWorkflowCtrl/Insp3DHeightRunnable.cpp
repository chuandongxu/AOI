#include <QJsonObject>
#include <QJsonDocument>

#include "Insp3DHeightRunnable.h"
#include "TimeLog.h"
#include "../include/IVision.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"

#include "../DataModule/DataUtils.h"

Insp3DHeightRunnable::Insp3DHeightRunnable(
    QThreadPool                                *pCalc3DHeightThreadPool,
    const std::vector<Calc3DHeightRunnablePtr> &vecCalc3DHeightRunnable,
    Insp2DRunnablePtr                           ptrInsp2DRunnable,
    const cv::Point2f                          &ptFramePos,
    Vision::VectorOfMat                        *pVec3DFrameImages,
    int                                         nRow,
    int                                         nCol,
    int                                         nTotalRows,
    int                                         nTotalCols,
    BoardInspResultPtr                         &ptrBoardInsResult) :
        m_pThreadPoolCalc3DInsp2D   (pCalc3DHeightThreadPool),
        m_vecCalc3DHeightRunnable   (vecCalc3DHeightRunnable),
        m_ptrInsp2DRunnable         (ptrInsp2DRunnable),
        m_pVec3DFrameImages         (pVec3DFrameImages),
        m_nRow                      (nRow),
        m_nCol                      (nCol),
        m_nTotalRows                (nTotalRows),
        m_nTotalCols                (nTotalCols),
        InspRunnable                (ptFramePos, ptrBoardInsResult)
{
}

Insp3DHeightRunnable::~Insp3DHeightRunnable() {
}

void Insp3DHeightRunnable::set3DHeight(const cv::Mat &mat3DHeight) {
    m_mat3DHeight = mat3DHeight;
}

void Insp3DHeightRunnable::run()
{
    m_pThreadPoolCalc3DInsp2D->waitForDone();
    TimeLogInstance->addTimeLog(std::string("Finished wait for 3D calculation done in thead ") + QThread::currentThread()->objectName().toStdString());

    if (!m_vecCalc3DHeightRunnable.empty()) {
        QVector<cv::Mat> vecMatHeight;
        for (const auto &ptrCalc3DHeightRunnable : m_vecCalc3DHeightRunnable)
            vecMatHeight.push_back(ptrCalc3DHeightRunnable->get3DHeight());

        IVision* pVision = getModule<IVision>(VISION_MODEL);
        if (!pVision) return;

        pVision->setInspect3DHeight(vecMatHeight);
        pVision->merge3DHeight(vecMatHeight, m_mat3DHeight);
        (*m_pVec3DFrameImages)[m_nRow * m_nTotalCols + m_nCol] = m_mat3DHeight;
    }

    auto vecDeviceInspWindow = m_ptrInsp2DRunnable->getDeviceInspWindow();
    for (const auto &deviceInspWindow : vecDeviceInspWindow) {
        if (! deviceInspWindow.bAlignmentPassed)
            continue;

        for (const auto &windowGroup : deviceInspWindow.vecWindowGroup) {
            auto iterHeightCheckWindow = std::find_if(windowGroup.vecWindows.begin(), windowGroup.vecWindows.end(), [](const Engine::Window &window) { return Engine::Window::Usage::HEIGHT_MEASURE == window.usage; });
            if (iterHeightCheckWindow != windowGroup.vecWindows.end()) {
                _insp3DHeightGroup(windowGroup);
            }
        }
        m_ptrBoardInspResult->addDeviceInspWindow(deviceInspWindow);
    }
}

void Insp3DHeightRunnable::_insp3DHeightGroup(const Engine::WindowGroup &windowGroup)
{
    auto iterHeightCheckWindow = std::find_if(windowGroup.vecWindows.begin(), windowGroup.vecWindows.end(), [](const Engine::Window &window) { return Engine::Window::Usage::HEIGHT_MEASURE == window.usage; });
    auto windowInsp = *iterHeightCheckWindow;

    QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(windowInsp.inspParams.c_str(), &json_error);
	if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        std::string strErrorMsg = "Window \"" + windowInsp.name + "\" color parameters \"" + windowInsp.colorParams + "\" is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
		return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_CALC_3D_HEIGHT_DIFF_CMD stCmd;
	Vision::PR_CALC_3D_HEIGHT_DIFF_RPY stRpy;

    stCmd.fEffectHRatioStart = jsonValue["MinRange"].toDouble();
	stCmd.fEffectHRatioEnd = jsonValue["MaxRange"].toDouble();

    stCmd.rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(windowInsp.x, windowInsp.y),
        windowInsp.width,
        windowInsp.height,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);
    stCmd.matHeight = m_mat3DHeight;

    for (const auto &window : windowGroup.vecWindows) {
        if (Engine::Window::Usage::HEIGHT_BASE == window.usage) {
            auto rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
                window.width,
                window.height,
                m_ptFramePos,
                m_nImageWidthPixel,
                m_nImageHeightPixel,
                m_dResolutionX,
                m_dResolutionY);
            stCmd.vecRectBases.push_back(rectROI);
        }
    }

    Vision::PR_Calc3DHeightDiff(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + windowInsp.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }

    for (const auto &window : windowGroup.vecWindows)
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}