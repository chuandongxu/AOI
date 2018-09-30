#include <QJsonObject>
#include <QJsonDocument>

#include "Insp3DHeightRunnable.h"
#include "TimeLog.h"

#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../include/constants.h"
#include "../DataModule/DataUtils.h"
#include "../DataModule/CalcUtils.hpp"

Insp3DHeightRunnable::Insp3DHeightRunnable(
    QThreadPool*         pThreadPoolInsp2D,
    Insp2DRunnablePtr    ptrInsp2DRunnable,
    const cv::Point2f&   ptFramePos,        
    int                  nRow,
    int                  nCol,
    int                  nTotalRows,
    int                  nTotalCols,
    const AutoRunParams& stAutoRunParams,
    BoardInspResultPtr& ptrBoardInsResult) :
    m_pThreadPoolInsp2D (pThreadPoolInsp2D),
    m_ptrInsp2DRunnable (ptrInsp2DRunnable),
    m_ptFramePos        (ptFramePos),
    m_nRow              (nRow),
    m_nCol              (nCol),
    m_nTotalRows        (nTotalRows),
    m_nTotalCols        (nTotalCols),
    m_stAutoRunParams   (stAutoRunParams),
    InspRunnable        (ptFramePos, ptrBoardInsResult)
{
}

Insp3DHeightRunnable::~Insp3DHeightRunnable() {
}

void Insp3DHeightRunnable::set3DHeight(const cv::Mat &mat3DHeight) {
    m_mat3DHeight = mat3DHeight;
}

void Insp3DHeightRunnable::run() {
    m_pThreadPoolInsp2D->waitForDone();
    TimeLogInstance->addTimeLog(std::string("Finished wait for 2D inspection done in thead ") + QThread::currentThread()->objectName().toStdString());
    if (m_mat3DHeight.empty())
        m_mat3DHeight = m_ptrInsp2DRunnable->get3DHeight();
    m_vec2DImages = m_ptrInsp2DRunnable->get2DImages();

    auto vecDeviceInspWindow = m_ptrInsp2DRunnable->getDeviceInspWindow();
    for (const auto &deviceWindow : vecDeviceInspWindow) {
        if (! deviceWindow.bAlignmentPassed)
            continue;

        for (const auto &windowGroup : deviceWindow.vecWindowGroup) {
            auto iterHeightCheckWindow = std::find_if(windowGroup.vecWindows.begin(), windowGroup.vecWindows.end(), [](const Engine::Window &window) { return Engine::Window::Usage::HEIGHT_MEASURE == window.usage; });
            if (iterHeightCheckWindow != windowGroup.vecWindows.end()) {
                _insp3DHeightGroup(windowGroup);
            }
        }

        for (const auto &window : deviceWindow.vecUngroupedWindows) {
            switch (window.usage)
            {
            case Engine::Window::Usage::HEIGHT_MEASURE:
                _insp3DHeightGlobalBase(window);
                break;
            case Engine::Window::Usage::INSP_3D_SOLDER:
                _insp3DSolder(window);
                break;
            default:
                break;

            }
        }
        m_ptrBoardInspResult->addDeviceInspWindow(deviceWindow);
    }
}

