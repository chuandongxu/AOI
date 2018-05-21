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
    BoardInspResultPtr                         &ptrBoardInsResult) :
    m_pThreadPoolCalc3DInsp2D   (pCalc3DHeightThreadPool),
    m_vecCalc3DHeightRunnable   (vecCalc3DHeightRunnable),
    m_ptrInsp2DRunnable         (ptrInsp2DRunnable),
    InspRunnable                (ptFramePos, ptrBoardInsResult)
{
}

Insp3DHeightRunnable::~Insp3DHeightRunnable()
{
}

void Insp3DHeightRunnable::run()
{
    m_pThreadPoolCalc3DInsp2D->waitForDone();    
    TimeLogInstance->addTimeLog(std::string("Finished wait for 3D calculation done in thead ") + QThread::currentThread()->objectName().toStdString());

    auto vecDeviceInspWindow = m_ptrInsp2DRunnable->getDeviceInspWindow();

    QVector<cv::Mat> vecMatHeight;
    for (const auto &ptrCalc3DHeightRunnable : m_vecCalc3DHeightRunnable)
        vecMatHeight.push_back(ptrCalc3DHeightRunnable->get3DHeight());

    IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return;

    pVision->setInspect3DHeight(vecMatHeight);
    pVision->merge3DHeight(vecMatHeight, m_mat3DHeight);   

    for (const auto &deviceInspWindow : vecDeviceInspWindow) {
        for (const auto &windowGroup : deviceInspWindow.vecWindowGroup) {
            auto iterHeightCheckWindow = std::find_if(windowGroup.vecWindows.begin(), windowGroup.vecWindows.end(), [](const Engine::Window &window) { return Engine::Window::Usage::HEIGHT_MEASURE == window.usage; });
            if (iterHeightCheckWindow != windowGroup.vecWindows.end()) {
                _insp3DHeightGroup(windowGroup);
            }
        }
    }
}

void Insp3DHeightRunnable::_insp3DHeightGroup(const Engine::WindowGroup &windowGroup)
{
    auto iterHeightCheckWindow = std::find_if(windowGroup.vecWindows.begin(), windowGroup.vecWindows.end(), [](const Engine::Window &window) { return Engine::Window::Usage::HEIGHT_MEASURE == window.usage; });
    auto window = *iterHeightCheckWindow;

    QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
	if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
		return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_CALC_3D_HEIGHT_DIFF_CMD stCmd;
	Vision::PR_CALC_3D_HEIGHT_DIFF_RPY stRpy;

    stCmd.fEffectHRatioStart = jsonValue["MinRange"].toDouble();
	stCmd.fEffectHRatioEnd = jsonValue["MaxRange"].toDouble();

    stCmd.rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        window.width,
        window.height,
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
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}