void Insp3DHeightRunnable::_insp3DHeightGroup(const Engine::WindowGroup &windowGroup) {
    auto iterHeightCheckWindow = std::find_if(windowGroup.vecWindows.begin(), windowGroup.vecWindows.end(), [](const Engine::Window &window) { return Engine::Window::Usage::HEIGHT_MEASURE == window.usage; });
    auto windowInsp = *iterHeightCheckWindow;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(windowInsp.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        std::string strErrorMsg = "Window \"" + windowInsp.name + "\" inspect parameters \"" + windowInsp.inspParams + "\" is invalid.";
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

    float fNominalAbsHeight = jsonValue["AbsHt"].toDouble();
    float fMaxAbsHeightDiff = jsonValue["MaxAbxHt"].toDouble();
    float fMinAbsHeightDiff = jsonValue["MinAbxHt"].toDouble();
    bool bPass = true;
    if (stRpy.fHeightDiff * MM_TO_UM - fNominalAbsHeight > fMaxAbsHeightDiff)
        bPass = false;
    else if (stRpy.fHeightDiff * MM_TO_UM - fNominalAbsHeight > fMinAbsHeightDiff)
        bPass = false;

    const int HEIGHT_INSP_FAIL = 1000;
    if (bPass)
        m_ptrBoardInspResult->addWindowStatus(windowInsp.Id, Vision::ToInt32(Vision::VisionStatus::OK));
    else
        m_ptrBoardInspResult->addWindowStatus(windowInsp.Id, HEIGHT_INSP_FAIL);
}

void Insp3DHeightRunnable::_insp3DHeightGlobalBase(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        std::string strErrorMsg = "Window \"" + window.name + "\" inspect parameters \"" + window.inspParams + "\" is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        return;
    }

    QJsonObject jsonValue = parse_doucment.object();
    bool bGlobalBase = jsonValue["GlobalBase"].toBool();
    if (!bGlobalBase) {
        m_ptrBoardInspResult->setFatalError();
        std::string strErrorMsg = "Height detect window \"" + window.name + "\" is not in a group and not using global base, then cannot inspect.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        return;
    }

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        std::string strErrorMsg = "Window \"" + window.name + "\" image id " + std::to_string(window.lightId) + " is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return;
    }    

    int baseScale = jsonValue["GlobalBaseScale"].toInt();
    if (baseScale < 2)
        baseScale = 4;
    auto rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        window.width,
        window.height,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);
    auto rectBase = CalcUtils::resizeRect(rectROI, cv::Size(rectROI.width * baseScale, rectROI.height * baseScale));
    auto matColorImage = m_vec2DImages[nImageIndex];
    CalcUtils::adjustRectROI(rectBase, matColorImage);
    cv::Mat matColorROI(matColorImage, rectBase);

    Vision::PR_PICK_COLOR_CMD stPickColorCmd;
    Vision::PR_PICK_COLOR_RPY stPickColorRpy;
    stPickColorCmd.matInputImg = matColorROI.clone();
    stPickColorCmd.rectROI = cv::Rect(0, 0, stPickColorCmd.matInputImg.cols, stPickColorCmd.matInputImg.rows);
    stPickColorCmd.nColorDiff = m_stAutoRunParams.nGlobalBaseColorDiff;
    stPickColorCmd.nGrayDiff = m_stAutoRunParams.nGlobalBaseGrayDiff;
    stPickColorCmd.enMethod = Vision::PR_PICK_COLOR_METHOD::SELECT_COLOR;
    stPickColorCmd.scalarSelect = m_stAutoRunParams.scalarGlobalBase;
    Vision::PR_PickColor(&stPickColorCmd, &stPickColorRpy);
    if (Vision::VisionStatus::OK != stPickColorRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stPickColorRpy.enStatus, &stGetErrInfoRpy);
        std::string strErrorMsg = "Window \"" + window.name + "\" extract color area with error: " + stGetErrInfoRpy.achErrorStr;
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return;
    }

    Vision::PR_CALC_3D_HEIGHT_DIFF_CMD stCmd;
    Vision::PR_CALC_3D_HEIGHT_DIFF_RPY stRpy;

    stCmd.matHeight = m_mat3DHeight;
    stCmd.rectROI = rectROI;
    stCmd.fEffectHRatioStart = jsonValue["MinRange"].toDouble();
    stCmd.fEffectHRatioEnd = jsonValue["MaxRange"].toDouble();

    cv::Mat matBigMask = cv::Mat::ones(stCmd.matHeight.size(), CV_8UC1);
    matBigMask *= Vision::PR_MAX_GRAY_LEVEL;
    cv::Mat matMaskROI(matBigMask, cv::Rect(rectBase));
    stPickColorRpy.matResultImg.copyTo(matMaskROI);
    stCmd.matMask = matBigMask;
    stCmd.vecRectBases.push_back(rectBase);

    Vision::PR_Calc3DHeightDiff(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }

    float fNominalAbsHeight = jsonValue["AbsHt"].toDouble();
    float fMaxAbsHeightDiff = jsonValue["MaxAbxHt"].toDouble();
    float fMinAbsHeightDiff = jsonValue["MinAbxHt"].toDouble();
    bool bPass = true;
    if (stRpy.fHeightDiff * MM_TO_UM - fNominalAbsHeight > fMaxAbsHeightDiff)
        bPass = false;
    else if (stRpy.fHeightDiff * MM_TO_UM - fNominalAbsHeight > fMinAbsHeightDiff)
        bPass = false;

    const int HEIGHT_INSP_FAIL = 1000;
    if (bPass)
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::OK));
    else
        m_ptrBoardInspResult->addWindowStatus(window.Id, HEIGHT_INSP_FAIL);
}

void Insp3DHeightRunnable::_insp3DSolder(const Engine::Window &window) {
    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        std::string strErrorMsg = "Window \"" + window.name + "\" image id " + std::to_string(window.lightId) + " is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return;
    }

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        std::string strErrorMsg = "Window \"" + window.name + "\" inspect parameters \"" + window.inspParams + "\" is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_INSP_3D_SOLDER_CMD stCmd;
    Vision::PR_INSP_3D_SOLDER_RPY stRpy;
    stCmd.matHeight = m_mat3DHeight;
    stCmd.matColorImg = m_vec2DImages[nImageIndex];
    auto rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        window.width,
        window.height,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);
    stCmd.rectDeviceROI = rectROI;

    int nNumSubROI = jsonValue["NumSubROI"].toInt();
    Vision::VectorOfRect vecSubROIs;
    for (int index = 0; index < nNumSubROI; ++ index) {
        QString strKey = "SubROI_" + QString::number(index);
        auto strRect = jsonValue[strKey].toString().toStdString();
        auto rectSubROI = DataUtils::parseRect(strRect);
        rectSubROI.x += rectROI.x;
        rectSubROI.y += rectROI.y;
        stCmd.vecRectCheckROIs.push_back(rectSubROI);
    }
    stCmd.scalarBaseColor = m_stAutoRunParams.scalarGlobalBase;
    stCmd.nBaseColorDiff = m_stAutoRunParams.nGlobalBaseColorDiff;
    stCmd.nBaseGrayDiff = m_stAutoRunParams.nGlobalBaseGrayDiff;
    stCmd.nWettingWidth = Vision::ToInt32(jsonValue["SolderWettingWidth"].toDouble() / m_dResolutionX);

    Vision::PR_Insp3DSolder(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }

    auto conductorAbsHeight = jsonValue["ConductorAbsHeight"].toDouble();
    auto conductorAbsHeightUpLimit = jsonValue["ConductorAbsHeightUpLimit"].toDouble();
    auto conductorAbsHeightLoLimit = jsonValue["ConductorAbsHeightLoLimit"].toDouble();
    auto conductorRelHeightUpLimit = jsonValue["ConductorRelHeightUpLimit"].toDouble();
    auto solderHeightLoLimit = jsonValue["SolderHeightLoLimit"].toDouble();
    auto solderHeightRatioLoLimit = jsonValue["SolderHeightRatioLoLimit"].toDouble();
    auto solderCovRatioLoLimit = jsonValue["SolderCovRatioLoLimit"].toDouble();
    auto solderHeightUpLimit = jsonValue["SolderHeightUpLimit"].toDouble();

    bool bPass = true;

    for (const auto &result : stRpy.vecResults) {
        float fHeightDiff = result.fComponentHeight * MM_TO_UM - conductorAbsHeight;
        if (fHeightDiff >= conductorAbsHeightUpLimit) {
            bPass = false;
        }else if(fHeightDiff < conductorAbsHeightLoLimit) {
            bPass = false;
        }

        if (result.fSolderHeight * MM_TO_UM < solderHeightLoLimit)
            bPass = false;

        float fFullRatioHeight = solderHeightUpLimit < result.fComponentHeight * MM_TO_UM ?
        solderHeightUpLimit : result.fComponentHeight * MM_TO_UM;
        float fRatio = result.fSolderHeight * MM_TO_UM / fFullRatioHeight * ONE_HUNDRED_PERCENT;
        if (fRatio < solderHeightRatioLoLimit)
            bPass = false;

        float fSolderCovPercent = result.fSolderRatio * ONE_HUNDRED_PERCENT;
        if (fSolderCovPercent < solderCovRatioLoLimit)
            bPass = false;
    }

    if (stRpy.vecResults.size() >= 2) {
        float fRelHeight = fabs(stRpy.vecResults[0].fComponentHeight - stRpy.vecResults[1].fComponentHeight) * MM_TO_UM;
        if (fRelHeight > conductorRelHeightUpLimit)
            bPass = false;
    }

    const int INSP_3D_SOLDER_FAIL = 1001;
    if (bPass)
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::OK));
    else
        m_ptrBoardInspResult->addWindowStatus(window.Id, INSP_3D_SOLDER_FAIL);
}
